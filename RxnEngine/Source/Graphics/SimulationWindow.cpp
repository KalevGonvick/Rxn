#include "Rxn.h"
#include "SimulationWindow.h"
#include "DescriptorHeapDesc.h"

namespace Rxn::Graphics
{
    SimulationWindow::SimulationWindow(WString windowTitle, WString windowClass, int width, int height)
        : Platform::Win32::Window(windowTitle, windowClass)
        , Renderer(width, height)
        , m_LastDrawTime(0)
        , m_FrameCount(0)
    {
    }

    SimulationWindow::~SimulationWindow() = default;


    void SimulationWindow::ShutdownRender()
    {
        m_RenderFence.SignalFence(m_CommandQueueManager.GetCommandQueue("Basic"), m_FrameIndex);
        m_RenderFence.WaitInfinite(m_FrameIndex);
        m_RenderFence.IncrementFenceValue(m_FrameIndex);
        m_RenderFence.Shutdown();
    }

    void SimulationWindow::DestroySwapChainResources()
    {
        m_RenderTargets[SwapChainBuffers::BUFFER_ONE].Release();
        m_RenderTargets[SwapChainBuffers::BUFFER_TWO].Release();
        m_IntermediateRenderTarget.Release();

        m_RenderFence.SignalFence(m_CommandQueueManager.GetCommandQueue("Basic"), m_FrameIndex);
        m_RenderFence.WaitInfinite(m_FrameIndex);
        m_RenderFence.IncrementFenceValue(m_FrameIndex);
    }

    uint32 SimulationWindow::GetFPS()
    {
        return Engine::EngineContext::GetFramesPerSecond();
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

        InitializeRender();

        m_Initialized = true;

        ShowWindow(m_HWnd, SW_SHOW);
        UpdateWindow(m_HWnd);
    }

    void SimulationWindow::InitializeRender()
    {
        RenderContext::SetHWND(m_HWnd);

        HRESULT result;

        m_CommandQueueManager.CreateCommandQueue("Basic");

        {
            /* RTV + Intermediate target view*/
            DescriptorHeapDesc rtvDesc(SwapChainBuffers::TOTAL_BUFFERS + 1);
            rtvDesc.CreateRTVDescriptorHeap(m_RTVHeap);

            DescriptorHeapDesc srvDesc(1);
            srvDesc.CreateSRVDescriptorHeap(m_SRVHeap);

            m_RTVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            m_SRVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }


        //result = Renderer::CreateDescriptorHeaps();

        result = Renderer::CreateRootSignature();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create root signature.");
        }

        m_SwapChain.SetTearingSupport(m_HasTearingSupport);
        m_SwapChain.CreateSwapChain(m_CommandQueueManager.GetCommandQueue("Basic").Get());

        m_FrameIndex = m_SwapChain.GetCurrentBackBufferIndex();
        m_SwapChainEvent = m_SwapChain.GetFrameLatencyWaitableObject();

        if (m_HasTearingSupport)
        {
            ThrowIfFailed(RenderContext::GetFactory()->MakeWindowAssociation(RenderContext::GetHWND(), DXGI_MWA_NO_ALT_ENTER));
        }

