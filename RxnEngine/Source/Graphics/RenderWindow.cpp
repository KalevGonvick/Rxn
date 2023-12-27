#include "Rxn.h"
#include "RenderWindow.h"
#include "Core/RxnBinaryHandler.h"

namespace Rxn::Graphics
{
    RenderWindow::RenderWindow(const WString &windowTitle, const WString &windowClass, int32 width, int32 height)
        : Renderer(width, height)
        , Platform::Win32::Window(windowTitle, windowClass, width, height)
    {
    }

    RenderWindow::~RenderWindow() = default;


    void RenderWindow::ShutdownRender()
    {
        GetFence().ShutdownFence(
            GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), 
            GetDisplay().GetFrameIndex()
        );
    }


    
    uint64 RenderWindow::GetFPS() const
    {
        return Engine::EngineContext::GetTimer().GetFramesPerSecond();
    }


    void RenderWindow::SetupWindow()
    {
        m_AddCloseButton = true;
        m_AddMaximizeButton = true;
        m_AddMinimizeButton = true;

        m_WindowStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;

        RegisterComponentClass();
        InitializeWin32();

        RenderContext::SetHWND(m_HWnd);
        InitializeRender();
        LoadSceneData();
        
        uint32 FRAME_INDEX = GetDisplay().GetFrameIndex();
        GetFence().CreateFence(FRAME_INDEX);
        
        const uint32 nextFrameIndex = GetDisplay().GetSwapChain().GetCurrentBackBufferIndex();
        GetFence().MoveFenceMarker(GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), FRAME_INDEX, nextFrameIndex);
        GetDisplay().TurnOverSwapChainBuffer();
        GetFence().SignalFence(GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), FRAME_INDEX);
        GetFence().WaitInfinite(FRAME_INDEX);
        GetFence().IncrementFenceValue(FRAME_INDEX);

        ShowWindow(m_HWnd, SW_SHOW);
        UpdateWindow(m_HWnd);
        
    }

    /* -------------------------------------------------------- */
    /*  WIN-API                                                 */
    /* -------------------------------------------------------- */
