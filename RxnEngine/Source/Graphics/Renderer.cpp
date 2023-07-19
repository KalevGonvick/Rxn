#include "Rxn.h"
#include "Renderer.h"
#include "DescriptorHeapDesc.h"

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
        , m_CommandQueueManager(1)
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        m_AssetsPath = assetsPath;
        m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
        memset(m_EnabledEffects, true, sizeof(m_EnabledEffects));
    }

    Renderer::~Renderer() = default;



    HRESULT Renderer::CreateDescriptorHeaps()
    {
        DescriptorHeapDesc rtvDesc(Constants::Graphics::BUFFER_COUNT + 1);
        rtvDesc.CreateRTVDescriptorHeap(m_RTVHeap);

        DescriptorHeapDesc srvDesc(1);
        srvDesc.CreateSRVDescriptorHeap(m_SRVHeap);

        m_RTVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_SRVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);



        return S_OK;
    }

    HRESULT Renderer::CreateCommandAllocators()
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

    HRESULT Renderer::CreateRootSignature()
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

    HRESULT Renderer::CreateCommandList()
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

    HRESULT Renderer::CreateVertexBufferResource()
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

        result = m_Shape.UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), m_CommandQueueManager.GetCommandQueue().Get(), m_CommandAllocators[m_FrameIndex].Get(), m_CommandList.Get());
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

        result = m_Quad.UploadGpuResources(RenderContext::GetGraphicsDevice().Get(), m_CommandQueueManager.GetCommandQueue().Get(), m_CommandAllocators[m_FrameIndex].Get(), m_CommandList.Get());
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to upload quad resources to gpu");
            return result;
        }

        return S_OK;
    }

    HRESULT Renderer::CreatePipelineSwapChain()
    {
        HRESULT result;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = Constants::Graphics::BUFFER_COUNT;
        swapChainDesc.Width = m_Width;
        swapChainDesc.Height = m_Height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        // It is recommended to always use the tearing flag when it is available.
        swapChainDesc.Flags = m_HasTearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;


        ComPointer<IDXGISwapChain1> swapChain;
        result = RenderContext::GetFactory()->CreateSwapChainForHwnd(m_CommandQueueManager.GetCommandQueue().Get(), RenderContext::GetHWND(), &swapChainDesc, nullptr, nullptr, &swapChain);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create a swap chain for window.");
            return result;
        }

        if (m_HasTearingSupport)
        {
            // This sample does not support fullscreen transitions.
            result = RenderContext::GetFactory()->MakeWindowAssociation(RenderContext::GetHWND(), DXGI_MWA_NO_ALT_ENTER);
            if (FAILED(result))
            {
                RXN_LOGGER::Error(L"Factory failed to set window option.");
                return result;
            }
        }


        /* Cast */
        result = swapChain->QueryInterface(&m_SwapChain);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to cast swapchain.");
        }

        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
        m_SwapChainEvent = m_SwapChain->GetFrameLatencyWaitableObject();

        return result;
    }

    HRESULT Renderer::CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap)
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

    HRESULT Renderer::CreateFrameSyncObjects()
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

    HRESULT Renderer::MoveToNextFrame()
    {
        HRESULT result;

        const UINT64 fenceValue = m_FenceValues[m_FrameIndex];

        result = m_CommandQueueManager.GetCommandQueue()->Signal(m_Fence.Get(), fenceValue);
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

    void Renderer::WaitForBufferedFence()
    {
        // Schedule a Signal command in the queue.
        ThrowIfFailed(m_CommandQueueManager.GetCommandQueue()->Signal(m_Fence.Get(), m_FenceValues[m_FrameIndex]));

        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_FenceEvent));
        WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

        m_FenceValues[m_FrameIndex]++;
    }

    void Renderer::WaitForSingleFrame()
    {
        const UINT64 fence = m_FenceValues[m_FrameIndex];
        ThrowIfFailed(m_CommandQueueManager.GetCommandQueue()->Signal(m_Fence.Get(), fence));
        m_FenceValues[m_FrameIndex]++;

        if (m_Fence->GetCompletedValue() < fence)
        {
            ThrowIfFailed(m_Fence->SetEventOnCompletion(fence, m_FenceEvent));
            WaitForSingleObject(m_FenceEvent, INFINITE);
        }
    }

    void Renderer::ToggleEffect(Mapped::EffectPipelineType type)
    {
        if (m_EnabledEffects[type])
        {
            WaitForBufferedFence();
            m_PipelineLibrary.DestroyShader(type);
        }

        m_EnabledEffects[type] = !m_EnabledEffects[type];
    }

    void Renderer::PopulateCommandList()
    {

    }








}
