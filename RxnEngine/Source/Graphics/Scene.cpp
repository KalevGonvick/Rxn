#include "Rxn.h"
#include "Scene.h"
#include "DescriptorHeapDesc.h"

namespace Rxn::Graphics
{
    Scene::Scene()
        : m_RTVDescriptorSize(0)
        , m_SRVDescriptorSize(0)
        , m_DynamicConstantBuffer(sizeof(DrawConstantBuffer), 256, SwapChainBuffers::TOTAL_BUFFERS)
    {
        m_Camera.Init({ 0.0f, 0.0f, 5.0f });
        m_Camera.SetMoveSpeed(1.0f);
        
    }

    Scene::~Scene() = default;

    void Scene::SetSerializedRootSignature(CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc)
    {
        ComPointer<ID3DBlob> signature;
        ComPointer<ID3DBlob> error;

        HRESULT result;
        result = D3DX12SerializeVersionedRootSignature(&rootSigDesc, RenderContext::GetHighestRootSignatureVersion(), &signature, &error);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to serialize root signature descriptor.");
            return;
        }

        result = RenderContext::GetGraphicsDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to declare root signature.");
            return;
        }

        NAME_D3D12_OBJECT(m_RootSignature);
    }

    void Scene::AddShapeFromRaw(const std::vector<VertexPositionColour> &vertices, const std::vector<UINT> &indices, ID3D12CommandAllocator *cmdAl, ID3D12CommandQueue *cmdQueue, ID3D12GraphicsCommandList *cmdList)
    {
        auto shape = std::make_shared<Basic::Shape>();
        shape->ReadDataFromRaw(vertices, indices);
        HRESULT result = shape->UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), cmdQueue, cmdAl, cmdList);

        //m_Shape.ReadDataFromRaw(vertices, indices);
        //HRESULT result = m_Shape.UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), cmdQueue, cmdAl, cmdList);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload shape resources to gpu");
            return;
        }

        m_SceneShapes.push_back(std::move(shape));
    }

    void Scene::AddQuadFromRaw(const std::vector<VertexPositionUV> &quadVertices, ID3D12CommandAllocator *cmdAl, ID3D12CommandQueue *cmdQueue, ID3D12GraphicsCommandList *cmdList)
    {
        m_Quad.ReadDataFromRaw(quadVertices);

        HRESULT result = m_Quad.UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), cmdQueue, cmdAl, cmdList);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload quad resources to gpu");
        }
    }

    void Scene::AddTexture(ComPointer<ID3D12Resource> &textureUploadHeap, D3D12_RESOURCE_DESC textureDesc, ID3D12GraphicsCommandList *cmdList, D3D12_SUBRESOURCE_DATA textureData)
    {
        const auto heapTextureProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        HRESULT result = RenderContext::GetGraphicsDevice()->CreateCommittedResource(&heapTextureProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Texture));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource for texture.");
            return;
        }

        // Create the GPU upload buffer.
        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, 1);
        const auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto uploadBufferSizeDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
        ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &uploadBufferSizeDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap)));

        UpdateSubresources(cmdList, m_Texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
        const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        cmdList->ResourceBarrier(1, &transition);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        RenderContext::GetGraphicsDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRVHeap->GetCPUDescriptorHandleForHeapStart());
    }

    void Scene::InitHeaps()
    {
        DescriptorHeapDesc rtvDesc(SwapChainBuffers::TOTAL_BUFFERS + 1);
        rtvDesc.CreateRTVDescriptorHeap(m_RTVHeap);

        DescriptorHeapDesc srvDesc(1);
        srvDesc.CreateSRVDescriptorHeap(m_SRVHeap);

        m_RTVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_SRVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    void Scene::ReleaseResourceViews()
    {
        m_RenderTargets[SwapChainBuffers::BUFFER_ONE].Release();
        m_RenderTargets[SwapChainBuffers::BUFFER_TWO].Release();
        m_IntermediateRenderTarget.Release();
    }

    void Scene::InitSceneRenderTargets(const uint32 frameIndex, GPU::SwapChain swapChain)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());
        CreateRenderTargets(swapChain, rtvHandle);
        CreateIntermediateRenderTarget(frameIndex, rtvHandle);
    }

    void Scene::SetDynamicConstantBufferByIndex(ComPointer<ID3D12GraphicsCommandList> frameCmdList, const uint32 frameIndex, uint32 drawIndex)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE intermediateRtvHandle(GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), SwapChainBuffers::TOTAL_BUFFERS, GetRtvDescriptorHeapSize());
        frameCmdList->SetGraphicsRootConstantBufferView(RootParameterCB, GetDynamicConstantBuffer().GetGpuVirtualAddress(drawIndex, frameIndex));
        frameCmdList->OMSetRenderTargets(1, &intermediateRtvHandle, FALSE, nullptr);
        frameCmdList->ClearRenderTargetView(intermediateRtvHandle, INTERMEDIATE_CLEAR_COLOUR, 0, nullptr);
    }

    void Scene::ClearOutputMergerRenderTarget(ComPointer<ID3D12GraphicsCommandList> frameCmdList, const uint32 frameIndex)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), frameIndex, GetRtvDescriptorHeapSize());
        frameCmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
        frameCmdList->ClearRenderTargetView(rtvHandle, INTERMEDIATE_CLEAR_COLOUR, 0, nullptr);
    }

    void Scene::SetShaderResourceViewDescriptorHeap(ComPointer<ID3D12GraphicsCommandList> frameCmdList)
    {
        frameCmdList->SetGraphicsRootSignature(GetRootSignature().Get());
        ID3D12DescriptorHeap *ppHeaps[] = { GetSrvHeap().Get() };
        frameCmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
        frameCmdList->SetGraphicsRootDescriptorTable(RootParameterSRV, GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());
    }

    void Scene::UpdateConstantBufferByIndex(const DirectX::XMMATRIX &projMat, const uint32 frameIndex, const uint32 drawIndex)
    {
        static float rot = 0.0f;
        auto *drawCB = (DrawConstantBuffer *)GetDynamicConstantBuffer().GetMappedMemory(drawIndex, frameIndex);
        drawCB->worldViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(rot) * DirectX::XMMatrixRotationX(-rot) * GetCamera().GetViewMatrix() * projMat);
        rot += 0.01f;
    }

    DrawConstantBuffer *Scene::GetConstantBuffer(const uint32 drawIndex, const uint32 frameIndex)
    {
        return  (DrawConstantBuffer *)m_DynamicConstantBuffer.GetMappedMemory(drawIndex, frameIndex);
    }

    Buffer::DynamicConstantBuffer & Scene::GetDynamicConstantBuffer()
    {
        return m_DynamicConstantBuffer;
    }

    Camera & Scene::GetCamera()
    {
        return m_Camera;
    }

    ComPointer<ID3D12RootSignature> & Scene::GetRootSignature()
    {
        return m_RootSignature;
    }

    Basic::Quad & Scene::GetQuad()
    {
        return m_Quad;
    }

    ComPointer<ID3D12Resource> & Scene::GetRenderTarget(const uint32 index)
    {
        return m_RenderTargets[index];
    }

    ComPointer<ID3D12DescriptorHeap> & Scene::GetRtvHeap()
    {
        return m_RTVHeap;
    }

    ComPointer<ID3D12DescriptorHeap> & Scene::GetSrvHeap()
    {
        return m_SRVHeap;
    }

    ComPointer<ID3D12Resource> & Scene::GetIntermediateRenderTarget()
    {
        return m_IntermediateRenderTarget;
    }

    const uint32 Scene::GetRtvDescriptorHeapSize()
    {
        return m_RTVDescriptorSize;
    }

    const uint32 Scene::GetSrvDescriptorHeapSize()
    {
        return m_SRVDescriptorSize;
    }

    void Scene::DrawSceneShapes(ComPointer<ID3D12GraphicsCommandList> cmdList) const
    {
        for (const auto &shape : m_SceneShapes)
        {
            shape->DrawInstanced(cmdList, 1);
        }
    }

    void Scene::CreateRenderTargets(GPU::SwapChain &swapChain, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle)
    {
        
        ThrowIfFailed(swapChain.GetBuffer(SwapChainBuffers::BUFFER_ONE, m_RenderTargets[SwapChainBuffers::BUFFER_ONE]));
        RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_RenderTargets[SwapChainBuffers::BUFFER_ONE].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_RTVDescriptorSize);
        NAME_D3D12_OBJECT_INDEXED(m_RenderTargets, SwapChainBuffers::BUFFER_ONE);

        ThrowIfFailed(swapChain.GetBuffer(SwapChainBuffers::BUFFER_TWO, m_RenderTargets[SwapChainBuffers::BUFFER_TWO]));
        RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_RenderTargets[SwapChainBuffers::BUFFER_TWO].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_RTVDescriptorSize);
        NAME_D3D12_OBJECT_INDEXED(m_RenderTargets, SwapChainBuffers::BUFFER_TWO);

        
    }

    void Scene::CreateIntermediateRenderTarget(const uint32 frameIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle)
    {
        //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());
        D3D12_CLEAR_VALUE clearValue = {};
        memcpy(clearValue.Color, INTERMEDIATE_CLEAR_COLOUR, sizeof(INTERMEDIATE_CLEAR_COLOUR));
        clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        // Create an intermediate render target that is the same dimensions as the swap chain.
        auto renderTargetDescCopy = m_RenderTargets[frameIndex]->GetDesc();
        const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &renderTargetDescCopy, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&m_IntermediateRenderTarget)));


        RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_IntermediateRenderTarget.Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_RTVDescriptorSize);
        NAME_D3D12_OBJECT(m_IntermediateRenderTarget);
        CreateShaderResourceViewForResource(renderTargetDescCopy);
    }

    void Scene::CreateShaderResourceViewForResource(D3D12_RESOURCE_DESC &resDesc)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = resDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_SRVHeap->GetCPUDescriptorHandleForHeapStart());
        RenderContext::GetGraphicsDevice()->CreateShaderResourceView(m_IntermediateRenderTarget.Get(), &srvDesc, srvHandle);
    }

}