        ThrowIfFailed(Renderer::CreateCommandAllocators());


        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame swap chain buffer
            ThrowIfFailed(m_SwapChain.GetBuffer(SwapChainBuffers::BUFFER_ONE, m_RenderTargets[SwapChainBuffers::BUFFER_ONE]));
            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_RenderTargets[SwapChainBuffers::BUFFER_ONE].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_RTVDescriptorSize);
            NAME_D3D12_OBJECT_INDEXED(m_RenderTargets, SwapChainBuffers::BUFFER_ONE);

            ThrowIfFailed(m_SwapChain.GetBuffer(SwapChainBuffers::BUFFER_TWO, m_RenderTargets[SwapChainBuffers::BUFFER_TWO]));
            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_RenderTargets[SwapChainBuffers::BUFFER_TWO].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_RTVDescriptorSize);
            NAME_D3D12_OBJECT_INDEXED(m_RenderTargets, SwapChainBuffers::BUFFER_TWO);


            D3D12_CLEAR_VALUE clearValue = {};
            memcpy(clearValue.Color, INTERMEDIATE_CLEAR_COLOUR, sizeof(INTERMEDIATE_CLEAR_COLOUR));
            clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

            // Create an intermediate render target that is the same dimensions as the swap chain.
            auto renderTargetDescCopy = m_RenderTargets[m_FrameIndex]->GetDesc();
            const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &renderTargetDescCopy, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&m_IntermediateRenderTarget)));


            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_IntermediateRenderTarget.Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_RTVDescriptorSize);
            NAME_D3D12_OBJECT(m_IntermediateRenderTarget);

            // Create a SRV of the intermediate render target.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = renderTargetDescCopy.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_SRVHeap->GetCPUDescriptorHandleForHeapStart());
            RenderContext::GetGraphicsDevice()->CreateShaderResourceView(m_IntermediateRenderTarget.Get(), &srvDesc, srvHandle);
            ThrowIfFailed(Renderer::CreateCommandList());
        }

        {
            m_CommandListManager.CreateCommandList("Setup", m_CommandAllocators[m_FrameIndex]);


            ThrowIfFailed(Renderer::CreateVertexBufferResource());

            m_DynamicConstantBuffer.Init(RenderContext::GetGraphicsDevice().Get());

            // Close the command list and execute it to begin the initial GPU setup.
            ThrowIfFailed(m_CommandListManager.GetCommandList("Setup")->Close());
            ID3D12CommandList *ppCommandLists[] = { m_CommandListManager.GetCommandList("Setup").Get() };
            m_CommandQueueManager.GetCommandQueue("Basic")->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


        }

        m_RenderFence.CreateFence(m_FrameIndex);

        const uint32 nextFrameIndex = m_SwapChain.GetCurrentBackBufferIndex();
        m_RenderFence.MoveFenceMarker(m_CommandQueueManager.GetCommandQueue("Basic").Get(), m_FrameIndex, nextFrameIndex);
        m_FrameIndex = nextFrameIndex;
        
        
        m_PipelineLibrary.Build(RenderContext::GetGraphicsDevice().Get(), m_RootSignature.Get());
        m_Camera.Init({ 0.0f, 0.0f, 5.0f });
        m_Camera.SetMoveSpeed(1.0f);
        m_ProjectionMatrix = m_Camera.GetProjectionMatrix(0.8f, m_AspectRatio);
        m_RenderFence.SignalFence(m_CommandQueueManager.GetCommandQueue("Basic"), m_FrameIndex);
        m_RenderFence.WaitInfinite(m_FrameIndex);
        m_RenderFence.IncrementFenceValue(m_FrameIndex);
    }

    /* -------------------------------------------------------- */
    /*  WIN-API                                                 */
    /* -------------------------------------------------------- */
#pragma region WIN-API

    LRESULT SimulationWindow::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_KEYUP:
        {
            HandleKeyDown(static_cast<UINT8>(wParam));
            return 0;
        }
        //case WM_SIZE:
        //{
        //    OnSizeChange();
        //    return 0;
        //}
        case WM_KEYDOWN:
        {
            HandleKeyUp(static_cast<UINT8>(wParam));
            return 0;
        }
        default:
            break;
        }

        return Window::MessageHandler(hWnd, msg, wParam, lParam);
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
            m_RenderFence.SignalFence(m_CommandQueueManager.GetCommandQueue("Basic"), m_FrameIndex);
            m_RenderFence.WaitInfinite(m_FrameIndex);
            m_RenderFence.IncrementFenceValue(m_FrameIndex);
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
            ToggleEffect(Mapped::PostBlit);
            break;

        case '2':
            ToggleEffect(Mapped::PostInvert);
            break;

        case '3':
            ToggleEffect(Mapped::PostGrayScale);
            break;

        case '4':
            ToggleEffect(Mapped::PostEdgeDetect);
            break;

        case '5':
            ToggleEffect(Mapped::PostBlur);
            break;

        case '6':
            ToggleEffect(Mapped::PostWarp);
            break;

        case '7':
            ToggleEffect(Mapped::PostPixelate);
            break;

        case '8':
            ToggleEffect(Mapped::PostDistort);
            break;

        case '9':
            ToggleEffect(Mapped::PostWave);
            break;

        default:
            break;
        }
    }

