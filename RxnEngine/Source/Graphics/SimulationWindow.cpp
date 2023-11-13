#include "Rxn.h"
#include "SimulationWindow.h"
#include "DescriptorHeapDesc.h"

namespace Rxn::Graphics
{
    SimulationWindow::SimulationWindow(const WString &windowTitle, const WString &windowClass, int width, int height)
        : Renderer(width, height)
        , Platform::Win32::Window(windowTitle, windowClass)
    {
    }

    SimulationWindow::~SimulationWindow() = default;


    void SimulationWindow::ShutdownRender()
    {
        m_Fence.SignalFence(m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), m_Display.GetFrameIndex());
        m_Fence.WaitInfinite(m_Display.GetFrameIndex());
        m_Fence.IncrementFenceValue(m_Display.GetFrameIndex());
        m_Fence.Shutdown();
    }

    void SimulationWindow::DestroySwapChainResources()
    {
        m_Scene.ReleaseResourceViews();
        m_Fence.SignalFence(m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), m_Display.GetFrameIndex());
        m_Fence.WaitInfinite(m_Display.GetFrameIndex());
        m_Fence.IncrementFenceValue(m_Display.GetFrameIndex());
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




        //result = Renderer::CreateDescriptorHeaps();

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

        m_AllocatorPool.Create(RenderContext::GetGraphicsDevice());
        m_CommandAllocators[SwapChainBuffers::BUFFER_ONE] = m_AllocatorPool.RequestAllocator(SwapChainBuffers::BUFFER_ONE);
        m_CommandAllocators[SwapChainBuffers::BUFFER_TWO] = m_AllocatorPool.RequestAllocator(SwapChainBuffers::BUFFER_TWO);
        

        m_CommandQueueManager.CreateCommandQueue(GOHKeys::CmdQueue::PRIMARY);
        m_Scene.InitHeaps();
        m_Scene.SetSerializedRootSignature(rootSignatureDesc);

        m_Display.GetSwapChain().SetTearingSupport(m_HasTearingSupport);
        m_Display.GetSwapChain().CreateSwapChain(m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY).Get());
        m_Display.TurnOverSwapChainBuffer();

        {
            
            m_Scene.InitSceneRenderTargets(m_Display.GetFrameIndex(), m_Display.GetSwapChain());
            m_CommandListManager.CreateCommandList(GOHKeys::CmdList::PRIMARY, m_CommandAllocators[m_Display.GetFrameIndex()]);
            ThrowIfFailed(m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->Close());
            ID3D12CommandList *ppCommandLists[] = { m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY).Get() };
            m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY)->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

           
        }

        {
            m_CommandListManager.CreateCommandList(GOHKeys::CmdList::INIT, m_CommandAllocators[m_Display.GetFrameIndex()]);
        }

        {

            // Define the geometry for a cube.
            std::vector<VertexPositionColour> cubeVertices;
            cubeVertices.push_back(VertexPositionColour{ { -1.0f, 1.0f, -1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });
            cubeVertices.push_back(VertexPositionColour{ {  1.0f, 1.0f, -1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });
            cubeVertices.push_back(VertexPositionColour{ {  1.0f, 1.0f, 1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });
            cubeVertices.push_back(VertexPositionColour{ { -1.0f, 1.0f, 1.0f, 1.0f }, { GetRandomColour(), GetRandomColour(), GetRandomColour() } });

            cubeVertices.push_back(VertexPositionColour{ { -1.0f, -1.0f, -1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });
            cubeVertices.push_back(VertexPositionColour{ {  1.0f, -1.0f, -1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });
            cubeVertices.push_back(VertexPositionColour{ {  1.0f, -1.0f, 1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });
            cubeVertices.push_back(VertexPositionColour{ { -1.0f, -1.0f, 1.0f, 1.0f }, { GetRandomColour(),GetRandomColour(), GetRandomColour() } });

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

            m_Scene.AddShapeFromRaw(cubeVertices, cubeIndices, m_CommandAllocators[m_Display.GetFrameIndex()], m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY).Get(), m_CommandListManager.GetCommandList(GOHKeys::CmdList::INIT).Get());

            std::vector<VertexPositionUV> quadVertices;
            quadVertices.push_back(VertexPositionUV{ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } });
            quadVertices.push_back(VertexPositionUV{ { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } });
            quadVertices.push_back(VertexPositionUV{ { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } });
            quadVertices.push_back(VertexPositionUV{ { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } });

            m_Scene.AddQuadFromRaw(quadVertices, m_CommandAllocators[m_Display.GetFrameIndex()], m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY).Get(), m_CommandListManager.GetCommandList(GOHKeys::CmdList::INIT).Get());
        }
        {
            m_Scene.GetDynamicConstantBuffer().Init(RenderContext::GetGraphicsDevice().Get());

            // Close the command list and execute it to begin the initial GPU setup.
            ThrowIfFailed(m_CommandListManager.GetCommandList(GOHKeys::CmdList::INIT)->Close());
            ID3D12CommandList *ppCommandLists[] = { m_CommandListManager.GetCommandList(GOHKeys::CmdList::INIT).Get() };
            m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY)->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
           

        }

        m_Fence.CreateFence(m_Display.GetFrameIndex());
        const uint32 nextFrameIndex = m_Display.GetSwapChain().GetCurrentBackBufferIndex();
        m_Fence.MoveFenceMarker(m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY).Get(), m_Display.GetFrameIndex(), nextFrameIndex);
        m_Display.TurnOverSwapChainBuffer();
        
        m_PipelineLibrary.Build(RenderContext::GetGraphicsDevice().Get(), m_Scene.GetRootSignature());
        
        m_Display.UpdateProjectionMatrix(m_Scene.GetCamera());
        m_Fence.SignalFence(m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), m_Display.GetFrameIndex());
        m_Fence.WaitInfinite(m_Display.GetFrameIndex());
        m_Fence.IncrementFenceValue(m_Display.GetFrameIndex());

        
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
        m_Scene.GetCamera().OnKeyDown(key);
    }

    void SimulationWindow::HandleKeyUp(uint8 key)
    {
        m_Scene.GetCamera().OnKeyUp(key);

        switch (key)
        {
        case 'C':
            m_Fence.SignalFence(m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), m_Display.GetFrameIndex());
            m_Fence.WaitInfinite(m_Display.GetFrameIndex());
            m_Fence.IncrementFenceValue(m_Display.GetFrameIndex());
            m_PipelineLibrary.ClearPSOCache();
            m_PipelineLibrary.Build(RenderContext::GetGraphicsDevice().Get(), m_Scene.GetRootSignature().Get());
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


    void SimulationWindow::OnSizeChange()
    {
        RECT clientRect;
        GetClientRect(m_HWnd, &clientRect);
        int64 newWidth = clientRect.right - clientRect.left;
        int64 newHeight = clientRect.bottom - clientRect.top;

        //
        // If the window size changed, resize our swapchain and recreate swapchain resources.
        //
        if (newWidth != m_Display.GetWidth() || newHeight != m_Display.GetHeight())
        {
            DestroySwapChainResources();
            m_Display.HandleSizeChange(newWidth, newHeight);
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_Scene.GetRtvHeap()->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (uint32 n = 0; n < SwapChainBuffers::TOTAL_BUFFERS; n++)
            {

                ThrowIfFailed(m_Display.GetSwapChain().GetBuffer(n, m_Scene.GetRenderTarget(n)));
                RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_Scene.GetRenderTarget(n), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_Scene.GetRtvDescriptorHeapSize());
            }
            
            D3D12_RESOURCE_DESC renderTargetDesc = m_Scene.GetRenderTarget(m_Display.GetFrameIndex())->GetDesc();
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
                IID_PPV_ARGS(&m_Scene.GetIntermediateRenderTarget())));
            
            NAME_D3D12_OBJECT(m_Scene.GetIntermediateRenderTarget());

            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(m_Scene.GetIntermediateRenderTarget().Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_Scene.GetRtvDescriptorHeapSize());

            // Create a SRV of the intermediate render target.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = renderTargetDesc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_Scene.GetSrvHeap()->GetCPUDescriptorHandleForHeapStart());
            RenderContext::GetGraphicsDevice()->CreateShaderResourceView(m_Scene.GetIntermediateRenderTarget().Get(), &srvDesc, srvHandle);
        }

        }

    void SimulationWindow::UpdateSimulation()
    {

        Engine::EngineContext::Tick();

        uint32 fps = GetFPS();
        m_Scene.GetCamera().Update(static_cast<float>(Engine::EngineContext::GetElapsedSeconds()));
    }


    void SimulationWindow::RenderPass()
    {

        // Present the frame.
        ThrowIfFailed(m_Display.GetSwapChain().Present(1, 0));

        m_DrawIndex = 0;
        m_PipelineLibrary.EndFrame();

        const uint32 nextFrameIndex = m_Display.GetSwapChain().GetCurrentBackBufferIndex();
        m_Fence.MoveFenceMarker(m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), m_Display.GetFrameIndex(), nextFrameIndex);
        m_Display.TurnOverSwapChainBuffer();
    }

    void SimulationWindow::PreRenderPass()
    {
        ThrowIfFailed(m_CommandAllocators[m_Display.GetFrameIndex()]->Reset());
        ThrowIfFailed(m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->Reset(m_CommandAllocators[m_Display.GetFrameIndex()], nullptr));

        {
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->SetGraphicsRootSignature(m_Scene.GetRootSignature().Get());

            ID3D12DescriptorHeap *ppHeaps[] = { m_Scene.GetSrvHeap().Get() };
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->RSSetViewports(1, &m_Display.GetViewPort());
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->RSSetScissorRects(1, &m_Display.GetScissorRect());

            /* This rotation could probably move to the update loop */
            static float rot = 0.0f;
            DrawConstantBuffer *drawCB = (DrawConstantBuffer *) m_Scene.GetDynamicConstantBuffer().GetMappedMemory(m_DrawIndex, m_Display.GetFrameIndex());
            drawCB->worldViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(rot) * DirectX::XMMatrixRotationX(-rot) * m_Scene.GetCamera().GetViewMatrix() * m_Display.GetProjectionMatrix());
            rot += 0.01f;
        }


        {

            CD3DX12_CPU_DESCRIPTOR_HANDLE intermediateRtvHandle(m_Scene.GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), SwapChainBuffers::TOTAL_BUFFERS, m_Scene.GetRtvDescriptorHeapSize());
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->SetGraphicsRootConstantBufferView(RootParameterCB, m_Scene.GetDynamicConstantBuffer().GetGpuVirtualAddress(m_DrawIndex, m_Display.GetFrameIndex()));
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->OMSetRenderTargets(1, &intermediateRtvHandle, FALSE, nullptr);
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->ClearRenderTargetView(intermediateRtvHandle, INTERMEDIATE_CLEAR_COLOUR, 0, nullptr);

            m_PipelineLibrary.SetPipelineState(
                RenderContext::GetGraphicsDevice().Get(), 
                m_Scene.GetRootSignature(), 
                m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY).Get(), 
                Mapped::BaseNormal3DRender, 
                m_Display.GetFrameIndex());

            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->IASetVertexBuffers(0, 1, &m_Scene.GetShape().m_VertexBufferView);
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->IASetIndexBuffer(&m_Scene.GetShape().m_IndexBufferView);


            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->DrawIndexedInstanced(m_Scene.GetShape().m_Indices.size(), 1, 0, 0, 0);
        }


        D3D12_RESOURCE_BARRIER barriers[2] = {};
        barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(m_Scene.GetRenderTarget(m_Display.GetFrameIndex()).Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_Scene.GetIntermediateRenderTarget().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->ResourceBarrier(_countof(barriers), barriers);
        m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->SetGraphicsRootDescriptorTable(RootParameterSRV, m_Scene.GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());

        {

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_Scene.GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), m_Display.GetFrameIndex(), m_Scene.GetRtvDescriptorHeapSize());
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->ClearRenderTargetView(rtvHandle, INTERMEDIATE_CLEAR_COLOUR, 0, nullptr);
            m_PipelineLibrary.SetPipelineState(RenderContext::GetGraphicsDevice().Get(), m_Scene.GetRootSignature().Get(), m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY).Get(), Mapped::PostBlit, m_Display.GetFrameIndex());
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->IASetVertexBuffers(0, 1, &m_Scene.GetQuad().m_QuadBufferView);
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->DrawInstanced(m_Scene.GetQuad().m_Quads.size(), 1, 0, 0);
        }

        // Revert resource states back to original values.
        barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        m_CommandListManager.GetCommandList(GOHKeys::CmdList::PRIMARY)->ResourceBarrier(_countof(barriers), barriers);
        m_CommandListManager.CloseCommandList(GOHKeys::CmdList::PRIMARY);
        m_CommandListManager.ExecuteCommandList(GOHKeys::CmdList::PRIMARY, m_CommandQueueManager.GetCommandQueue(GOHKeys::CmdQueue::PRIMARY));

    }

    void SimulationWindow::PostRenderPass()
    {
        m_DrawIndex++;
    }








}
