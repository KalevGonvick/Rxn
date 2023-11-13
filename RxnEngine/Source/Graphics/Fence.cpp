#include "Rxn.h"
#include "Fence.h"

namespace Rxn::Graphics::GPU
{
    Fence::Fence() = default;

    Fence::~Fence()
    {
        Shutdown();
    }

    HANDLE Fence::GetFenceEvent()
    {
        return m_FenceEvent;
    }

    ComPointer<ID3D12Fence> Fence::GetFence()
    {
        return m_Fence;
    }

    uint64 Fence::GetFenceValue(uint64 index) const
    {
        return m_FenceValues[index];
    }

    void Fence::CreateFence(uint32 frameIndex)
    {
        HRESULT result = RenderContext::GetGraphicsDevice()->CreateFence(m_FenceValues[frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create a new fence.");
        }

        IncrementFenceValue(frameIndex);
        CreateFenceEvent();
    }

    void Fence::CreateFenceEvent()
    {
        // Create an event handle to use for frame synchronization.
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_FenceEvent == nullptr)
        {
            RXN_LOGGER::Error(L"Failed to create a new fence event.");
        }
    }

    void Fence::IncrementFenceValue(uint32 frameIndex)
    {
        m_FenceValues[frameIndex]++;
    }

    void Fence::SignalFence(ID3D12CommandQueue *cmdQueue, const uint32 frameIndex)
    {
        const uint64 fenceValue = m_FenceValues[frameIndex];

        HRESULT result = cmdQueue->Signal(m_Fence.Get(), fenceValue);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to signal fence value: %d", fenceValue);
        }
    }

    void Fence::Shutdown()
    {
        CloseHandle(m_FenceEvent);
    }

    void Fence::WaitInfinite(const uint32 frameIndex)
    {
        if (m_Fence->GetCompletedValue() < m_FenceValues[frameIndex])
        {
            ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[frameIndex], m_FenceEvent));
            WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
        }
    }

    void Fence::Wait(const uint32 frameIndex, uint64 ms)
    {
        if (m_Fence->GetCompletedValue() < m_FenceValues[frameIndex])
        {
            ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValues[frameIndex], m_FenceEvent));
            WaitForSingleObjectEx(m_FenceEvent, ms, FALSE);
        }
    }

    void Fence::MoveFenceMarker(ID3D12CommandQueue * cmdQueue, const uint32 frameIndex, uint32 nextFrameIndex)
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
