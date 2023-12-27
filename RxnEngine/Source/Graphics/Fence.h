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
    class RXN_ENGINE_API Fence
    {
    public:

        Fence();
        ~Fence();

    public:

        HANDLE GetFenceEvent();
        ID3D12Fence *GetFence();
        uint64 GetFenceValue(uint64 index) const;

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
        uint64 m_FenceValues[2];
    };
}
