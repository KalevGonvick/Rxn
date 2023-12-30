#pragma once

namespace Rxn::Graphics::GPU
{
    class RXN_ENGINE_API SwapChain
    {

    public:

        SwapChain(uint32 width, uint32 height);
        ~SwapChain();

    public:

        void CreateSwapChain(ID3D12CommandQueue *cmdQueue, bool tearingSupport);

        uint32 GetCurrentBackBufferIndex();
        uint32 GetBufferCount() const;
        HANDLE GetFrameLatencyWaitableObject();
        HRESULT GetBuffer(uint32 bufferNum, ID3D12Resource **bufferResource);
        HRESULT ResizeBuffers(uint32 width, uint32 height);
        HRESULT Present(uint32 syncInterval, uint32 flags);

    private:

        HANDLE m_SwapChainEvent = nullptr;
        DXGI_SWAP_CHAIN_DESC1 m_SwapChainDesc{};
        ComPointer<IDXGISwapChain4> m_SwapChain;
        bool m_Initialized = false;

    };
}
