#include "Rxn.h"
#include "RenderFence.h"

namespace Rxn::Graphics
{
    RenderFence::RenderFence()
    {

    }

    RenderFence::~RenderFence()
    {
        Shutdown();
    }

    HANDLE RenderFence::GetFenceEvent()
    {
        return m_FenceEvent;
    }

    ComPointer<ID3D12Fence> RenderFence::GetFence()
    {
        return m_Fence;
    }

    UINT64 RenderFence::GetFenceValue(uint64 index)
    {
        return m_FenceValues[index];
    }

    void RenderFence::CreateFence(uint32 frameIndex)
    {
        HRESULT result = RenderContext::GetGraphicsDevice()->CreateFence(m_FenceValues[frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create a new fence.");
        }

        IncrementFenceValue(frameIndex);
        CreateFenceEvent();
    }

    void RenderFence::CreateFenceEvent()
    {
        // Create an event handle to use for frame synchronization.
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_FenceEvent == nullptr)
        {
            RXN_LOGGER::Error(L"Failed to create a new fence event.");
        }
    }

    void RenderFence::IncrementFenceValue(uint32 frameIndex)
    {
        m_FenceValues[frameIndex]++;
    }

    void RenderFence::SignalFence(ID3D12CommandQueue *cmdQueue, const uint32 frameIndex)
    {
        const uint64 fenceValue = m_FenceValues[frameIndex];

        HRESULT result = cmdQueue->Signal(m_Fence.Get(), fenceValue);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to signal fence value: %d", fenceValue);
        }
    }

    void RenderFence::Shutdown()
    {
        CloseHandle(m_FenceEvent);
    }

    void RenderFence::WaitInfinite(const uint32 frameIndex)
    {
        if (m_Fence->GetCompletedValue() < m_FenceValues[frameIndex])
        {
            ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[frameIndex], m_FenceEvent));
            WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
        }
    }

    void RenderFence::Wait(const uint32 frameIndex, uint64 ms)
    {
        if (m_Fence->GetCompletedValue() < m_FenceValues[frameIndex])
        {
            ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[frameIndex], m_FenceEvent));
            WaitForSingleObjectEx(m_FenceEvent, ms, FALSE);
        }
    }

    void RenderFence::MoveFenceMarker(ID3D12CommandQueue * cmdQueue, const uint32 frameIndex, uint32 nextFrameIndex)
    {
        const UINT64 fenceValue = m_FenceValues[frameIndex];

        HRESULT result = cmdQueue->Signal(m_Fence.Get(), fenceValue);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to signal fence value: %d", fenceValue);
            return;
        }

        if (m_Fence->GetCompletedValue() < m_FenceValues[nextFrameIndex])
        {
            result = m_Fence->SetEventOnCompletion(m_FenceValues[nextFrameIndex], m_FenceEvent);
            if (FAILED(result))
            {
                RXN_LOGGER::Error(L"Failed to set new event on completion.");
                return;
            }

            WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
        }

        m_FenceValues[nextFrameIndex] = fenceValue + 1;
    }

}
