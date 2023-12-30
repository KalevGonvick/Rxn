#include "Rxn.h"
#include "Scene.h"

namespace Rxn::Graphics
{
    Scene::Scene()
    {
        m_Camera.Init({ 0.0f, 0.0f, 5.0f });
        m_Camera.SetMoveSpeed(5.0f); 
    }

    Scene::~Scene()
    {
       m_SceneShapes.clear();
       ReleaseResourceViews();
       m_RTVHeap.Release();
       m_SRVHeap.Release();
    }

    void Scene::ReleaseResourceViews()
    {
        for (auto &[index, target] : m_Rtvs)
        {
            target.Release();
        }
    }

    void Scene::Init(ID3D12Device8 *pDevice, GPU::SwapChain &swapChain, D3D_ROOT_SIGNATURE_VERSION version)
    {
        // two swap chain + additional intermediate target
        DescriptorHeapDesc rtvDesc(2 + 1);
        rtvDesc.CreateRTVDescriptorHeap(&m_RTVHeap);
        m_RTVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        DescriptorHeapDesc srvDesc(1);
        srvDesc.CreateSRVDescriptorHeap(&m_SRVHeap);
        m_SRVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        
        //ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);    // 2 frequently changed diffuse + normal textures - using registers t1 and t2.
        //ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);    // 1 frequently changed constant buffer.
        //ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);                                                // 1 infrequently changed shadow texture - starting in register t0.
        //ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0);
        std::vector<CD3DX12_DESCRIPTOR_RANGE1> rangeVector;
        //rangeVector.emplace_back();
        //rangeVector[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        //rangeVector.emplace_back();
        //rangeVector[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        rangeVector.emplace_back();
        rangeVector[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
        //rangeVector.emplace_back();
        //rangeVector[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0);
        

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameterVector;
        rootParameterVector.emplace_back();
        rootParameterVector[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameterVector.emplace_back();
        rootParameterVector[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameterVector.emplace_back();
        rootParameterVector[2].InitAsDescriptorTable(1, &rangeVector[0], D3D12_SHADER_VISIBILITY_PIXEL);
        //rootParameterVector.emplace_back();
        //rootParameterVector[3].InitAsDescriptorTable(1, &rangeVector[1], D3D12_SHADER_VISIBILITY_ALL);
        //rootParameterVector.emplace_back();
        //rootParameterVector[4].InitAsDescriptorTable(1, &rangeVector[2], D3D12_SHADER_VISIBILITY_PIXEL);
        //rootParameterVector.emplace_back();
        //rootParameterVector[5].InitAsDescriptorTable(1, &rangeVector[3], D3D12_SHADER_VISIBILITY_PIXEL);


        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Init_1_1(
            static_cast<uint32>(rootParameterVector.size()), 
            rootParameterVector.data(), 
            1, 
            &sampler, 
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );

        ComPointer<ID3DBlob> signature;
        ComPointer<ID3DBlob> error;

        HRESULT result;
        result = D3DX12SerializeVersionedRootSignature(
            &rootSignatureDesc, 
            version, 
            &signature, 
            &error
        );

        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to serialize root signature descriptor.");
            throw std::runtime_error("");
        }

        result = pDevice->CreateRootSignature(
            0, 
            signature->GetBufferPointer(), 
            signature->GetBufferSize(), 
            IID_PPV_ARGS(&m_RootSignature)
        );
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to declare root signature.");
            throw std::runtime_error("");
        }

#ifdef _DEBUG
        NAME_D3D12_OBJECT(m_RootSignature);
#endif

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());
        for (uint32 i = 0; i < swapChain.GetBufferCount(); i++)
        {
            CreateSwapChainRenderTargetView(swapChain, i, rtvHandle);
        }
        
        CreateRenderTargetCopy(0, 2, rtvHandle);
        CreateShaderResourceViewForRenderTargetView(GetRenderTarget(2));

        m_DynamicConstantBuffer.Create(pDevice, sizeof(DrawConstantBuffer), 256, 2);
        m_PipelineLibrary.Build(pDevice, m_RootSignature, 1, 256, 2);
    }

    void Scene::AddShapeFromRaw(ID3D12Device8 *pDevice, const std::vector<VertexPositionColour> &vertices, const std::vector<UINT> &indices, ID3D12GraphicsCommandList6 *cmdList)
    {
        auto shape = std::make_shared<Basic::Shape>();
        shape->ReadDataFromRaw(vertices, indices);
        HRESULT result = shape->UploadGpuResources(pDevice, cmdList);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload shape resources to gpu");
            throw std::runtime_error("");
        }

        m_SceneShapes.emplace_back(std::move(shape));
    }

    void Scene::AddQuadFromRaw(ID3D12Device8 *pDevice, const std::vector<VertexPositionUV> &quadVertices, ID3D12GraphicsCommandList6 *cmdList)
    {
        m_Quad.ReadDataFromRaw(quadVertices);

        HRESULT result = m_Quad.UploadGpuResources(pDevice, cmdList);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload quad resources to gpu");
            throw std::runtime_error("");
        }
    }

    void Scene::CreateSwapChainRenderTargetView(GPU::SwapChain &swapChain, uint32 swapChainBufferIndex, CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle)
    {
        ComPointer<ID3D12Resource> rtv;
        ThrowIfFailed(swapChain.GetBuffer(swapChainBufferIndex, &rtv));
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

    ID3D12RootSignature *Scene::GetRootSignature()
    {
        return m_RootSignature;
    }

    Mapped::PipelineLibrary &Scene::GetPipelineLibrary()
    {
        return m_PipelineLibrary;
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

    CD3DX12_CPU_DESCRIPTOR_HANDLE Scene::GetRenderTargetCPUDescriptorHandle(uint32 rtvIndex)
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE{ 
            m_RTVHeap->GetCPUDescriptorHandleForHeapStart(),
            static_cast<int32>(rtvIndex),
            m_RTVDescriptorSize 
        };
    }

    void Scene::UpdateConstantBufferByIndex(const DirectX::XMMATRIX &projMat, const uint32 frameIndex, const uint32 bufferOffset)
    {
        static float rot = 0.0f;
        auto *drawCB = (DrawConstantBuffer *)GetDynamicConstantBuffer().GetMappedMemory(frameIndex, bufferOffset);
        drawCB->worldViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(rot) * DirectX::XMMatrixRotationX(-rot) * GetCamera().GetViewMatrix() * projMat);
        rot += 0.005f;
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

    ID3D12Resource * Scene::GetRenderTarget(const uint32 rtvIndex)
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

    ID3D12DescriptorHeap* Scene::GetRtvHeap()
    {
        return m_RTVHeap;
    }

    ID3D12DescriptorHeap *Scene::GetSrvHeap()
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

    void Scene::DrawSceneShapes(ID3D12GraphicsCommandList6 *cmdList) const
    {
        for (const auto &shape : m_SceneShapes)
        {
            shape->DrawInstanced(cmdList, 1);
        }
    }

    void Scene::CreateSrvForResource(const D3D12_RESOURCE_DESC &resourceDesc, ID3D12Resource *resource)
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

