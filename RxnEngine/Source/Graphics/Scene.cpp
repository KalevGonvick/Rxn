#include "Rxn.h"
#include "Scene.h"

namespace Rxn::Graphics
{
    Scene::Scene()
    {
        m_Camera.Init({ 0.0f, 0.0f, 5.0f });
        m_Camera.SetMoveSpeed(1.0f); 
    }

    Scene::~Scene() = default;

    void Scene::SetSerializedRootSignature(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC &rootSigDesc)
    {
        ComPointer<ID3DBlob> signature;
        ComPointer<ID3DBlob> error;

        HRESULT result;
        result = D3DX12SerializeVersionedRootSignature(&rootSigDesc, RenderContext::GetHighestRootSignatureVersion(), &signature, &error);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to serialize root signature descriptor.");
            throw std::runtime_error("");
            return;
        }

        result = RenderContext::GetGraphicsDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to declare root signature.");
            throw std::runtime_error("");
            return;
        }

        NAME_D3D12_OBJECT(m_RootSignature);
    }

    void Scene::AddShapeFromRaw(const std::vector<VertexPositionColour> &vertices, const std::vector<UINT> &indices, ID3D12CommandAllocator *cmdAl, ID3D12CommandQueue *cmdQueue, ID3D12GraphicsCommandList *cmdList)
    {
        auto shape = std::make_shared<Basic::Shape>();
        shape->ReadDataFromRaw(vertices, indices);
        HRESULT result = shape->UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), cmdQueue, cmdAl, cmdList);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload shape resources to gpu");
            throw std::runtime_error("");
        }

        m_SceneShapes.emplace_back(std::move(shape));
    }

    void Scene::AddQuadFromRaw(const std::vector<VertexPositionUV> &quadVertices, ID3D12CommandAllocator *cmdAl, ID3D12CommandQueue *cmdQueue, ID3D12GraphicsCommandList *cmdList)
    {
        m_Quad.ReadDataFromRaw(quadVertices);

        HRESULT result = m_Quad.UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), cmdQueue, cmdAl, cmdList);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload quad resources to gpu");
            throw std::runtime_error("");
        }
    }

    void Scene::AddTexture(ComPointer<ID3D12Resource> &textureUploadHeap, const D3D12_RESOURCE_DESC &textureDesc, ID3D12GraphicsCommandList *cmdList, D3D12_SUBRESOURCE_DATA textureData)
    {
        const auto heapTextureProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        HRESULT result = RenderContext::GetGraphicsDevice()->CreateCommittedResource(&heapTextureProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Texture));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource for texture.");
            throw std::runtime_error("");
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
        DescriptorHeapDesc rtvDesc(static_cast<uint32>(SwapChainBuffers::TOTAL_BUFFERS) + 1);
        rtvDesc.CreateRTVDescriptorHeap(m_RTVHeap);
        m_RTVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        DescriptorHeapDesc srvDesc(1);
        srvDesc.CreateSRVDescriptorHeap(m_SRVHeap);
        m_SRVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    void Scene::InitHeap(uint32 descriptorCount, ComPointer<ID3D12DescriptorHeap> heap, D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        DescriptorHeapDesc desc(descriptorCount);
        switch (type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            desc.CreateSRVDescriptorHeap(heap);
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            desc.CreateSamplerDescriptorHeap(heap);
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            desc.CreateRTVDescriptorHeap(heap);
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            desc.CreateDSVDescriptorHeap(heap);
            break;
        default:
            throw std::runtime_error("");
        }
    }

    void Scene::ReleaseResourceViews()
    {
        m_RenderTargets[static_cast<uint32>(SwapChainBuffers::BUFFER_ONE)].Release();
        m_RenderTargets[static_cast<uint32>(SwapChainBuffers::BUFFER_TWO)].Release();
        m_IntermediateRenderTarget.Release();
    }

    void Scene::InitSceneRenderTargets(const uint32 frameIndex, GPU::SwapChain &swapChain)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

        ThrowIfFailed(swapChain.GetBuffer(static_cast<uint32>(SwapChainBuffers::BUFFER_ONE), m_RenderTargets[static_cast<uint32>(SwapChainBuffers::BUFFER_ONE)]));
        RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_RenderTargets[static_cast<uint32>(SwapChainBuffers::BUFFER_ONE)].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_RTVDescriptorSize);
        NAME_D3D12_OBJECT_INDEXED(m_RenderTargets, static_cast<uint32>(SwapChainBuffers::BUFFER_ONE));

        ThrowIfFailed(swapChain.GetBuffer(static_cast<uint32>(SwapChainBuffers::BUFFER_TWO), m_RenderTargets[static_cast<uint32>(SwapChainBuffers::BUFFER_TWO)]));
        RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_RenderTargets[static_cast<uint32>(SwapChainBuffers::BUFFER_TWO)].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_RTVDescriptorSize);
        NAME_D3D12_OBJECT_INDEXED(m_RenderTargets, static_cast<uint32>(SwapChainBuffers::BUFFER_TWO));

        D3D12_CLEAR_VALUE clearValue = {};
        memcpy(clearValue.Color, INTERMEDIATE_CLEAR_COLOUR, sizeof(INTERMEDIATE_CLEAR_COLOUR));
        clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        // Create an intermediate render target that is the same dimensions as the swap chain.
        auto renderTargetDescCopy = m_RenderTargets[frameIndex]->GetDesc();
        const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &renderTargetDescCopy, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&m_IntermediateRenderTarget)));


        RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_IntermediateRenderTarget, nullptr, rtvHandle);
        rtvHandle.Offset(1, m_RTVDescriptorSize);
        NAME_D3D12_OBJECT(m_IntermediateRenderTarget);


        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = renderTargetDescCopy.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_SRVHeap->GetCPUDescriptorHandleForHeapStart());
        RenderContext::GetGraphicsDevice()->CreateShaderResourceView(m_IntermediateRenderTarget, &srvDesc, srvHandle);
    }

    void Scene::SetDynamicConstantBufferByIndex(ComPointer<ID3D12GraphicsCommandList> frameCmdList, const uint32 frameIndex, uint32 drawIndex)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE intermediateRtvHandle(GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), static_cast<uint32>(SwapChainBuffers::TOTAL_BUFFERS), GetRtvDescriptorHeapSize());
        frameCmdList->SetGraphicsRootConstantBufferView(RootParameterCB, GetDynamicConstantBuffer().GetGpuVirtualAddress(drawIndex, frameIndex));
        frameCmdList->OMSetRenderTargets(1, &intermediateRtvHandle, FALSE, nullptr);
        frameCmdList->ClearRenderTargetView(intermediateRtvHandle, INTERMEDIATE_CLEAR_COLOUR, 0, nullptr);
    }

    void Scene::UpdateConstantBufferByIndex(const DirectX::XMMATRIX &projMat, const uint32 frameIndex, const uint32 drawIndex)
    {
        static float rot = 0.0f;
        auto *drawCB = (DrawConstantBuffer *)GetDynamicConstantBuffer().GetMappedMemory(drawIndex, frameIndex);
        drawCB->worldViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(rot) * DirectX::XMMatrixRotationX(-rot) * GetCamera().GetViewMatrix() * projMat);
        rot += 0.01f;
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

    uint32 Scene::GetRtvDescriptorHeapSize() const
    {
        return m_RTVDescriptorSize;
    }

    uint32 Scene::GetSrvDescriptorHeapSize() const
    {
        return m_SRVDescriptorSize;
    }

    void Scene::DrawSceneShapes(const ComPointer<ID3D12GraphicsCommandList> &cmdList) const
    {
        for (const auto &shape : m_SceneShapes)
        {
            shape->DrawInstanced(cmdList, 1);
        }
    }

    void Scene::CreateSrvForResource(const D3D12_RESOURCE_DESC &resourceDesc, ComPointer<ID3D12Resource> &resource)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = resourceDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_SRVHeap->GetCPUDescriptorHandleForHeapStart());
        RenderContext::GetGraphicsDevice()->CreateShaderResourceView(resource, &srvDesc, srvHandle);
    }

}