#pragma endregion // WIN-API
    /* -------------------------------------------------------- */


    HRESULT SimulationWindow::OnSizeChange()
    {
        RECT clientRect;
        GetClientRect(m_HWnd, &clientRect);
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

            ThrowIfFailed(m_SwapChain.ResizeBuffers(SwapChainBuffers::TOTAL_BUFFERS, newWidth, newHeight));

            m_FrameIndex = m_SwapChain.GetCurrentBackBufferIndex();
            m_RTVDescriptorSize = RenderContext::GetGraphicsDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (UINT n = 0; n < SwapChainBuffers::TOTAL_BUFFERS; n++)
            {

                ThrowIfFailed(m_SwapChain.GetBuffer(n, m_RenderTargets[n]));
                RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_RTVDescriptorSize);
                NAME_D3D12_OBJECT_INDEXED(m_RenderTargets, n);
            }

            D3D12_RESOURCE_DESC renderTargetDesc = m_RenderTargets[m_FrameIndex]->GetDesc();
            D3D12_CLEAR_VALUE clearValue = {};
            memcpy(clearValue.Color, INTERMEDIATE_CLEAR_COLOUR, sizeof(INTERMEDIATE_CLEAR_COLOUR));
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

    void SimulationWindow::UpdateSimulation()
    {

        Engine::EngineContext::Tick();

        uint32 fps = GetFPS();
        m_Camera.Update(static_cast<float>(Engine::EngineContext::GetElapsedSeconds()));
    }


    void SimulationWindow::RenderPass()
    {

        // Present the frame.
        ThrowIfFailed(m_SwapChain.Present(1, 0));

        m_DrawIndex = 0;
        m_PipelineLibrary.EndFrame();

        MoveToNextFrame();
    }

    void SimulationWindow::PreRenderPass()
    {
        ThrowIfFailed(m_CommandAllocators[m_FrameIndex]->Reset());
        ThrowIfFailed(m_CommandListManager.GetCommandList("Main")->Reset(m_CommandAllocators[m_FrameIndex].Get(), nullptr));

        {
            m_CommandListManager.GetCommandList("Main")->SetGraphicsRootSignature(m_RootSignature.Get());

            ID3D12DescriptorHeap *ppHeaps[] = { m_SRVHeap.Get() };
            m_CommandListManager.GetCommandList("Main")->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

            m_CommandListManager.GetCommandList("Main")->RSSetViewports(1, &m_Viewport);
            m_CommandListManager.GetCommandList("Main")->RSSetScissorRects(1, &m_ScissorRect);

            /* This rotation could probably move to the update loop */
            static float rot = 0.0f;
            DrawConstantBuffer *drawCB = (DrawConstantBuffer *)m_DynamicConstantBuffer.GetMappedMemory(m_DrawIndex, m_FrameIndex);
            drawCB->worldViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(rot) * DirectX::XMMatrixRotationX(-rot) * m_Camera.GetViewMatrix() * m_ProjectionMatrix);
            rot += 0.01f;
        }


        {

            CD3DX12_CPU_DESCRIPTOR_HANDLE intermediateRtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), SwapChainBuffers::TOTAL_BUFFERS, m_RTVDescriptorSize);
            m_CommandListManager.GetCommandList("Main")->SetGraphicsRootConstantBufferView(RootParameterCB, m_DynamicConstantBuffer.GetGpuVirtualAddress(m_DrawIndex, m_FrameIndex));
            m_CommandListManager.GetCommandList("Main")->OMSetRenderTargets(1, &intermediateRtvHandle, FALSE, nullptr);
            m_CommandListManager.GetCommandList("Main")->ClearRenderTargetView(intermediateRtvHandle, INTERMEDIATE_CLEAR_COLOUR, 0, nullptr);

            m_PipelineLibrary.SetPipelineState(RenderContext::GetGraphicsDevice().Get(), m_RootSignature.Get(), m_CommandListManager.GetCommandList("Main").Get(), Mapped::BaseNormal3DRender, m_FrameIndex);

            m_CommandListManager.GetCommandList("Main")->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_CommandListManager.GetCommandList("Main")->IASetVertexBuffers(0, 1, &m_Shape.m_VertexBufferView);
            m_CommandListManager.GetCommandList("Main")->IASetIndexBuffer(&m_Shape.m_IndexBufferView);


            m_CommandListManager.GetCommandList("Main")->DrawIndexedInstanced(m_Shape.m_Indices.size(), 1, 0, 0, 0);
        }


        D3D12_RESOURCE_BARRIER barriers[2] = {};
        barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_IntermediateRenderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_CommandListManager.GetCommandList("Main")->ResourceBarrier(_countof(barriers), barriers);
        m_CommandListManager.GetCommandList("Main")->SetGraphicsRootDescriptorTable(RootParameterSRV, m_SRVHeap->GetGPUDescriptorHandleForHeapStart());

        {

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RTVDescriptorSize);
            m_CommandListManager.GetCommandList("Main")->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
            m_CommandListManager.GetCommandList("Main")->ClearRenderTargetView(rtvHandle, INTERMEDIATE_CLEAR_COLOUR, 0, nullptr);
            m_PipelineLibrary.SetPipelineState(RenderContext::GetGraphicsDevice().Get(), m_RootSignature.Get(), m_CommandListManager.GetCommandList("Main").Get(), Mapped::PostBlit, m_FrameIndex);
            m_CommandListManager.GetCommandList("Main")->IASetVertexBuffers(0, 1, &m_Quad.m_QuadBufferView);
            m_CommandListManager.GetCommandList("Main")->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            m_CommandListManager.GetCommandList("Main")->DrawInstanced(m_Quad.m_Quads.size(), 1, 0, 0);
        }

        // Revert resource states back to original values.
        barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        m_CommandListManager.GetCommandList("Main")->ResourceBarrier(_countof(barriers), barriers);
        m_CommandListManager.CloseCommandList("Main");
        m_CommandListManager.ExecuteCommandList("Main", m_CommandQueueManager.GetCommandQueue("Basic"));

    }

    void SimulationWindow::PostRenderPass()
    {
        m_DrawIndex++;
    }








}
