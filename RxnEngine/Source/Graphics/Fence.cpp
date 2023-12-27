#include "Rxn.h"
#include "Fence.h"

namespace Rxn::Graphics::GPU
{
    Fence::Fence() = default;
    Fence::~Fence() = default;

    HANDLE Fence::GetFenceEvent()
    {
        return m_FenceEvent;
    }

    ID3D12Fence* Fence::GetFence()
    {
        return m_Fence;
    }

    uint64 Fence::GetFenceValue(uint64 index) const
    {
        return m_FenceValues[index];
    }

    void Fence::CreateFence(uint32 frameIndex, D3D12_FENCE_FLAGS flags)
    {
        HRESULT result = RenderContext::GetGraphicsDevice()->CreateFence(m_FenceValues[frameIndex], flags, IID_PPV_ARGS(&m_Fence));
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create a new fence.");
            throw std::runtime_error("");
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
            throw std::runtime_error("");
        }
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
            WaitForSingleObjectEx(m_FenceEvent, static_cast<uint_word>(ms), FALSE);
        }
    }

    void Fence::ShutdownFence(ID3D12CommandQueue *cmdQueue, const uint32 frameIndex) 
    {
        SignalFence(cmdQueue, frameIndex);
        WaitInfinite(frameIndex);
        IncrementFenceValue(frameIndex);
        CloseHandle(m_FenceEvent);
    }

    void Fence::MoveFenceMarker(ID3D12CommandQueue * cmdQueue, const uint32 frameIndex, uint32 nextFrameIndex)
    {
        const uint64 fenceValue = m_FenceValues[frameIndex];

        HRESULT result = cmdQueue->Signal(m_Fence.Get(), fenceValue);
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to signal fence value: %d", fenceValue);
            throw std::runtime_error("");
        }

        if (m_Fence->GetCompletedValue() < m_FenceValues[nextFrameIndex])
        {
            result = m_Fence->SetEventOnCompletion(m_FenceValues[nextFrameIndex], m_FenceEvent);
            if (FAILED(result))
            {
                RXN_LOGGER::Error(L"Failed to set new event on completion.");
                throw std::runtime_error("");
            }

            WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
        }

        m_FenceValues[nextFrameIndex] = fenceValue + 1;
    }

}
