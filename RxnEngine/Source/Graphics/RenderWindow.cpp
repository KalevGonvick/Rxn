#include "Rxn.h"
#include "RenderWindow.h"

namespace Rxn::Graphics
{
    RenderWindow::RenderWindow(const WString &windowTitle, const WString &windowClass, int32 width, int32 height)
        : Renderer(width, height)
        , Platform::Win32::Window(windowTitle, windowClass, width, height)
    {}

    RenderWindow::~RenderWindow() = default;
    
    uint64 RenderWindow::GetFPS() const
    {
        return Engine::EngineContext::GetTimer().GetFramesPerSecond();
    }

    void RenderWindow::SetupWindow()
    {
        m_AddCloseButton = true;
        m_AddMaximizeButton = true;
        m_AddMinimizeButton = true;

        m_WindowStyle = WS_SIZEBOX;

        RegisterComponentClass();
        InitializeWin32();

        RenderContext::SetHWND(m_HWnd);
        InitializeRender();        

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
            HandleKeyUp(static_cast<uint8>(wParam));
            return 0;
        }
        case WM_PAINT: 
        {
            UpdateSimulation();
            RenderPass(); 
            return 0;
        }
        case WM_KEYDOWN:
        {
            HandleKeyDown(static_cast<uint8>(wParam));
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
            GetDisplay().WaitForDisplay(GetCommandQueuePool().Request(GetDisplay().GetFrameIndex()));
            GetScene().GetPipelineLibrary().ClearPSOCache();
            GetScene().GetPipelineLibrary().Build(RenderContext::GetGraphicsDevice(), GetScene().GetRootSignature(), 1,256, 2);
            break;

        case 'U':
            RXN_LOGGER::Debug(L"U was pressed, toggling uber shader...");
            GetScene().GetPipelineLibrary().ToggleUberShader();
            break;

        case 'L':
            RXN_LOGGER::Debug(L"L was pressed, switching to disk library...");
            GetScene().GetPipelineLibrary().ToggleDiskLibrary();
            break;

        case 'M':
            RXN_LOGGER::Debug(L"M was pressed, switching to PSO caching...");
            GetScene().GetPipelineLibrary().SwitchPSOCachingMechanism();
            break;
        case 'R':
            RXN_LOGGER::Debug(L"R was pressed, reporting live objects...");
            SendMessage(m_HWnd, WM_CLOSE, 0, 0);
            break;

        default:
            break;
        }
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
            //GetFence().ShutdownFence(GetCommandQueueManager().GetCommandQueue(GOHKeys::CmdQueue::PRIMARY), FRAME_INDEX);
            GetDisplay().HandleSizeChange(static_cast<uint32>(newWidth), static_cast<uint32>(newHeight));
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(GetScene().GetRtvHeap()->GetCPUDescriptorHandleForHeapStart());
            
            ID3D12Resource *rtv1 = GetScene().GetRenderTarget(0);
            ID3D12Resource *rtv2 = GetScene().GetRenderTarget(1);
            ID3D12Resource *intermediateRtv = GetScene().GetRenderTarget(2);

            ThrowIfFailed(GetDisplay().GetSwapChain().GetBuffer(0, &rtv1));
            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(rtv1, nullptr, rtvHandle);
            rtvHandle.Offset(1, GetScene().GetRtvDescriptorHeapSize());

            ThrowIfFailed(GetDisplay().GetSwapChain().GetBuffer(1, &rtv2));
            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(rtv2, nullptr, rtvHandle);
            rtvHandle.Offset(1, GetScene().GetRtvDescriptorHeapSize());
            
            D3D12_RESOURCE_DESC renderTargetDesc = rtv1->GetDesc();
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
                IID_PPV_ARGS(&intermediateRtv)));

            RenderContext::GetGraphicsDevice()->CreateRenderTargetView(intermediateRtv, nullptr, rtvHandle);
            rtvHandle.Offset(1, GetScene().GetRtvDescriptorHeapSize());

            // Create a SRV of the intermediate render target.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = renderTargetDesc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;

            CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(GetScene().GetSrvHeap()->GetCPUDescriptorHandleForHeapStart());
            RenderContext::GetGraphicsDevice()->CreateShaderResourceView(intermediateRtv, &srvDesc, srvHandle);
        }

    }

    void RenderWindow::UpdateSimulation()
    {
        Engine::EngineContext::GetTimer().Tick(nullptr);
        GetScene().GetCamera().Update(static_cast<float32>(Engine::EngineContext::GetTimer().GetElapsedSeconds()));
        GetDisplay().UpdateProjectionMatrix(GetScene().GetCamera());
        GetScene().UpdateConstantBufferByIndex(GetDisplay().GetProjectionMatrix(), GetDisplay().GetFrameIndex(), 0);
    }


    void RenderWindow::RenderPass()
    {
        const uint32 FRAME_INDEX = GetDisplay().GetFrameIndex();

        auto cmdAlloc = GetCommandAllocatorPool().Request(FRAME_INDEX);
        auto cmdQueue = GetCommandQueuePool().Request(0);

        auto cmdList = GetCommandListPool().RequestAndReset(FRAME_INDEX, cmdAlloc);
        
        SceneRenderContext renderContext(GetScene(), GetDisplay());
        renderContext.FrameStart(cmdList);
        renderContext.ClearRtv(cmdList, 2);
        
        const uint32 BASE_PIPELINE_EFFECT = 0;
        renderContext.SetPipelineState(cmdList, BASE_PIPELINE_EFFECT);

        GetScene().DrawSceneShapes(cmdList); //TODO

        const uint32 BARRIER_ONE_INDEX = 0;
        const uint32 BARRIER_TWO_INDEX = 1;
        renderContext.AddBarrier(BARRIER_ONE_INDEX, GetScene().GetRenderTarget(FRAME_INDEX), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        renderContext.AddBarrier(BARRIER_TWO_INDEX, GetScene().GetRenderTarget(2), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        renderContext.ExecuteBarriers(cmdList);
        renderContext.SetRootDescriptorTable(cmdList, 2);

        const uint32 POST_PIPELINE_EFFECT = 6;
        renderContext.SetPipelineState(cmdList, POST_PIPELINE_EFFECT);
        renderContext.ClearRtv(cmdList, FRAME_INDEX);

        GetScene().GetQuad().DrawInstanced(cmdList, 1); //TODO

        renderContext.SwapBarrier(BARRIER_ONE_INDEX);
        renderContext.SwapBarrier(BARRIER_TWO_INDEX);
        renderContext.ExecuteBarriers(cmdList);

        GetCommandListPool().ExecuteAndDiscard(FRAME_INDEX, cmdList, cmdQueue);
        GetCommandAllocatorPool().Discard(FRAME_INDEX, cmdAlloc);
        GetCommandQueuePool().Discard(0, cmdQueue);

        renderContext.PresentFrame(cmdQueue);


    }








}
