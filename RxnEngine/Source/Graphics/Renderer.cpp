#include "Rxn.h"
#include "Renderer.h"
#include "Core/RxnBinaryHandler.h"

namespace Rxn::Graphics
{
    Renderer::Renderer(int32 width, int32 height)
        : m_Display(width, height)
    {}

    Renderer::~Renderer() = default;

    void Renderer::InitializeRender()
    {
        const uint32 STARTING_FRAME_INDEX = 0;

        m_Cmd_A_Pool.Create(RenderContext::GetGraphicsDevice());
        m_Cmd_L_Pool.Create(RenderContext::GetGraphicsDevice());
        m_Cmd_Q_Pool.Create(RenderContext::GetGraphicsDevice());
        
        auto cmdAlloc = m_Cmd_A_Pool.Request(STARTING_FRAME_INDEX);
        auto cmdQueue = m_Cmd_Q_Pool.Request(0);
        
        m_Display.Init(cmdQueue, m_HasTearingSupport);
        m_Scene.Init(RenderContext::GetGraphicsDevice(), m_Display.GetSwapChain(), RenderContext::GetHighestRootSignatureVersion());

        std::vector<VertexPositionColour> vertexData;
        std::vector<uint32> indexData;
        Core::RxnBinaryHandler::ReadRxnFile(vertexData, indexData, "cube.bin");
        
        auto initCmdList = m_Cmd_L_Pool.RequestAndReset(STARTING_FRAME_INDEX, cmdAlloc);
        m_Scene.AddShapeFromRaw(RenderContext::GetGraphicsDevice(), vertexData, indexData, initCmdList);

        std::vector<VertexPositionUV> quadVertices;
        quadVertices.push_back(VertexPositionUV{ { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } });
        quadVertices.push_back(VertexPositionUV{ { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } });
        quadVertices.push_back(VertexPositionUV{ { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } });
        quadVertices.push_back(VertexPositionUV{ { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } });

        m_Scene.AddQuadFromRaw(RenderContext::GetGraphicsDevice(), quadVertices, initCmdList);

        m_Cmd_L_Pool.ExecuteAndDiscard(STARTING_FRAME_INDEX, initCmdList, cmdQueue);
        m_Cmd_A_Pool.Discard(STARTING_FRAME_INDEX, cmdAlloc);
        m_Cmd_Q_Pool.Discard(0, cmdQueue);
    }

    Scene &Renderer::GetScene()
    {
        return m_Scene;
    }

    Display &Renderer::GetDisplay()
    {
        return m_Display;
    }

    Pooled::CommandQueuePool &Renderer::GetCommandQueuePool()
    {
        return m_Cmd_Q_Pool;
    }

    Pooled::CommandListPool &Renderer::GetCommandListPool()
    {
        return m_Cmd_L_Pool;
    }

    Pooled::CommandAllocatorPool &Renderer::GetCommandAllocatorPool()
    {
        return m_Cmd_A_Pool;
    }

    SceneRenderContext::SceneRenderContext(Scene &scene, Display &display)
        : m_Scene(scene)
        , m_Display(display)
    {}

    SceneRenderContext::~SceneRenderContext() = default;

    void SceneRenderContext::FrameStart(ID3D12GraphicsCommandList6 *cmdList)
    {
        cmdList->SetGraphicsRootSignature(m_Scene.GetRootSignature());
        cmdList->RSSetViewports(1, &m_Display.GetViewPort());
        cmdList->RSSetScissorRects(1, &m_Display.GetScissorRect());

        std::vector<ID3D12DescriptorHeap *> heaps;
        heaps.push_back(m_Scene.GetSrvHeap());
        cmdList->SetDescriptorHeaps(static_cast<uint32>(heaps.size()), heaps.data());

        cmdList->SetGraphicsRootConstantBufferView(
            1,
            m_Scene.GetDynamicConstantBuffer().GetGpuVirtualAddress(0, m_Display.GetFrameIndex())
        );
    }

    void SceneRenderContext::ClearRtv(ID3D12GraphicsCommandList6 *cmdList, uint32 rtvIndex) const
    {
        auto rtvHandle = m_Scene.GetRenderTargetCPUDescriptorHandle(rtvIndex);
        cmdList->OMSetRenderTargets(
            1,
            &rtvHandle,
            FALSE,
            nullptr
        );

        cmdList->ClearRenderTargetView(
            rtvHandle,
            RTV_CLEAR_COLOUR.data(),
            0,
            nullptr
        );
    }

    void SceneRenderContext::AddBarrier(uint32 barrierIndex, ID3D12Resource *pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
    {
        m_Barriers.emplace_back(barrierIndex, CD3DX12_RESOURCE_BARRIER::Transition(pResource, before, after));
    }

    void SceneRenderContext::SetRootDescriptorTable(ID3D12GraphicsCommandList6 *cmdList, uint32 rootParameterIndex)
    {
        cmdList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_Scene.GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());
    }


    void SceneRenderContext::ExecuteBarrier(ID3D12GraphicsCommandList6 *cmdList, uint32 barrierIndex) const
    {
        for (const auto & [index, barrier] : m_Barriers)
        {
            if (barrierIndex == index)
            {
                cmdList->ResourceBarrier(1, &barrier);
                return;
            }
        }

        RXN_LOGGER::Error(L"No barrier found with index %d", barrierIndex);
        throw RendererException(std::format("Render context contains no barrier at given index '{}'.", std::to_string(barrierIndex)));
    }

    void SceneRenderContext::ExecuteBarriers(ID3D12GraphicsCommandList6 *cmdList) const
    {
        std::vector<D3D12_RESOURCE_BARRIER> barrierVector;
        for (const auto &[index, barrier] : m_Barriers)
        {
            barrierVector.push_back(barrier);
        }

        cmdList->ResourceBarrier(static_cast<uint32>(barrierVector.size()), barrierVector.data());
    }

    void SceneRenderContext::SwapBarrier(uint32 barrierIndex)
    {

        for (auto &[index, barrier] : m_Barriers)
        {
            if (index == barrierIndex)
            {
                auto newAfterState = barrier.Transition.StateBefore;
                auto newBeforeState = barrier.Transition.StateAfter;
                barrier.Transition.StateBefore = newBeforeState;
                barrier.Transition.StateAfter = newAfterState;
                return;
            }
        }

        RXN_LOGGER::Error(L"No barrier found with index %d", barrierIndex);
        throw RendererException(std::format("Render context contains no barrier at given index '{}'.", std::to_string(barrierIndex)));

    }

    void SceneRenderContext::PresentFrame(ID3D12CommandQueue *pCmdQueue) const
    {
        // Present the frame.
        HRESULT result = m_Display.GetSwapChain().Present(1, 0);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to present the swap chain!");
            throw RendererException("Failed to present the swap chain!");
        }

        m_Display.RotateSwapChainMarker(pCmdQueue);
    }

    void SceneRenderContext::SetPipelineState(ID3D12GraphicsCommandList6 *cmdList, uint32 pipelineIndex)
    {
        m_Scene.GetPipelineLibrary().SetPipelineState(m_Scene.GetRootSignature(), cmdList, pipelineIndex, m_Display.GetFrameIndex());
    }

}
