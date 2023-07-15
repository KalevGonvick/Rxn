#include "Rxn.h"
#include "SimulationWindow.h"

namespace Rxn::Graphics
{
    SimulationWindow::SimulationWindow(WString windowTitle, WString windowClass, int width, int height)
        : Platform::Win32::Window(windowTitle, windowClass)
        , RenderFramework(width, height)
    {
    }

    SimulationWindow::~SimulationWindow() = default;

    void SimulationWindow::Render()
    {
        DX12_Render();
    }

    LRESULT SimulationWindow::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_SIZE:
        {
            if (m_IsRenderReady)
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
        m_Size = SIZE(1280, 720);
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

        result = DX12_CreateFrameResources();
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

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        DX12_WaitForGPUFence();

        m_IsRenderReady = true;

        ShowWindow(m_pHWnd, SW_SHOW);
        UpdateWindow(m_pHWnd);
    }

    HRESULT SimulationWindow::CreatePipelineSwapChain()
    {
        HRESULT result;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = Constants::Graphics::SLIDE_COUNT;
        swapChainDesc.Width = m_Width;
        swapChainDesc.Height = m_Height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPointer<IDXGISwapChain1> swapChain;
        result = m_Factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_pHWnd, &swapChainDesc, nullptr, nullptr, &swapChain);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create a swap chain for window.");
            return result;
        }

        // This sample does not support fullscreen transitions.
        result = m_Factory->MakeWindowAssociation(m_pHWnd, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Factory failed to set window option.");
            return result;
        }

        /* Cast */
        result = swapChain->QueryInterface(&m_SwapChain);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to cast swapchain.");
        }

        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        return result;
    }

    void SimulationWindow::DestroySwapChainResources()
    {
        for (int i = 0; i < Constants::Graphics::SLIDE_COUNT; ++i)
        {
            m_RenderTargets[i].Release();
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

            HRESULT result = m_SwapChain->ResizeBuffers(Constants::Graphics::SLIDE_COUNT, newWidth, newHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
            if (FAILED(result))
            {
                RXN_LOGGER::Error(L"Failed to resize back buffer, hr=0x.8x", result);
                return result;
            }

            m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
            m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

            result;
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
            }

            if (FAILED(result))
            {
                RXN_LOGGER::Error(L"Failed to recreate swap chain resources! hr=0x.8x", result);
                return result;
            }
        }

        return S_OK;
    }








}
