/*****************************************************************//**
 * \file   RenderFence.h
 * \brief  RenderFence is a wrapper for a d3d12 fence object.
 * 
 * \author kalev
 * \date   November 2023
 *********************************************************************/
#pragma once

namespace Rxn::Graphics
{
    class RXN_ENGINE_API RenderFence
    {
    public:

        RenderFence();
        ~RenderFence();

    public:

        HANDLE GetFenceEvent();
        ComPointer<ID3D12Fence> GetFence();
        UINT64 GetFenceValue(uint64 index);

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
