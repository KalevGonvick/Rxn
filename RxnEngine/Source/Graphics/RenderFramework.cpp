#include "Rxn.h"
#include "RenderFramework.h"

namespace Rxn::Graphics
{
    RenderFramework::RenderFramework(int width, int height)
        : m_UseWarpDevice(false)
        , m_HasTearingSupport(false)
        , m_Width(width)
        , m_Viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
        , m_ScissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
        , m_Height(height)
        , m_RTVDescriptorSize(0)
        , m_IsRenderReady(false)
        , m_FenceValues{}
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        m_AssetsPath = assetsPath;
        m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
    }

    RenderFramework::~RenderFramework()
    {
    }

    HRESULT RenderFramework::DX12_LoadPipeline()
    {
        HRESULT result;

        result = DX12_CreateFactory();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create factory...");
            return result;
        }

        result = DX12_CreateIndependantDevice();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create DX12 device");
            return result;
        }

        result = DX12_CreateCommantQueue();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create command queue");
            return result;
        }

        result = DX12_CreateDescriptorHeaps();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create descriptor heaps.");
            return result;
        }

        result = DX12_CreateRootSignature();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create root signature.");
        }

        return result;
    }

    HRESULT RenderFramework::DX12_CreateCommantQueue()
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        return m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
    }

    HRESULT RenderFramework::DX12_CreateDescriptorHeaps()
    {
        HRESULT result;
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = Constants::Graphics::SLIDE_COUNT;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        result = m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create new RTV descriptor heap.");
            return result;
        }

        // Describe and create a shader resource view (SRV) heap for the texture.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        result = m_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SRVHeap));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create new SRV descriptor heap.");
            return result;
        }


        m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        return S_OK;
    }

    HRESULT RenderFramework::DX12_CreateFrameResources()
    {

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

        HRESULT result;
        // Create a RTV for each frame.
        for (UINT n = 0; n < Constants::Graphics::SLIDE_COUNT; n++)
        {

            result = m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n]));
            if (FAILED(result))
            {
                RXN_LOGGER::Error(L"Failed to create RTV for buffer %d", n);
                return result;
            }

            m_Device->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_RTVDescriptorSize);

            result = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[n]));
            if (FAILED(result))
            {
                RXN_LOGGER::Error(L"Failed to create command allocator for buffer %d.", n);
                return result;
            }
        }

        return S_OK;
    }

    HRESULT RenderFramework::DX12_CreateRootSignature()
    {

        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

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

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPointer<ID3DBlob> signature;
        ComPointer<ID3DBlob> error;

        HRESULT result;
        result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to serialize root signature descriptor.");
            return result;
        }

        result = m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to declare root signature.");
            return result;
        }

        return S_OK;
    }

    HRESULT RenderFramework::DX12_LoadAssets()
    {
        HRESULT result;

        result = DX12_CreateNewPSO();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create new pipeline state object.");
            return result;
        }

        result = DX12_CreateCommandList();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create new commandlist");
            return result;
        }

        result = DX12_CreateVertexBufferResource();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create vertex buffer resource.");
            return result;
        }

        ComPointer<ID3D12Resource> textureUploadHeap;

        result = DX12_CreateTextureUploadHeap(textureUploadHeap);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload texture.");
            return result;
        }

        // Close the command list and execute it to begin the initial GPU setup.
        ThrowIfFailed(m_CommandList->Close());
        ID3D12CommandList *ppCommandLists[] = { m_CommandList.Get() };
        m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        result = DX12_CreateFrameSyncObjects();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create frame sync objects");
            return result;
        }

        const UINT64 fence = m_FenceValues[m_FrameIndex];
        ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), fence));
        m_FenceValues[m_FrameIndex]++;

        // Wait until the previous frame is finished.
        if (m_Fence->GetCompletedValue() < fence)
        {
            ThrowIfFailed(m_Fence->SetEventOnCompletion(fence, m_FenceEvent));
            WaitForSingleObject(m_FenceEvent, INFINITE);
        }

        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        return S_OK;
    }

    HRESULT RenderFramework::DX12_CreateNewPSO()
    {
        HRESULT result;

        ComPointer<ID3DBlob> vertexShader;
        ComPointer<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        result = D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to compile vertex shader.");
            return result;
        }

        result = D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to compile pixel shader");
            return result;
        }

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_RootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;

        return m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));

    }

    HRESULT RenderFramework::DX12_CreateCommandList()
    {
        HRESULT result;
        result = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[m_FrameIndex].Get(), m_PipelineState.Get(), IID_PPV_ARGS(&m_CommandList));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to initialize new command list.");
            return result;
        }

        return result;
    }

    HRESULT RenderFramework::DX12_CreateVertexBufferResource()
    {
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f * m_AspectRatio, 0.0f }, { 0.5f, 0.0f } },
            { { 0.25f, -0.25f * m_AspectRatio, 0.0f }, { 1.0f, 1.0f } },
            { { -0.25f, -0.25f * m_AspectRatio, 0.0f }, { 0.0f, 1.0f } }
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);
        const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto buf = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

        HRESULT result;

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        result = m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &buf, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexBuffer));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload buffer data.");
            return result;
        }

        // Copy the triangle data to the vertex buffer.
        UINT8 *pVertexDataBegin = nullptr;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.

        result = m_VertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to map vertex buffer");
            return result;
        }

        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        m_VertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
        m_VertexBufferView.StrideInBytes = sizeof(Vertex);
        m_VertexBufferView.SizeInBytes = vertexBufferSize;

        return S_OK;
    }

    HRESULT RenderFramework::DX12_CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap)
    {
        HRESULT result;

        // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
        // the command list that references it has finished executing on the GPU.
        // We will flush the GPU at the end of this method to ensure the resource is not
        // prematurely destroyed.


        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = TextureWidth;
        textureDesc.Height = TextureHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        const auto heapTextureProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        result = m_Device->CreateCommittedResource(&heapTextureProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Texture));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource for texture.");
            return result;
        }

        // Create the GPU upload buffer.
        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, 1);
        const auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto uploadBufferSizeDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
        ThrowIfFailed(m_Device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &uploadBufferSizeDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap)));

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
        std::vector<UINT8> texture = GenerateTextureData();

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = TextureWidth * TexturePixelSize;
        textureData.SlicePitch = textureData.RowPitch * TextureHeight;

        UpdateSubresources(m_CommandList.Get(), m_Texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
        const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_CommandList->ResourceBarrier(1, &transition);

        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        m_Device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRVHeap->GetCPUDescriptorHandleForHeapStart());

        return S_OK;
    }

    // Generate a simple black and white checkerboard texture.
    std::vector<UINT8> RenderFramework::GenerateTextureData()
    {
        const UINT rowPitch = TextureWidth * TexturePixelSize;
        const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
        const UINT cellHeight = TextureWidth >> 3;    // The height of a cell in the checkerboard texture.
        const UINT textureSize = rowPitch * TextureHeight;

        std::vector<UINT8> data(textureSize);
        UINT8 *pData = &data[0];

        for (UINT n = 0; n < textureSize; n += TexturePixelSize)
        {
            UINT x = n % rowPitch;
            UINT y = n / rowPitch;
            UINT i = x / cellPitch;
            UINT j = y / cellHeight;

            if (i % 2 == j % 2)
            {
                pData[n] = 0x00;        // R
                pData[n + 1] = 0x00;    // G
                pData[n + 2] = 0x00;    // B
                pData[n + 3] = 0xff;    // A
            }
            else
            {
                pData[n] = 0xff;        // R
                pData[n + 1] = 0xff;    // G
                pData[n + 2] = 0xff;    // B
                pData[n + 3] = 0xff;    // A
            }
        }

        return data;
    }

    HRESULT RenderFramework::DX12_CreateFrameSyncObjects()
    {
        HRESULT result;

        result = m_Device->CreateFence(m_FenceValues[m_FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));

        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create new fence.");
            return result;
        }
        m_FenceValues[m_FrameIndex]++;

        // Create an event handle to use for frame synchronization.
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_FenceEvent == nullptr)
        {
            RXN_LOGGER::Error(L"Failed to create new fence event.");
            return HRESULT_FROM_WIN32(GetLastError());
        }

        return result;
    }

    void RenderFramework::DX12_MoveToNextFrame()
    {
        const UINT64 fenceValue = m_FenceValues[m_FrameIndex];
        ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), fenceValue));

        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        // If the next frame is not ready to be rendered yet, wait until it is ready.
        if (m_Fence->GetCompletedValue() < m_FenceValues[m_FrameIndex])
        {
            ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_FenceEvent));
            WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
        }

        m_FenceValues[m_FrameIndex] = fenceValue + 1;
    }

    void RenderFramework::DX12_WaitForGPUFence()
    {
        // Schedule a Signal command in the queue.
        ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_FenceValues[m_FrameIndex]));

        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_FenceEvent));
        WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

        m_FenceValues[m_FrameIndex]++;
    }

    void RenderFramework::DX12_PopulateCommandList()
    {
        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        ThrowIfFailed(m_CommandAllocators[m_FrameIndex]->Reset());

        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        ThrowIfFailed(m_CommandList->Reset(m_CommandAllocators[m_FrameIndex].Get(), m_PipelineState.Get()));

        // Set necessary state.
        m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

        ID3D12DescriptorHeap *ppHeaps[] = { m_SRVHeap.Get() };
        m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        m_CommandList->SetGraphicsRootDescriptorTable(0, m_SRVHeap->GetGPUDescriptorHandleForHeapStart());
        m_CommandList->RSSetViewports(1, &m_Viewport);
        m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

        // Indicate that the back buffer will be used as a render target.
        const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_CommandList->ResourceBarrier(1, &transition);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RTVDescriptorSize);
        m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

        // Record commands.
        const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
        m_CommandList->DrawInstanced(3, 1, 0, 0);


        // Indicate that the back buffer will now be used to present.
        const auto finaltransition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_CommandList->ResourceBarrier(1, &finaltransition);

        ThrowIfFailed(m_CommandList->Close());
    }

    void RenderFramework::DX12_Destroy()
    {
        DX12_WaitForGPUFence();
        CloseHandle(m_FenceEvent);
    }

    void RenderFramework::DX12_Render()
    {
        if (m_IsRenderReady)
        {
            // Record all the commands we need to render the scene into the command list.
            DX12_PopulateCommandList();

            // Execute the command list.
            ID3D12CommandList *ppCommandLists[] = { m_CommandList.Get() };
            m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

            // Present the frame.
            ThrowIfFailed(m_SwapChain->Present(1, 0));

            DX12_MoveToNextFrame();
        }

    }

    void RenderFramework::DX12_GetHardwareAdapter(IDXGIFactory1 *pFactory, IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;

        ComPointer<IDXGIAdapter1> adapter;
        ComPointer<IDXGIFactory7> factory7;

        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory7))))
        {
            for (UINT adapterIndex = 0; SUCCEEDED(factory7->EnumAdapterByGpuPreference(adapterIndex, requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter))); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    RXN_LOGGER::Info(L"Using software adaptor, no need to query for physical device...");
                    continue;
                }

                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    RXN_LOGGER::Info(L"Found device that supports DX12");
                    break;
                }
            }
        }

        if (adapter.Get() == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    RXN_LOGGER::Info(L"Using software adaptor, no need to setup physical device...");
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    RXN_LOGGER::Info(L"Created device successfully!");
                    break;
                }
            }
        }

        adapter.Release();
    }

    void RenderFramework::DX12_CheckTearingSupport()
    {
#ifndef PIXSUPPORT
        ComPointer<IDXGIFactory7> factory;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        BOOL allowTearing = FALSE;
        if (SUCCEEDED(hr))
        {
            hr = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        }

        m_HasTearingSupport = SUCCEEDED(hr) && allowTearing;
#else
        m_HasTearingSupport = TRUE;
#endif

    }

    HRESULT RenderFramework::DX12_CreateFactory()
    {
        UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
        ComPointer<ID3D12Debug3> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
#endif

        return CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_Factory));
    }

    HRESULT RenderFramework::DX12_CreateIndependantDevice()
    {
        if (m_UseWarpDevice)
        {
            ComPointer<IDXGIAdapter> warpAdapter;
            ThrowIfFailed(m_Factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
            return D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
        }
        else
        {
            ComPointer<IDXGIAdapter1> hardwareAdapter;
            DX12_GetHardwareAdapter(m_Factory.Get(), &hardwareAdapter);
            return D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
        }
    }
}
