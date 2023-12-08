#include "Rxn.h"
#include "SwapChain.h"

namespace Rxn::Graphics::GPU
{
    SwapChain::SwapChain(int width, int height)
    {
        m_SwapChainDesc.BufferCount = SwapChainBuffers::TOTAL_BUFFERS;
        m_SwapChainDesc.Width = width;
        m_SwapChainDesc.Height = height;
        m_SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        m_SwapChainDesc.SampleDesc.Count = 1;
    }

    SwapChain::~SwapChain() = default;

    void SwapChain::SetTearingSupport(bool tearingSupport)
    {
        m_SwapChainDesc.Flags = tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        if (tearingSupport)
        {
            ThrowIfFailed(RenderContext::GetFactory()->MakeWindowAssociation(RenderContext::GetHWND(), DXGI_MWA_NO_ALT_ENTER));
        }
    }

    void SwapChain::CreateSwapChain(ID3D12CommandQueue *cmdQueue)
    {
        ComPointer<IDXGISwapChain1> swapChain;
        ThrowIfFailed(RenderContext::GetFactory()->CreateSwapChainForHwnd(cmdQueue, RenderContext::GetHWND(), &m_SwapChainDesc, nullptr, nullptr, &swapChain));
        ThrowIfFailed(swapChain->QueryInterface(&m_SwapChain));
        m_SwapChainEvent = GetFrameLatencyWaitableObject();
    }

    uint32 SwapChain::GetCurrentBackBufferIndex()
    {
        return m_SwapChain->GetCurrentBackBufferIndex();
    }

    HANDLE SwapChain::GetFrameLatencyWaitableObject()
    {
        return m_SwapChain->GetFrameLatencyWaitableObject();
    }

    HRESULT SwapChain::GetBuffer(uint32 bufferNum, ComPointer<ID3D12Resource> &bufferResource)
    {
        return m_SwapChain->GetBuffer(bufferNum, IID_PPV_ARGS(&bufferResource));
    }

    HRESULT SwapChain::ResizeBuffers(int width, int height)
    {
        return m_SwapChain->ResizeBuffers(SwapChainBuffers::TOTAL_BUFFERS, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    }

    HRESULT SwapChain::Present(uint32 syncInterval, uint32 flags)
    {
        return m_SwapChain->Present(syncInterval, flags);
    }
}
