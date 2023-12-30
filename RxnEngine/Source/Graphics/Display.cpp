#include "Rxn.h"
#include "Display.h"

namespace Rxn::Graphics
{
    Display::Display(int32 width, int32 height)
        : m_Viewport(0.0f, 0.0f, static_cast<float32>(width), static_cast<float32>(height))
        , m_ScissorRect(0, 0, static_cast<uword>(width), static_cast<uword>(height))
        , m_Width(width)
        , m_Height(height)
        , m_SwapChain(width, height)
        , m_Fence(2)
    {
        m_AspectRatio = static_cast<float32>(m_Width) / static_cast<float32>(m_Height);
    };

    Display::~Display() = default;

    void Display::Init(ID3D12CommandQueue *pCmdQueue, bool hasTearingSupport)
    {
        m_SwapChain.CreateSwapChain(pCmdQueue, hasTearingSupport);
        TurnOverSwapChainBuffer();

        m_Fence.CreateFence(m_FrameIndex);

        RotateSwapChainMarker(pCmdQueue);
        WaitForDisplay(pCmdQueue);
    }

    void Display::RotateSwapChainMarker(ID3D12CommandQueue *pCmdQueue)
    {
        const uint32 nextFrameIndex = m_SwapChain.GetCurrentBackBufferIndex();
        m_Fence.MoveFenceMarker(pCmdQueue, m_FrameIndex, nextFrameIndex);
        TurnOverSwapChainBuffer();
    }

    void Display::WaitForDisplay(ID3D12CommandQueue *pCmdQueue)
    {
        m_Fence.SignalFence(pCmdQueue, m_FrameIndex);
        m_Fence.WaitInfinite(m_FrameIndex);
        m_Fence.IncrementFenceValue(m_FrameIndex);
    }

    bool Display::IsSizeEqual(uint32 width, uint32 height) const
    {
        return (width == m_Width && height == m_Height);
    }

    uint32 Display::GetHeight() const
    {
        return m_Height;
    }

    uint32 Display::GetWidth() const
    {
        return m_Width;
    }

    void Display::SetWidth(uint32 width)
    {
        m_Width = width;
    }

    void Display::SetHeight(uint32 height)
    {
        m_Height = height;
    }

    void Display::HandleSizeChange(uint32 newWidth, uint32 newHeight)
    {
        m_Height = newHeight;
        m_Width = newWidth;

        float32 viewWidthRatio = static_cast<float32>(1920) / static_cast<float32>(m_Width);
        float32 viewHeightRatio = static_cast<float32>(1080) / static_cast<float32>(m_Height);
        m_AspectRatio = static_cast<float32>(m_Width) / static_cast<float32>(m_Height);

        float32 x = 1.0f;
        float32 y = 1.0f;

        if (viewWidthRatio < viewHeightRatio)
        {
            x = viewWidthRatio / viewHeightRatio;
        }
        else
        {
            y = viewHeightRatio / viewWidthRatio;
        }

        m_Viewport.TopLeftX = static_cast<float32>(m_Width) * (1.0f - x) / 2.0f;
        m_Viewport.TopLeftY = static_cast<float32>(m_Height) * (1.0f - y) / 2.0f;
        m_Viewport.Width = x * static_cast<float32>(m_Width);
        m_Viewport.Height = y * static_cast<float32>(m_Height);

        m_ScissorRect.right = m_Width;
        m_ScissorRect.bottom = m_Height;

        ThrowIfFailed(m_SwapChain.ResizeBuffers(newWidth, newHeight));
        TurnOverSwapChainBuffer();
    }

    uint32 Display::GetFrameIndex() const
    {
        return m_FrameIndex;
    }

    void Display::TurnOverSwapChainBuffer()
    {
        m_FrameIndex = m_SwapChain.GetCurrentBackBufferIndex();
    }

    void Display::UpdateProjectionMatrix(Camera &cam)
    {
        m_ProjectionMatrix = cam.GetProjectionMatrix(0.8f, m_AspectRatio);
    }

    GPU::SwapChain &Display::GetSwapChain()
    {
        return m_SwapChain;
    }

    CD3DX12_VIEWPORT &Display::GetViewPort()
    {
        return m_Viewport;
    }

    CD3DX12_RECT &Display::GetScissorRect()
    {
        return m_ScissorRect;
    }

    const DirectX::XMMATRIX & Display::GetProjectionMatrix() const
    {
        return m_ProjectionMatrix;
    }
}
