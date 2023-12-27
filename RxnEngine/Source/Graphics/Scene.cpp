#include "Rxn.h"
#include "Scene.h"

namespace Rxn::Graphics
{
    Scene::Scene()
    {
        m_Camera.Init({ 0.0f, 0.0f, 5.0f });
        m_Camera.SetMoveSpeed(1.0f); 
    }

    Scene::~Scene()
    {
       m_Texture.Release();
       m_SceneShapes.clear();
       
       //m_RenderTargets[0].Release();
       //m_RenderTargets[1].Release();
       //m_RenderTargets[2].Release();

       m_RTVHeap.Release();
       
       m_SRVHeap.Release();
    }

    void Scene::AddShapeFromRaw(const std::vector<VertexPositionColour> &vertices, const std::vector<uint32> &indices, ComPointer<ID3D12CommandAllocator> cmdAl, ComPointer<ID3D12CommandQueue> cmdQueue, ComPointer<ID3D12GraphicsCommandList6> cmdList)
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

    void Scene::AddQuadFromRaw(const std::vector<VertexPositionUV> &quadVertices, ComPointer<ID3D12CommandAllocator> cmdAl, ComPointer<ID3D12CommandQueue> cmdQueue, ComPointer<ID3D12GraphicsCommandList6> cmdList)
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
        // two swap chain + additional intermediate target
        DescriptorHeapDesc rtvDesc(2 + 1);
        rtvDesc.CreateRTVDescriptorHeap(m_RTVHeap);
        m_RTVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        DescriptorHeapDesc srvDesc(1);
        srvDesc.CreateSRVDescriptorHeap(m_SRVHeap);
        m_SRVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    void Scene::InitHeap(uint32 descriptorCount, ComPointer<ID3D12DescriptorHeap> &heap, D3D12_DESCRIPTOR_HEAP_TYPE type)
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
        //m_RenderTargets[0].Release();
        //m_RenderTargets[1].Release();
        //m_RenderTargets[2].Release();

        for (auto &[index, target] : m_Rtvs)
        {
            target.Release();
        }
    }

    void Scene::CreateSwapChainRenderTargetView(GPU::SwapChain &swapChain, uint32 swapChainBufferIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle)
    {
        ComPointer<ID3D12Resource> rtv;
        //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

        ThrowIfFailed(swapChain.GetBuffer(swapChainBufferIndex, rtv));
        RenderContext::GetGraphicsDevice()->CreateRenderTargetView(rtv, nullptr, rtvHandle);
        rtvHandle.Offset(1, m_RTVDescriptorSize);

#ifdef _DEBUG
        WString rtvName = L"rtv[";
        rtvName.append(std::to_wstring(swapChainBufferIndex).c_str());
        rtvName.append(L"]");
        rtv->SetName(rtvName.c_str());
#endif

        std::pair<uint32, ComPointer<ID3D12Resource>> pair;
        pair.first = swapChainBufferIndex;
        pair.second = rtv;

        m_Rtvs.emplace_back(pair);
    }

    void Scene::CreateRenderTargetCopy(uint32 copyFromIndex, uint32 destinationIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle)
    {
        ComPointer<ID3D12Resource> rtv;        
        D3D12_CLEAR_VALUE clearValue = {};
        memcpy(clearValue.Color, INTERMEDIATE_CLEAR_COLOUR, sizeof(INTERMEDIATE_CLEAR_COLOUR));
        clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

        D3D12_RESOURCE_DESC copyDesc = GetResourceForRenderTarget(copyFromIndex);

        const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateCommittedResource(
            &heapProps, 
            D3D12_HEAP_FLAG_NONE, 
            &copyDesc, 
            D3D12_RESOURCE_STATE_RENDER_TARGET, 
            &clearValue, 
            IID_PPV_ARGS(&rtv)
        ));

        RenderContext::GetGraphicsDevice()->CreateRenderTargetView(rtv, nullptr, rtvHandle);
        rtvHandle.Offset(1, m_RTVDescriptorSize);

        std::pair<uint32, ComPointer<ID3D12Resource>> pair(destinationIndex, rtv);

        m_Rtvs.emplace_back(pair);
    }

    void Scene::CreateShaderResourceViewForRenderTargetView(ID3D12Resource *pResource, uint32 mipLevel)
    {
        const D3D12_RESOURCE_DESC copyDesc = pResource->GetDesc();
        
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = copyDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = mipLevel;

        CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_SRVHeap->GetCPUDescriptorHandleForHeapStart());

        RenderContext::GetGraphicsDevice()->CreateShaderResourceView(pResource, &srvDesc, srvHandle);

    }

    void Scene::InitSceneRenderTargets(const uint32 frameIndex, GPU::SwapChain &swapChain)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());
        CreateSwapChainRenderTargetView(swapChain, 0, rtvHandle);
        CreateSwapChainRenderTargetView(swapChain, 1, rtvHandle);
        CreateRenderTargetCopy(frameIndex, 2, rtvHandle);
        CreateShaderResourceViewForRenderTargetView(GetRenderTarget(2));
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

    Basic::Quad & Scene::GetQuad()
    {
        return m_Quad;
    }

    ComPointer<ID3D12Resource> & Scene::GetRenderTarget(const uint32 rtvIndex)
    {
        for (auto &[index, target] : m_Rtvs)
        {
            if (index == rtvIndex)
            {
                return target;
            }
        }

        RXN_LOGGER::Error(L"Could not find render target view stored at index %d", rtvIndex);
        throw std::runtime_error("");
    }

    ComPointer<ID3D12DescriptorHeap> & Scene::GetRtvHeap()
    {
        return m_RTVHeap;
    }

    ComPointer<ID3D12DescriptorHeap> & Scene::GetSrvHeap()
    {
        return m_SRVHeap;
    }

    uint32 Scene::GetRtvDescriptorHeapSize() const
    {
        return m_RTVDescriptorSize;
    }

    uint32 Scene::GetSrvDescriptorHeapSize() const
    {
        return m_SRVDescriptorSize;
    }

    void Scene::DrawSceneShapes(const ComPointer<ID3D12GraphicsCommandList6> &cmdList) const
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

    D3D12_RESOURCE_DESC Scene::GetResourceForRenderTarget(uint32 rtvIndex)
    {
        for (auto &[index, target] : m_Rtvs)
        {
            if (index == rtvIndex)
            {
                return target->GetDesc();
            }
        }

        RXN_LOGGER::Error(L"Render target could not be found!");
        throw std::runtime_error("");
    }

}

