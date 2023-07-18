#include "Rxn.h"
#include "SimulationWindow.h"

namespace Rxn::Graphics
{
    SimulationWindow::SimulationWindow(WString windowTitle, WString windowClass, int width, int height)
        : Platform::Win32::Window(windowTitle, windowClass)
        , Renderer(width, height)
    {
    }

    SimulationWindow::~SimulationWindow() = default;

    LRESULT SimulationWindow::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_KEYUP:
        {
            HandleKeyDown(static_cast<UINT8>(wParam));
            return 0;
        }
        case WM_KEYDOWN:
        {
            HandleKeyUp(static_cast<UINT8>(wParam));
            return 0;
        }
        case WM_SIZE:
        {
            if (m_Initialized)
                OnSizeChange();

            return 0;
        }
        default:
            break;
        }

        return Window::MessageHandler(hWnd, msg, wParam, lParam);
    }

    void SimulationWindow::SetupWindow()
    {
        m_Size = SIZE(1920, 1080);
        m_AddCloseButton = true;
        m_AddMaximizeButton = true;
        m_AddMinimizeButton = true;
        m_WindowStyle = WS_SIZEBOX;

        RegisterComponentClass();
        InitializeWin32();

        HRESULT result;

        result = DX12_LoadPipeline();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Error loading pipeline, exiting...");
            throw std::exception("Error loading pipeline, exiting...");
        }

        result = CreatePipelineSwapChain();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Error creating swapchain...");
            throw std::exception("Error creating swapchain...");
        }

        result = DX12_CreateCommandAllocators();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Error creating frame resources...");
            throw std::exception("Error creating frame resources...");
        }

        result = DX12_LoadAssets();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to load assets...");
            throw std::exception("Failed to load assets...");
        }

        m_Camera.Init({ 0.0f, 0.0f, 5.0f });
        m_Camera.SetMoveSpeed(1.0f);

        m_ProjectionMatrix = m_Camera.GetProjectionMatrix(0.8f, m_AspectRatio);

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        DX12_WaitForGPUFence();

        m_Initialized = true;

        ShowWindow(m_pHWnd, SW_SHOW);
        UpdateWindow(m_pHWnd);
    }

    void SimulationWindow::HandleKeyDown(uint8 key)
    {
        m_Camera.OnKeyDown(key);
    }

    void SimulationWindow::HandleKeyUp(uint8 key)
    {
        m_Camera.OnKeyUp(key);

        switch (key)
        {
        case 'C':
            DX12_WaitForGPUFence();
            m_PipelineLibrary.ClearPSOCache();
            m_PipelineLibrary.Build(RenderContext::GetGraphicsDevice().Get(), m_RootSignature.Get());
            break;

        case 'U':
            m_PipelineLibrary.ToggleUberShader();
            break;

        case 'L':
            m_PipelineLibrary.ToggleDiskLibrary();
            break;

        case 'M':
            m_PipelineLibrary.SwitchPSOCachingMechanism();
            break;

        case '1':
            DX12_ToggleEffect(Mapped::PostBlit);
            break;

        case '2':
            DX12_ToggleEffect(Mapped::PostInvert);
            break;

        case '3':
            DX12_ToggleEffect(Mapped::PostGrayScale);
            break;

        case '4':
            DX12_ToggleEffect(Mapped::PostEdgeDetect);
            break;

        case '5':
            DX12_ToggleEffect(Mapped::PostBlur);
            break;

        case '6':
            DX12_ToggleEffect(Mapped::PostWarp);
            break;

        case '7':
            DX12_ToggleEffect(Mapped::PostPixelate);
            break;

        case '8':
            DX12_ToggleEffect(Mapped::PostDistort);
            break;

        case '9':
            DX12_ToggleEffect(Mapped::PostWave);
            break;

        default:
            break;
        }
    }

    HRESULT SimulationWindow::CreatePipelineSwapChain()
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

        ComPointer<IDXGISwapChain1> swapChain;
        result = RenderContext::GetFactory()->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_pHWnd, &swapChainDesc, nullptr, nullptr, &swapChain);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create a swap chain for window.");
            return result;
        }

        if (m_HasTearingSupport)
        {
            // This sample does not support fullscreen transitions.
            result = RenderContext::GetFactory()->MakeWindowAssociation(m_pHWnd, DXGI_MWA_NO_ALT_ENTER);
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

    void SimulationWindow::DestroySwapChainResources()
    {
        for (int i = 0; i < Constants::Graphics::BUFFER_COUNT; ++i)
        {
            m_RenderTargets[i].Release();
            m_IntermediateRenderTarget.Release();
        }

        DX12_WaitForGPUFence();
    }

    HRESULT SimulationWindow::OnSizeChange()
    {
        RECT clientRect;
        GetClientRect(m_pHWnd, &clientRect);
        LONG newWidth = clientRect.right - clientRect.left;
        LONG newHeight = clientRect.bottom - clientRect.top;

        //
        // If the window size changed, resize our swapchain and recreate swapchain resources.
        //
        if (newWidth != m_Width || newHeight != m_Height)
        {
            m_Height = newHeight;
            m_Width = newWidth;

            DestroySwapChainResources();

            float viewWidthRatio = static_cast<float>(m_Resolutions[1].Width) / m_Width;
            float viewHeightRatio = static_cast<float>(m_Resolutions[1].Height) / m_Height;

            float x = 1.0f;
            float y = 1.0f;

            if (viewWidthRatio < viewHeightRatio)
            {
                // The scaled image's height will fit to the viewport's height and 
                // its width will be smaller than the viewport's width.
                x = viewWidthRatio / viewHeightRatio;
            }
            else
            {
                // The scaled image's width will fit to the viewport's width and 
                // its height may be smaller than the viewport's height.
                y = viewHeightRatio / viewWidthRatio;
            }

            m_Viewport.TopLeftX = m_Width * (1.0f - x) / 2.0f;
            m_Viewport.TopLeftY = m_Height * (1.0f - y) / 2.0f;
            m_Viewport.Width = x * m_Width;
            m_Viewport.Height = y * m_Height;

            m_ScissorRect.right = m_Width;
            m_ScissorRect.bottom = m_Height;

            HRESULT result = m_SwapChain->ResizeBuffers(Constants::Graphics::BUFFER_COUNT, newWidth, newHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
            if (FAILED(result))
            {
                RXN_LOGGER::Error(L"Failed to resize back buffer, hr=0x.8x", result);
                return result;
            }

            m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
            m_RTVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (UINT n = 0; n < Constants::Graphics::BUFFER_COUNT; n++)
            {

                result = m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n]));
                if (FAILED(result))
                {
                    RXN_LOGGER::Error(L"Failed to create RTV for buffer %d", n);
                    return result;
                }
                RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_RTVDescriptorSize);
                NAME_D3D12_OBJECT_INDEXED(m_RenderTargets, n);
            }

            D3D12_RESOURCE_DESC renderTargetDesc = m_RenderTargets[m_FrameIndex]->GetDesc();
            D3D12_CLEAR_VALUE clearValue = {};
            memcpy(clearValue.Color, Constants::Graphics::INTERMEDIATE_CLEAR_COLOUR, sizeof(Constants::Graphics::INTERMEDIATE_CLEAR_COLOUR));
            clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

            // Create an intermediate render target that is the same dimensions as the swap chain.
            const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &renderTargetDesc,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                &clearValue,
                IID_PPV_ARGS(&m_IntermediateRenderTarget)));

            NAME_D3D12_OBJECT(m_IntermediateRenderTarget);

            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_IntermediateRenderTarget.Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_RTVDescriptorSize);

            // Create a SRV of the intermediate render target.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = renderTargetDesc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_SRVHeap->GetCPUDescriptorHandleForHeapStart());
            RenderContext::GetGraphicsDevice()->CreateShaderResourceView(m_IntermediateRenderTarget.Get(), &srvDesc, srvHandle);
        }

        return S_OK;
    }








}
