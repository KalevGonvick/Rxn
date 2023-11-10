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
        ComPointer<ID3D12Fence> GetFence();
        uint64 GetFenceValue(uint64 index);

        void CreateFenceEvent();
        void CreateFence(uint32 frameIndex);
        void IncrementFenceValue(uint32 frameIndex);
        void Shutdown();
        void SignalFence(ID3D12CommandQueue *cmdQueue, uint32 frameIndex);
        void WaitInfinite(uint32 frameIndex);
        void Wait(const uint32 frameIndex, uint64 ms);
        void MoveFenceMarker(ID3D12CommandQueue *cmdQueue, uint32 frameIndex, uint32 nextFrameIndex);

    private:

        HANDLE m_FenceEvent;
        ComPointer<ID3D12Fence> m_Fence;
        uint64 m_FenceValues[SwapChainBuffers::TOTAL_BUFFERS];
    };
}
