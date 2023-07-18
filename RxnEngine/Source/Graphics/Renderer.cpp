#include "Rxn.h"
#include "Renderer.h"

namespace Rxn::Graphics
{
    Renderer::Renderer(int width, int height)
        : m_UseWarpDevice(false)
        , m_HasTearingSupport(false)
        , m_Width(width)
        , m_Viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
        , m_ScissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
        , m_Height(height)
        , m_RTVDescriptorSize(0)
        , m_SRVDescriptorSize(0)
        , m_Initialized(false)
        , m_DrawIndex(0)
        , m_FenceValues{}
        , m_DynamicConstantBuffer(sizeof(DrawConstantBuffer), MaxDrawsPerFrame, Constants::Graphics::BUFFER_COUNT)
        , m_PipelineLibrary(Constants::Graphics::BUFFER_COUNT, RootParameterCB)
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        m_AssetsPath = assetsPath;
        m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
        memset(m_EnabledEffects, true, sizeof(m_EnabledEffects));
    }

    Renderer::~Renderer() = default;

    void Renderer::Initialize()
    {
        if (m_Initialized)
            return;


    }

    HRESULT Renderer::DX12_LoadPipeline()
    {
        HRESULT result;

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

    HRESULT Renderer::DX12_CreateCommantQueue()
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        return RenderContext::GetGraphicsDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
    }

    HRESULT Renderer::DX12_CreateDescriptorHeaps()
    {
        HRESULT result;
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = Constants::Graphics::BUFFER_COUNT + 1;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        result = RenderContext::GetGraphicsDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap));
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

        result = RenderContext::GetGraphicsDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SRVHeap));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create new SRV descriptor heap.");
            return result;
        }


        m_RTVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_SRVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        return S_OK;
    }

    HRESULT Renderer::DX12_CreateCommandAllocators()
    {

        HRESULT result;
        // Create a RTV for each frame.
        for (UINT n = 0; n < Constants::Graphics::BUFFER_COUNT; n++)
        {

            result = RenderContext::GetGraphicsDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[n]));
            if (FAILED(result))
            {
                RXN_LOGGER::Error(L"Failed to create command allocator for buffer %d.", n);
                return result;
            }
            NAME_D3D12_OBJECT_INDEXED(m_CommandAllocators, n);
        }



        return S_OK;
    }

    HRESULT Renderer::DX12_CreateRootSignature()
    {



        // We don't modify the SRV in the command list after SetGraphicsRootDescriptorTable
        // is executed on the GPU so we can use the default range behavior:
        // D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
        CD3DX12_DESCRIPTOR_RANGE1 ranges[RootParametersCount]{};
        ranges[RootParameterSRV].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[3]{};
        rootParameters[RootParameterUberShaderCB].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[RootParameterCB].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
        rootParameters[RootParameterSRV].InitAsDescriptorTable(1, &ranges[RootParameterSRV], D3D12_SHADER_VISIBILITY_PIXEL);

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
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPointer<ID3DBlob> signature;
        ComPointer<ID3DBlob> error;

        HRESULT result;
        result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, RenderContext::GetHighestRootSignatureVersion(), &signature, &error);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to serialize root signature descriptor.");
            return result;
        }

        result = RenderContext::GetGraphicsDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to declare root signature.");
            return result;
        }
        NAME_D3D12_OBJECT(m_RootSignature);

        return S_OK;
    }

    HRESULT Renderer::DX12_LoadAssets()
    {
        HRESULT result;

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

        m_DynamicConstantBuffer.Init(RenderContext::GetGraphicsDevice().Get());

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

        m_PipelineLibrary.Build(RenderContext::GetGraphicsDevice().Get(), m_RootSignature.Get());

        return S_OK;
    }

    HRESULT Renderer::DX12_CreateCommandList()
    {
        HRESULT result;
        result = RenderContext::GetGraphicsDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[m_FrameIndex].Get(), nullptr, IID_PPV_ARGS(&m_CommandList));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to initialize new command list.");
            return result;
        }

        return result;
    }

    HRESULT Renderer::DX12_CreateVertexBufferResource()
    {

        // Define the geometry for a cube.
        std::vector<Basic::VertexPositionColour> cubeVertices;
        cubeVertices.push_back(Basic::VertexPositionColour{ { -1.0f, 1.0f, -1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(Basic::VertexPositionColour{ {  1.0f, 1.0f, -1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(Basic::VertexPositionColour{ {  1.0f, 1.0f, 1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(Basic::VertexPositionColour{ { -1.0f, 1.0f, 1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });

        cubeVertices.push_back(Basic::VertexPositionColour{ { -1.0f, -1.0f, -1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(Basic::VertexPositionColour{ {  1.0f, -1.0f, -1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(Basic::VertexPositionColour{ {  1.0f, -1.0f, 1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });
        cubeVertices.push_back(Basic::VertexPositionColour{ { -1.0f, -1.0f, 1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });

        std::vector<UINT> cubeIndices;
        cubeIndices.push_back(0);
        cubeIndices.push_back(1);
        cubeIndices.push_back(3);

        cubeIndices.push_back(1);
        cubeIndices.push_back(2);
        cubeIndices.push_back(3);


        cubeIndices.push_back(3);
        cubeIndices.push_back(2);
        cubeIndices.push_back(7);

        cubeIndices.push_back(6);
        cubeIndices.push_back(7);
        cubeIndices.push_back(2);


        cubeIndices.push_back(2);
        cubeIndices.push_back(1);
        cubeIndices.push_back(6);

        cubeIndices.push_back(5);
        cubeIndices.push_back(6);
        cubeIndices.push_back(1);


        cubeIndices.push_back(1);
        cubeIndices.push_back(0);
        cubeIndices.push_back(5);

        cubeIndices.push_back(4);
        cubeIndices.push_back(5);
        cubeIndices.push_back(0);


        cubeIndices.push_back(0);
        cubeIndices.push_back(3);
        cubeIndices.push_back(4);

        cubeIndices.push_back(7);
        cubeIndices.push_back(4);
        cubeIndices.push_back(3);


        cubeIndices.push_back(7);
        cubeIndices.push_back(6);
        cubeIndices.push_back(4);

        cubeIndices.push_back(5);
        cubeIndices.push_back(4);
        cubeIndices.push_back(6);

        m_Shape.ReadDataFromRaw(cubeVertices, cubeIndices);

        HRESULT result;

        result = m_Shape.UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), m_CommandQueue.Get(), m_CommandAllocators[m_FrameIndex].Get(), m_CommandList.Get());
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload shape resources to gpu");
            return result;
        }

        std::vector<Basic::VertexPositionUV> quadVertices;
        quadVertices.push_back(Basic::VertexPositionUV{ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } });
        quadVertices.push_back(Basic::VertexPositionUV{ { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } });
        quadVertices.push_back(Basic::VertexPositionUV{ { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } });
        quadVertices.push_back(Basic::VertexPositionUV{ { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } });

        m_Quad.ReadDataFromRaw(quadVertices);

        result = m_Quad.UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), m_CommandQueue.Get(), m_CommandAllocators[m_FrameIndex].Get(), m_CommandList.Get());
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload quad resources to gpu");
            return result;
        }

        return S_OK;


        //const UINT vertexIndexBufferSize = sizeof(cubeIndices) + sizeof(cubeVertices) + sizeof(quadVertices);

        //HRESULT result;

        //const auto defaultBuf = CD3DX12_RESOURCE_DESC::Buffer(vertexIndexBufferSize);
        //const CD3DX12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        //ThrowIfFailed(m_Device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &defaultBuf, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_VertexIndexBuffer)));

        //const CD3DX12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        //const auto uploadBuf = CD3DX12_RESOURCE_DESC::Buffer(vertexIndexBufferSize);
        //ThrowIfFailed(m_Device->CreateCommittedResource(
        //    &uploadHeapProps,
        //    D3D12_HEAP_FLAG_NONE,
        //    &uploadBuf,
        //    D3D12_RESOURCE_STATE_GENERIC_READ,
        //    nullptr,
        //    IID_PPV_ARGS(&vertexIndexBufferUpload)));

        //NAME_D3D12_OBJECT(m_VertexIndexBuffer);

        //UINT8 *mappedUploadHeap = nullptr;
        //CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        //ThrowIfFailed(vertexIndexBufferUpload->Map(0, &readRange, reinterpret_cast<void **>(&mappedUploadHeap)));

        //// Fill in part of the upload heap with our index and vertex data.
        //UINT8 *heapLocation = static_cast<UINT8 *>(mappedUploadHeap);
        //memcpy(heapLocation, cubeVertices, sizeof(cubeVertices));
        //heapLocation += sizeof(cubeVertices);
        //memcpy(heapLocation, cubeIndices, sizeof(cubeIndices));
        //heapLocation += sizeof(cubeIndices);
        //memcpy(heapLocation, quadVertices, sizeof(quadVertices));

        //// Pack the vertices and indices into their destination by copying from the upload heap.
        //m_CommandList->CopyBufferRegion(m_VertexIndexBuffer.Get(), 0, vertexIndexBufferUpload.Get(), 0, vertexIndexBufferSize);
        //const auto transition = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER);
        //m_CommandList->ResourceBarrier(1, &transition);

        //// Create the index and vertex buffer views.
        //m_CubeVbv.BufferLocation = m_VertexIndexBuffer.Get()->GetGPUVirtualAddress();
        //m_CubeVbv.SizeInBytes = sizeof(cubeVertices);
        //m_CubeVbv.StrideInBytes = sizeof(VertexPositionColour);

        //m_CubeIbv.BufferLocation = m_CubeVbv.BufferLocation + sizeof(cubeVertices);
        //m_CubeIbv.SizeInBytes = sizeof(cubeIndices);
        //m_CubeIbv.Format = DXGI_FORMAT_R32_UINT;

        //m_QuadVbv.BufferLocation = m_CubeIbv.BufferLocation + sizeof(cubeIndices);
        //m_QuadVbv.SizeInBytes = sizeof(quadVertices);
        //m_QuadVbv.StrideInBytes = sizeof(VertexPositionUV);

        //return S_OK;
    }

    HRESULT Renderer::DX12_CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap)
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

        result = RenderContext::GetGraphicsDevice()->CreateCommittedResource(&heapTextureProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Texture));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create committed resource for texture.");
            return result;
        }

        // Create the GPU upload buffer.
        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, 1);
        const auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const auto uploadBufferSizeDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
        ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &uploadBufferSizeDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap)));

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
        std::vector<UINT8> texture = GenerateTextureData();

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = static_cast<LONG_PTR>(TextureWidth) * TextureBytesPerPixel;
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
        RenderContext::GetGraphicsDevice()->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRVHeap->GetCPUDescriptorHandleForHeapStart());

        return S_OK;
    }

    // Generate a simple black and white checkerboard texture.
    std::vector<UINT8> Renderer::GenerateTextureData()
    {
        const UINT rowPitch = TextureWidth * TextureBytesPerPixel;
        const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
        const UINT cellHeight = TextureWidth >> 3;    // The height of a cell in the checkerboard texture.
        const UINT textureSize = rowPitch * TextureHeight;

        std::vector<UINT8> data(textureSize);
        UINT8 *pData = &data[0];

        for (UINT n = 0; n < textureSize; n += TextureBytesPerPixel)
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

    HRESULT Renderer::DX12_CreateFrameSyncObjects()
    {
        HRESULT result;

        result = RenderContext::GetGraphicsDevice()->CreateFence(m_FenceValues[m_FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));

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

    HRESULT Renderer::DX12_MoveToNextFrame()
    {
        HRESULT result;

        const UINT64 fenceValue = m_FenceValues[m_FrameIndex];

        result = m_CommandQueue->Signal(m_Fence.Get(), fenceValue);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to signal fence value: %d", fenceValue);
            return result;
        }

        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        // If the next frame is not ready to be rendered yet, wait until it is ready.
        if (m_Fence->GetCompletedValue() < m_FenceValues[m_FrameIndex])
        {
            result = m_Fence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_FenceEvent);
            if (FAILED(result))
            {
                RXN_LOGGER::Error(L"Failed to set new event on completion.");
                return result;
            }

            WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
        }

        m_FenceValues[m_FrameIndex] = fenceValue + 1;
    }

    void Renderer::DX12_WaitForGPUFence()
    {
        // Schedule a Signal command in the queue.
        ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_FenceValues[m_FrameIndex]));

        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_FenceEvent));
        WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

        m_FenceValues[m_FrameIndex]++;
    }

    void Renderer::DX12_ToggleEffect(Mapped::EffectPipelineType type)
    {
        if (m_EnabledEffects[type])
        {
            DX12_WaitForGPUFence();
            m_PipelineLibrary.DestroyShader(type);
        }

        m_EnabledEffects[type] = !m_EnabledEffects[type];
    }

    void Renderer::DX12_PopulateCommandList()
    {
        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        ThrowIfFailed(m_CommandAllocators[m_FrameIndex]->Reset());

        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        ThrowIfFailed(m_CommandList->Reset(m_CommandAllocators[m_FrameIndex].Get(), nullptr));

        // Set necessary state.
        m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

        ID3D12DescriptorHeap *ppHeaps[] = { m_SRVHeap.Get() };
        m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        //m_CommandList->SetGraphicsRootDescriptorTable(0, m_SRVHeap->GetGPUDescriptorHandleForHeapStart());
        m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_CommandList->RSSetViewports(1, &m_Viewport);
        m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RTVDescriptorSize);
        CD3DX12_CPU_DESCRIPTOR_HANDLE intermediateRtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), Constants::Graphics::BUFFER_COUNT, m_RTVDescriptorSize);
        CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_SRVHeap->GetGPUDescriptorHandleForHeapStart());

        m_CommandList->OMSetRenderTargets(1, &intermediateRtvHandle, FALSE, nullptr);

        // Record commands.
        m_CommandList->ClearRenderTargetView(intermediateRtvHandle, Constants::Graphics::INTERMEDIATE_CLEAR_COLOUR, 0, nullptr);

        {
            static float rot = 0.0f;
            DrawConstantBuffer *drawCB = (DrawConstantBuffer *)m_DynamicConstantBuffer.GetMappedMemory(m_DrawIndex, m_FrameIndex);
            drawCB->worldViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(rot) * DirectX::XMMatrixRotationX(-rot) * m_Camera.GetViewMatrix() * m_ProjectionMatrix);

            rot += 0.01f;

            m_CommandList->IASetVertexBuffers(0, 1, &m_Shape.m_VertexBufferView);
            m_CommandList->IASetIndexBuffer(&m_Shape.m_IndexBufferView);
            m_PipelineLibrary.SetPipelineState(RenderContext::GetGraphicsDevice().Get(), m_RootSignature.Get(), m_CommandList.Get(), Mapped::BaseNormal3DRender, m_FrameIndex);

            m_CommandList->SetGraphicsRootConstantBufferView(RootParameterCB, m_DynamicConstantBuffer.GetGpuVirtualAddress(m_DrawIndex, m_FrameIndex));
            m_CommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
            m_DrawIndex++;
        }

        // Set up the state for a fullscreen quad.
        m_CommandList->IASetVertexBuffers(0, 1, &m_Quad.GetBufferView());
        m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        // Indicate that the back buffer will be used as a render target and the
        // intermediate render target will be used as a SRV.
        D3D12_RESOURCE_BARRIER barriers[] = {
            CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
            CD3DX12_RESOURCE_BARRIER::Transition(m_IntermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
        };

        m_CommandList->ResourceBarrier(_countof(barriers), barriers);
        m_CommandList->SetGraphicsRootDescriptorTable(RootParameterSRV, m_SRVHeap->GetGPUDescriptorHandleForHeapStart());

        const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        m_CommandList->ClearRenderTargetView(rtvHandle, black, 0, nullptr);
        m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

        {
            UINT quadCount = 0;
            static const UINT quadsX = 4;
            static const UINT quadsY = 4;

            // Cycle through all of the effects.
            for (UINT i = Mapped::PostBlit; i < Mapped::EffectPipelineTypeCount; i++)
            {
                if (m_EnabledEffects[i])
                {
                    CD3DX12_VIEWPORT viewport(
                        (quadCount % quadsX) * (m_Viewport.Width / quadsX),
                        (quadCount / quadsY) * (m_Viewport.Height / quadsY),
                        m_Viewport.Width / quadsX,
                        m_Viewport.Height / quadsY);

                    m_CommandList->RSSetViewports(1, &viewport);
                    m_PipelineLibrary.SetPipelineState(RenderContext::GetGraphicsDevice().Get(), m_RootSignature.Get(), m_CommandList.Get(), static_cast<Mapped::EffectPipelineType>(i), m_FrameIndex);
                    m_CommandList->DrawInstanced(4, 1, 0, 0);
                }

                quadCount++;
            }
        }

        // Revert resource states back to original values.
        barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        m_CommandList->ResourceBarrier(_countof(barriers), barriers);

        ThrowIfFailed(m_CommandList->Close());
    }

    void Renderer::DX12_Destroy()
    {
        DX12_WaitForGPUFence();
        CloseHandle(m_FenceEvent);
    }

    void Renderer::DX12_Render()
    {
        // Record all the commands we need to render the scene into the command list.
        DX12_PopulateCommandList();

        // Execute the command list.
        ID3D12CommandList *ppCommandLists[] = { m_CommandList.Get() };
        m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame.
        ThrowIfFailed(m_SwapChain->Present(1, 0));

        m_DrawIndex = 0;
        m_PipelineLibrary.EndFrame();

        DX12_MoveToNextFrame();

    }

    void Renderer::DX12_Update()
    {
        // Wait for the previous Present to complete.
        //WaitForSingleObjectEx(m_SwapChainEvent, 100, FALSE);

        Engine::EngineContext::Tick(NULL);
        m_Camera.Update(static_cast<float>(Engine::EngineContext::GetElapsedSeconds()));
    }








}