#pragma region WIN-API

    LRESULT RenderWindow::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

    void RenderWindow::HandleKeyDown(uint8 key)
    {
        GetScene().GetCamera().OnKeyDown(key);
    }

    void RenderWindow::HandleKeyUp(uint8 key)
    {
        GetScene().GetCamera().OnKeyUp(key);

        switch (key)
        {
        case 'C':
            GetFence().SignalFence(GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), GetDisplay().GetFrameIndex());
            GetFence().WaitInfinite(GetDisplay().GetFrameIndex());
            GetFence().IncrementFenceValue(GetDisplay().GetFrameIndex());
            GetPipelineLibrary().ClearPSOCache();
            GetPipelineLibrary().Build(RenderContext::GetGraphicsDevice(), GetRootSignature());
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
        case 'R':
            RXN_LOGGER::Debug(L"R was pressed, reporting live objects...");
            SendMessage(m_HWnd, WM_CLOSE, 0, 0);
            break;

        default:
            break;
        }
    }

    void RenderWindow::LoadSceneData()
    {
            
            const auto &primaryCommandQueue = GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY);
            auto &currentCommandAllocator = GetCommandAllocator(GetDisplay().GetFrameIndex());
            auto &initCmdList = GetCommandListManager().GetCommandList(GOHKeys::CmdList::INIT);

            initCmdList->Reset(currentCommandAllocator, nullptr);

            //Core::RxnBinaryHandler::WriteRxnFile( "cube.bin");
            std::vector<VertexPositionColour> vertexData;
            std::vector<uint32> indexData;
            Core::RxnBinaryHandler::ReadRxnFile(vertexData, indexData, "cube.bin");

            GetScene().AddShapeFromRaw(vertexData, indexData, currentCommandAllocator, primaryCommandQueue, initCmdList);

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


    void RenderWindow::OnSizeChange()
    {
        RECT clientRect;
        GetClientRect(m_HWnd, &clientRect);
        int64 newWidth = static_cast<int64>(clientRect.right) - static_cast<int64>(clientRect.left);
        int64 newHeight = static_cast<int64>(clientRect.bottom) - static_cast<int64>(clientRect.top);

        //
        // If the window size changed, resize our swapchain and recreate swapchain resources.
        //
        if (!GetDisplay().IsSizeEqual(static_cast<uint32>(newWidth), static_cast<uint32>(newHeight)))
        {
            GetScene().ReleaseResourceViews();
            GetFence().ShutdownFence(GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), GetDisplay().GetFrameIndex());
            GetDisplay().HandleSizeChange(static_cast<uint32>(newWidth), static_cast<uint32>(newHeight));
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetScene().GetRtvHeap()->GetCPUDescriptorHandleForHeapStart());

            ThrowIfFailed(GetDisplay().GetSwapChain().GetBuffer(0, GetScene().GetRenderTarget(0)));
            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(GetScene().GetRenderTarget(0), nullptr, rtvHandle);
            rtvHandle.Offset(1, GetScene().GetRtvDescriptorHeapSize());

            ThrowIfFailed(GetDisplay().GetSwapChain().GetBuffer(1, GetScene().GetRenderTarget(1)));
            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(GetScene().GetRenderTarget(1), nullptr, rtvHandle);
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
                IID_PPV_ARGS(&GetScene().GetRenderTarget(2))));

            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(GetScene().GetRenderTarget(2).Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, GetScene().GetRtvDescriptorHeapSize());

            // Create a SRV of the intermediate render target.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = renderTargetDesc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(GetScene().GetSrvHeap()->GetCPUDescriptorHandleForHeapStart());
            RenderContext::GetGraphicsDevice()->CreateShaderResourceView(GetScene().GetRenderTarget(2).Get(), &srvDesc, srvHandle);
        }

        }

    void RenderWindow::UpdateSimulation()
    {
        GetDisplay().UpdateProjectionMatrix(GetScene().GetCamera());
        Engine::EngineContext::GetTimer().Tick(nullptr);
        GetScene().GetCamera().Update(static_cast<float>(Engine::EngineContext::GetTimer().GetElapsedSeconds()));
        GetScene().UpdateConstantBufferByIndex(GetDisplay().GetProjectionMatrix(), GetDisplay().GetFrameIndex(), 0);
    }


    void RenderWindow::RenderPass()
    {

        // Present the frame.
        ThrowIfFailed(GetDisplay().GetSwapChain().Present(1, 0));

        GetPipelineLibrary().EndFrame();

        const uint32 nextFrameIndex = GetDisplay().GetSwapChain().GetCurrentBackBufferIndex();
        GetFence().MoveFenceMarker(GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), GetDisplay().GetFrameIndex(), nextFrameIndex);
        GetDisplay().TurnOverSwapChainBuffer();
    }

    void RenderWindow::PreRenderPass()
    {
        const uint32 FRAME_INDEX = GetDisplay().GetFrameIndex();
        const String CMD_LIST_HASH_KEY = GOHKeys::CmdList::PRIMARY;
        const String CMD_QUEUE_HASH_KEY = GOHKeys::CmdQueue::PRIMARY;

        auto &cmdList = GetCommandListManager().GetCommandList(CMD_LIST_HASH_KEY);
        auto &rootSignature = GetRootSignature();

        GetCommandAllocator(GetDisplay().GetFrameIndex())->Reset();
        
        cmdList->Reset(GetCommandAllocator(FRAME_INDEX), nullptr);
        
        SceneRenderContext renderContext(GetScene(), GetDisplay());
        renderContext.FrameStart(cmdList, rootSignature);
        
        CD3DX12_CPU_DESCRIPTOR_HANDLE intermediateRtvHandle(
            GetScene().GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(),
            2,
            GetScene().GetRtvDescriptorHeapSize()
        );

        renderContext.ClearRtv(cmdList, intermediateRtvHandle);

        GetPipelineLibrary().SetPipelineState(rootSignature,cmdList, 0, FRAME_INDEX);
        GetScene().DrawSceneShapes(cmdList);
        
        const uint32 BARRIER_ONE_INDEX = 0;
        const uint32 BARRIER_TWO_INDEX = 1;
        renderContext.AddBarrier(BARRIER_ONE_INDEX, GetScene().GetRenderTarget(FRAME_INDEX), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        renderContext.AddBarrier(BARRIER_TWO_INDEX, GetScene().GetRenderTarget(2), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        renderContext.ExecuteBarriers(cmdList);

        const uint32 PIPELINE_EFFECT_INDEX = 2;
        renderContext.SetRootDescriptorTable(cmdList, PIPELINE_EFFECT_INDEX);
        GetPipelineLibrary().SetPipelineState(rootSignature, cmdList, PIPELINE_EFFECT_INDEX, FRAME_INDEX);
        
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            GetScene().GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(),
            FRAME_INDEX,
            GetScene().GetRtvDescriptorHeapSize()
        );

        renderContext.ClearRtv(cmdList, rtvHandle);
        
        GetScene().GetQuad().DrawInstanced(cmdList, 1);

        
        renderContext.SwapBarrier(BARRIER_ONE_INDEX);
        renderContext.SwapBarrier(BARRIER_TWO_INDEX);
        renderContext.ExecuteBarriers(cmdList);
        renderContext.FrameEnd(cmdList);

        GetCommandListManager().ExecuteCommandList(CMD_LIST_HASH_KEY, GetCommandQueueManager().GetCommandQueue(CMD_QUEUE_HASH_KEY));

    }








}
