#include "Rxn.h"
#include "SimulationWindow.h"
#include "DescriptorHeapDesc.h"

namespace Rxn::Graphics
{
    SimulationWindow::SimulationWindow(const WString &windowTitle, const WString &windowClass, int32 width, int32 height)
        : Renderer(width, height)
        , Platform::Win32::Window(windowTitle, windowClass, width, height)
    {
    }

    SimulationWindow::~SimulationWindow() = default;


    void SimulationWindow::ShutdownRender()
    {
        GetFence().ShutdownFence(
            GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), 
            GetDisplay().GetFrameIndex()
        );
    }


    
    uint64 SimulationWindow::GetFPS() const
    {
        return Engine::EngineContext::GetTimer().GetFramesPerSecond();
    }


    void SimulationWindow::SetupWindow()
    {
        //m_Size = SIZE(1920, 1080);
        m_AddCloseButton = true;
        m_AddMaximizeButton = true;
        m_AddMinimizeButton = true;
        m_WindowStyle = WS_OVERLAPPEDWINDOW;

        RegisterComponentClass();
        InitializeWin32();

        InitializeRender();

        ShowWindow(m_HWnd, SW_SHOW);
        UpdateWindow(m_HWnd);
        
    }

    void SimulationWindow::InitializeRender()
    {

        RenderContext::SetHWND(m_HWnd);
        
        GetCommandAllocatorPool().Create(RenderContext::GetGraphicsDevice());
        
        InitCommandAllocator(*this, SwapChainBuffers::BUFFER_ONE);
        InitCommandAllocator(*this, SwapChainBuffers::BUFFER_TWO);
        
        InitCommandQueues(*this, GOHKeys::CmdQueue::PRIMARY);
        InitRendererDisplay(*this, GOHKeys::CmdQueue::PRIMARY);
        InitCommandList(*this, GOHKeys::CmdQueue::PRIMARY, GOHKeys::CmdList::INIT);
        InitCommandList(*this, GOHKeys::CmdQueue::PRIMARY, GOHKeys::CmdList::PRIMARY);
        
        InitScene();
        InitCachedPipeline(*this, RenderContext::GetGraphicsDevice());
        LoadSceneData();
        InitGpuFence(*this, GOHKeys::CmdQueue::PRIMARY);
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
            HandleKeyDown(static_cast<uint8>(wParam));
            return 0;
        }
        case WM_KEYDOWN:
        {
            HandleKeyUp(static_cast<uint8>(wParam));
            return 0;
        }
        default:
            break;
        }

        return Window::MessageHandler(hWnd, msg, wParam, lParam);
    }

    void SimulationWindow::HandleKeyDown(uint8 key)
    {
        GetScene().GetCamera().OnKeyDown(key);
    }

    void SimulationWindow::HandleKeyUp(uint8 key)
    {
        GetScene().GetCamera().OnKeyUp(key);

        switch (key)
        {
        case 'C':
            GetFence().SignalFence(GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), GetDisplay().GetFrameIndex());
            GetFence().WaitInfinite(GetDisplay().GetFrameIndex());
            GetFence().IncrementFenceValue(GetDisplay().GetFrameIndex());
            GetPipelineLibrary().ClearPSOCache();
            GetPipelineLibrary().Build(RenderContext::GetGraphicsDevice().Get(), GetScene().GetRootSignature().Get());
            break;

        case 'U':
            RXN_LOGGER::Debug(L"U was pressed, toggling uber shader...");
            GetPipelineLibrary().ToggleUberShader();
            break;

        case 'L':
            RXN_LOGGER::Debug(L"L was pressed, switching to disk library...");
            GetPipelineLibrary().ToggleDiskLibrary();
            break;

        case 'M':
            RXN_LOGGER::Debug(L"M was pressed, switching to PSO caching...");
            GetPipelineLibrary().SwitchPSOCachingMechanism();
            break;

        default:
            break;
        }
    }

    void SimulationWindow::LoadSceneData()
    {
            
            //ResetFrameCommandObjects(GetCommandListManager().GetCommandList(GOHKeys::CmdList::INIT), GetCommandAllocator(GetDisplay().GetFrameIndex()));
            auto primaryCommandQueue = GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY);
            auto currentCommandAllocator = GetCommandAllocator(GetDisplay().GetFrameIndex());
            auto initCmdList = GetCommandListManager().GetCommandList(GOHKeys::CmdList::INIT);

            initCmdList->Reset(currentCommandAllocator, nullptr);
        

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

            GetScene().AddShapeFromRaw(cubeVertices, cubeIndices, currentCommandAllocator, primaryCommandQueue, initCmdList);

            std::vector<VertexPositionUV> quadVertices;
            quadVertices.push_back(VertexPositionUV{ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } });
            quadVertices.push_back(VertexPositionUV{ { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } });
            quadVertices.push_back(VertexPositionUV{ { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } });
            quadVertices.push_back(VertexPositionUV{ { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } });

            GetScene().AddQuadFromRaw(quadVertices, currentCommandAllocator, primaryCommandQueue, initCmdList);
            
            GetCommandListManager().CloseCommandList(GOHKeys::CmdList::INIT);
            GetCommandListManager().ExecuteCommandList(GOHKeys::CmdList::INIT, primaryCommandQueue);
        
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
        if (newWidth != GetDisplay().GetWidth() || newHeight != GetDisplay().GetHeight())
        {
            DestroySwapChainResources();
            GetDisplay().HandleSizeChange(static_cast<uint32>(newWidth), static_cast<uint32>(newHeight));
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetScene().GetRtvHeap()->GetCPUDescriptorHandleForHeapStart());

            ThrowIfFailed(GetDisplay().GetSwapChain().GetBuffer(SwapChainBuffers::BUFFER_ONE, GetScene().GetRenderTarget(SwapChainBuffers::BUFFER_ONE)));
            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(GetScene().GetRenderTarget(SwapChainBuffers::BUFFER_ONE), nullptr, rtvHandle);
            rtvHandle.Offset(1, GetScene().GetRtvDescriptorHeapSize());

            ThrowIfFailed(GetDisplay().GetSwapChain().GetBuffer(SwapChainBuffers::BUFFER_TWO, GetScene().GetRenderTarget(SwapChainBuffers::BUFFER_TWO)));
            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(GetScene().GetRenderTarget(SwapChainBuffers::BUFFER_TWO), nullptr, rtvHandle);
            rtvHandle.Offset(1, GetScene().GetRtvDescriptorHeapSize());
            
            D3D12_RESOURCE_DESC renderTargetDesc = GetScene().GetRenderTarget(GetDisplay().GetFrameIndex())->GetDesc();
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
                IID_PPV_ARGS(&GetScene().GetIntermediateRenderTarget())));
            
            NAME_D3D12_OBJECT(GetScene().GetIntermediateRenderTarget());

            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(GetScene().GetIntermediateRenderTarget().Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, GetScene().GetRtvDescriptorHeapSize());

            // Create a SRV of the intermediate render target.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = renderTargetDesc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(GetScene().GetSrvHeap()->GetCPUDescriptorHandleForHeapStart());
            RenderContext::GetGraphicsDevice()->CreateShaderResourceView(GetScene().GetIntermediateRenderTarget().Get(), &srvDesc, srvHandle);
        }

        }

    void SimulationWindow::UpdateSimulation()
    {
        GetDisplay().UpdateProjectionMatrix(GetScene().GetCamera());
        Engine::EngineContext::GetTimer().Tick(nullptr);
        GetScene().GetCamera().Update(static_cast<float>(Engine::EngineContext::GetTimer().GetElapsedSeconds()));
        GetScene().UpdateConstantBufferByIndex(GetDisplay().GetProjectionMatrix(), GetDisplay().GetFrameIndex(), GetDrawIndex());
    }


    void SimulationWindow::RenderPass()
    {

        // Present the frame.
        ThrowIfFailed(GetDisplay().GetSwapChain().Present(1, 0));

        ResetDrawIndex();
        GetPipelineLibrary().EndFrame();

        const uint32 nextFrameIndex = GetDisplay().GetSwapChain().GetCurrentBackBufferIndex();
        GetFence().MoveFenceMarker(GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), GetDisplay().GetFrameIndex(), nextFrameIndex);
        GetDisplay().TurnOverSwapChainBuffer();
    }

    void SimulationWindow::PreRenderPass()
    {
        const String cmdListHashKey = GOHKeys::CmdList::PRIMARY;
        const String cmdQueueHashKey = GOHKeys::CmdQueue::PRIMARY;

        CD3DX12_CPU_DESCRIPTOR_HANDLE intermediateRtvHandle(GetScene().GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), SwapChainBuffers::TOTAL_BUFFERS, GetScene().GetRtvDescriptorHeapSize());
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetScene().GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), GetDisplay().GetFrameIndex(), GetScene().GetRtvDescriptorHeapSize());

        
        ThrowIfFailed(GetCommandAllocator(GetDisplay().GetFrameIndex())->Reset());
        ThrowIfFailed(GetCommandListManager().GetCommandList(cmdListHashKey)->Reset(GetCommandAllocator(GetDisplay().GetFrameIndex()), nullptr));
        

        GetCommandListManager().GetCommandList(cmdListHashKey)->SetGraphicsRootSignature(GetScene().GetRootSignature().Get());
        GetCommandListManager().GetCommandList(cmdListHashKey)->RSSetViewports(1, &GetDisplay().GetViewPort());
        GetCommandListManager().GetCommandList(cmdListHashKey)->RSSetScissorRects(1, &GetDisplay().GetScissorRect());

        ID3D12DescriptorHeap *ppHeaps[] = { GetScene().GetSrvHeap().Get() };

        GetCommandListManager().GetCommandList(cmdListHashKey)->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
        GetCommandListManager().GetCommandList(cmdListHashKey)->SetGraphicsRootConstantBufferView(RootParameterCB, GetScene().GetDynamicConstantBuffer().GetGpuVirtualAddress(GetDrawIndex(), GetDisplay().GetFrameIndex()));
        GetCommandListManager().GetCommandList(cmdListHashKey)->OMSetRenderTargets(1, &intermediateRtvHandle, FALSE, nullptr);
        GetCommandListManager().GetCommandList(cmdListHashKey)->ClearRenderTargetView(intermediateRtvHandle, INTERMEDIATE_CLEAR_COLOUR, 0, nullptr);

        GetPipelineLibrary().SetPipelineState(GetScene().GetRootSignature(), GetCommandListManager().GetCommandList(cmdListHashKey), Mapped::BaseNormal3DRender, GetDisplay().GetFrameIndex());
        
        GetScene().DrawSceneShapes(GetCommandListManager().GetCommandList(cmdListHashKey));

        // TODO - make barriers a class (1)
        D3D12_RESOURCE_BARRIER barriers[2] = {};
        barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(GetScene().GetRenderTarget(GetDisplay().GetFrameIndex()).Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(GetScene().GetIntermediateRenderTarget().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        GetCommandListManager().GetCommandList(cmdListHashKey)->ResourceBarrier(_countof(barriers), barriers);

        GetCommandListManager().GetCommandList(cmdListHashKey)->SetGraphicsRootDescriptorTable(RootParameterSRV, GetScene().GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());
        
        GetPipelineLibrary().SetPipelineState(GetScene().GetRootSignature(), GetCommandListManager().GetCommandList(cmdListHashKey), Mapped::PostBlit, GetDisplay().GetFrameIndex());

        //GetScene().ClearOutputMergerRenderTarget(GetCommandListManager().GetCommandList(cmdListHashKey), GetDisplay().GetFrameIndex());
        
        GetCommandListManager().GetCommandList(cmdListHashKey)->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
        GetCommandListManager().GetCommandList(cmdListHashKey)->ClearRenderTargetView(rtvHandle, INTERMEDIATE_CLEAR_COLOUR, 0, nullptr);        


        // TODO - make this more dynamic (2)
        GetScene().GetQuad().DrawInstanced(GetCommandListManager().GetCommandList(cmdListHashKey), 1);

        // TODO - make barriers a class (2)
        barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        GetCommandListManager().GetCommandList(cmdListHashKey)->ResourceBarrier(_countof(barriers), barriers);
        
        GetCommandListManager().CloseCommandList(cmdListHashKey);
        GetCommandListManager().ExecuteCommandList(cmdListHashKey, GetCommandQueueManager().GetCommandQueue(cmdQueueHashKey));
        IncrementDrawIndex();

    }

    void SimulationWindow::DestroySwapChainResources()
    {
        GetScene().ReleaseResourceViews();
        GetFence().ShutdownFence(GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), GetDisplay().GetFrameIndex());
    }

    void SimulationWindow::ResetFrameCommandObjects(ComPointer<ID3D12GraphicsCommandList> &frameCmdList, ComPointer<ID3D12CommandAllocator> &frameCmdAllocator) const
    {
        ThrowIfFailed(frameCmdAllocator->Reset());
        ThrowIfFailed(frameCmdList->Reset(frameCmdAllocator, nullptr));
    }

    void SimulationWindow::UpdateShaderParameters(ComPointer<ID3D12GraphicsCommandList> &frameCmdList, const uint32 frameIndex)
    {
        GetDisplay().SetRasterizerView(frameCmdList);
        GetScene().SetShaderResourceViewDescriptorHeap(frameCmdList);
        GetScene().SetDynamicConstantBufferByIndex(frameCmdList, frameIndex, GetDrawIndex());
    }








}
