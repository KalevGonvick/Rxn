#pragma once

namespace Rxn::Graphics::GPU
{
    class RXN_ENGINE_API SwapChain
    {

    public:

        SwapChain(int width, int height);
        ~SwapChain();

    public:

        void SetTearingSupport(bool tearingSupport);
        void CreateSwapChain(ID3D12CommandQueue *cmdQueue);

        uint32 GetCurrentBackBufferIndex();
        HANDLE GetFrameLatencyWaitableObject();
        HRESULT GetBuffer(uint32 bufferNum, ComPointer<ID3D12Resource> &bufferResource);
        HRESULT ResizeBuffers(int width, int height);
        HRESULT Present(uint32 syncInterval, uint32 flags);

    private:

        HANDLE m_SwapChainEvent;
        DXGI_SWAP_CHAIN_DESC1 m_SwapChainDesc{};
        ComPointer<IDXGISwapChain4> m_SwapChain;
        bool m_Initialized = false;

    };
}
