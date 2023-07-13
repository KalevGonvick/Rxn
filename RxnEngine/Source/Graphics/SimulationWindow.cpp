#include "Rxn.h"
#include "SimulationWindow.h"

namespace Rxn::Graphics
{
    SimulationWindow::SimulationWindow(WString windowTitle, WString windowClass, int width, int height)
        : Platform::Win32::Window(windowTitle, windowClass)
        , m_UseWarpDevice(false)
        , m_HasTearingSupport(false)
        , m_Width(width)
        , m_Viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))
        , m_ScissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
        , m_Height(height)
        , m_RTVDescriptorSize(0)
        , m_IsRenderReady(false)
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        m_AssetsPath = assetsPath;
        m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
    }

    SimulationWindow::~SimulationWindow() = default;

    LRESULT SimulationWindow::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_PAINT:
        {
            OnRender();
        }
        default:
            break;
        }

        return Window::MessageHandler(hWnd, msg, wParam, lParam);
    }

    void SimulationWindow::SetupWindowAppearance()
    {
        //auto childsim = std::make_shared<Platform::Win32::SimulationWindow>(L"TestChildWindow", L"TestChildWindow");
        //childsim->m_WindowStyle = Platform::Win32::WindowStyle::STATIC;
        //childsim->m_Size = SIZE(600, 600);
        //AddChildComponent(childsim);

        m_Size = SIZE(1280, 720);
        m_AddCloseButton = true;
        m_AddMaximizeButton = true;
        m_AddMinimizeButton = true;
    }

    void SimulationWindow::LoadPipeline()
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

        ComPointer<IDXGIFactory4> factory;
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

        if (m_UseWarpDevice)
        {
            ComPointer<IDXGIAdapter> warpAdapter;
            ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

            ThrowIfFailed(D3D12CreateDevice(
                warpAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&m_Device)
            ));
        }
        else
        {
            ComPointer<IDXGIAdapter1> hardwareAdapter;
            GetHardwareAdapter(factory.Get(), &hardwareAdapter);

            ThrowIfFailed(D3D12CreateDevice(
                hardwareAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&m_Device)
            ));
        }

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = Constants::Graphics::SLIDE_COUNT;
        swapChainDesc.Width = m_Width;
        swapChainDesc.Height = m_Height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPointer<IDXGISwapChain1> swapChain;
        ThrowIfFailed(factory->CreateSwapChainForHwnd(
            m_CommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
            m_pHWnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        ));

        // This sample does not support fullscreen transitions.
        ThrowIfFailed(factory->MakeWindowAssociation(m_pHWnd, DXGI_MWA_NO_ALT_ENTER));

        /* Cast swapchain to */
        ThrowIfFailed(swapChain->QueryInterface(&m_SwapChain));
        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        // Create descriptor heaps.
        {
            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = Constants::Graphics::SLIDE_COUNT;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap)));

            m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        // Create frame resources.
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (UINT n = 0; n < Constants::Graphics::SLIDE_COUNT; n++)
            {
                ThrowIfFailed(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n])));
                m_Device->CreateRenderTargetView(m_RenderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_RTVDescriptorSize);
            }
        }

        ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));

    }

    void SimulationWindow::LoadAssets()
    {
        // Create an empty root signature.
        {
            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
            rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            ComPointer<ID3DBlob> signature;
            ComPointer<ID3DBlob> error;
            ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
            ThrowIfFailed(m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
        }

        // Create the pipeline state, which includes compiling and loading shaders.
        {
            ComPointer<ID3DBlob> vertexShader;
            ComPointer<ID3DBlob> pixelShader;

#if defined(_DEBUG)
            // Enable better shader debugging with the graphics debugging tools.
            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
            UINT compileFlags = 0;
#endif

            ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
            ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

            // Define the vertex input layout.
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
            ThrowIfFailed(m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState)));
        }

        // Create the command list.
        ThrowIfFailed(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), m_PipelineState.Get(), IID_PPV_ARGS(&m_CommandList)));

        // Command lists are created in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        ThrowIfFailed(m_CommandList->Close());

        // Create the vertex buffer.
        {
            // Define the geometry for a triangle.
            Vertex triangleVertices[] =
            {
                { { 0.0f, 0.25f * m_AspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
                { { 0.25f, -0.25f * m_AspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
                { { -0.25f, -0.25f * m_AspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
            };

            const UINT vertexBufferSize = sizeof(triangleVertices);
            const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            const auto buf = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
            // Note: using upload heaps to transfer static data like vert buffers is not 
            // recommended. Every time the GPU needs it, the upload heap will be marshalled 
            // over. Please read up on Default Heap usage. An upload heap is used here for 
            // code simplicity and because there are very few verts to actually transfer.
            ThrowIfFailed(m_Device->CreateCommittedResource(
                &heapProps,
                D3D12_HEAP_FLAG_NONE,
                &buf,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_VertexBuffer)));

            // Copy the triangle data to the vertex buffer.
            UINT8 *pVertexDataBegin = nullptr;
            CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
            ThrowIfFailed(m_VertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
            memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
            m_VertexBuffer->Unmap(0, nullptr);

            // Initialize the vertex buffer view.
            m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
            m_VertexBufferView.StrideInBytes = sizeof(Vertex);
            m_VertexBufferView.SizeInBytes = vertexBufferSize;
        }

        // Create synchronization objects and wait until assets have been uploaded to the GPU.
        {
            ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
            m_FenceValue = 1;

            // Create an event handle to use for frame synchronization.
            m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (m_FenceEvent == nullptr)
            {
                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
            }

            // Wait for the command list to execute; we are reusing the same command 
            // list in our main loop but for now, we just want to wait for setup to 
            // complete before continuing.
            WaitForPreviousFrame();
        }
    }



    void SimulationWindow::GetHardwareAdapter(IDXGIFactory1 *pFactory, IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter)
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
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
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
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        adapter.Release();
    }

    void SimulationWindow::CheckTearingSupport()
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

    void SimulationWindow::WaitForPreviousFrame()
    {
        // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
        // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
        // sample illustrates how to use fences for efficient resource usage and to
        // maximize GPU utilization.

        // Signal and increment the fence value.
        const UINT64 fence = m_FenceValue;
        ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), fence));
        m_FenceValue++;

        // Wait until the previous frame is finished.
        if (m_Fence->GetCompletedValue() < fence)
        {
            ThrowIfFailed(m_Fence->SetEventOnCompletion(fence, m_FenceEvent));
            WaitForSingleObject(m_FenceEvent, INFINITE);
        }

        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
    }

    void SimulationWindow::PopulateCommandList()
    {
        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        ThrowIfFailed(m_CommandAllocator->Reset());

        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), m_PipelineState.Get()));

        // Set necessary state.
        m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
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
        const auto finaltransition = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        // Indicate that the back buffer will now be used to present.
        m_CommandList->ResourceBarrier(1, &finaltransition);

        ThrowIfFailed(m_CommandList->Close());
    }

    void SimulationWindow::OnDestroy()
    {
        WaitForPreviousFrame();
        CloseHandle(m_FenceEvent);
    }

    void SimulationWindow::OnRender()
    {
        if (m_IsRenderReady)
        {
            // Record all the commands we need to render the scene into the command list.
            PopulateCommandList();

            // Execute the command list.
            ID3D12CommandList *ppCommandLists[] = { m_CommandList.Get() };
            m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

            // Present the frame.
            ThrowIfFailed(m_SwapChain->Present(1, 0));

            WaitForPreviousFrame();
        }

    }

    void SimulationWindow::OnInit()
    {
        LoadPipeline();
        LoadAssets();
        m_IsRenderReady = true;
    }
}
