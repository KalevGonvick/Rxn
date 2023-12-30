/*****************************************************************//**
 * \file   RenderFence.h
 * \brief  RenderFence is a wrapper for a d3d12 fence object.
 * 
 * \author kalev
 * \date   November 2023
 *********************************************************************/
#pragma once

namespace Rxn::Graphics::GPU
{
    struct FenceException : std::runtime_error
    {
        explicit FenceException(const String &msg) : std::runtime_error(msg) {};
    };

    class Fence
    {
    public:

        explicit Fence(uint32 fenceCount);
        ~Fence();

    public:

        HANDLE GetFenceEvent();
        ID3D12Fence *GetFence();
        uint64 GetFenceValue(uint32 index) const;

        void CreateFenceEvent();
        void CreateFence(uint32 frameIndex, D3D12_FENCE_FLAGS flags = D3D12_FENCE_FLAG_NONE);
        void IncrementFenceValue(uint32 frameIndex);
        void SignalFence(ID3D12CommandQueue *cmdQueue, uint32 frameIndex);
        void WaitInfinite(uint32 frameIndex);
        void Wait(const uint32 frameIndex, uint64 ms);
        void MoveFenceMarker(ID3D12CommandQueue *cmdQueue, uint32 frameIndex, uint32 nextFrameIndex);
        void ShutdownFence(ID3D12CommandQueue *cmdQueue, const uint32 frameIndex);

    private:

        HANDLE m_FenceEvent = nullptr;
        ComPointer<ID3D12Fence> m_Fence;
        std::vector<uint64> m_FenceValues;
    };
}
