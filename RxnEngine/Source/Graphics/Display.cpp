#include "Rxn.h"
#include "Display.h"

namespace Rxn::Graphics
{
    Display::Display(int32 width, int32 height)
        : m_Width(width)
        , m_Height(height)
        , m_SwapChain(width, height)
        , m_Fov(0.8f)
        , m_Viewport(0.0f, 0.0f, static_cast<float32>(width), static_cast<float32>(height))
        , m_ScissorRect(0, 0, static_cast<uint64>(width), static_cast<uint64>(height))
    {
        m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
    };

    Display::~Display() = default;

    const uint32 Display::GetHeight()
    {
        return m_Height;
    }

    const uint32 Display::GetWidth()
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

        float32 viewWidthRatio = static_cast<float32>(m_Resolutions[1].Width) / m_Width;
        float32 viewHeightRatio = static_cast<float32>(m_Resolutions[1].Height) / m_Height;
        m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);

        float32 x = 1.0f;
        float32 y = 1.0f;

        if (viewWidthRatio < viewHeightRatio)
        {
            // The scaled image's height will fit to the viewport's height and 
            // its width will be smaller than the viewport's width.
            x = viewWidthRatio / viewHeightRatio;
        }
        else
        {
            // The scaled image's width will fit to the viewport's width and 
            // its height may be smaller than the viewport's height.
            y = viewHeightRatio / viewWidthRatio;
        }

        m_Viewport.TopLeftX = m_Width * (1.0f - x) / 2.0f;
        m_Viewport.TopLeftY = m_Height * (1.0f - y) / 2.0f;
        m_Viewport.Width = x * m_Width;
        m_Viewport.Height = y * m_Height;

        m_ScissorRect.right = m_Width;
        m_ScissorRect.bottom = m_Height;

        ThrowIfFailed(m_SwapChain.ResizeBuffers(SwapChainBuffers::TOTAL_BUFFERS, newWidth, newHeight));
        TurnOverSwapChainBuffer();
    }

    const uint32 Display::GetFrameIndex()
    {
        return m_FrameIndex;
    }

    void Display::TurnOverSwapChainBuffer()
    {
        m_FrameIndex = m_SwapChain.GetCurrentBackBufferIndex();
    }

    void Display::UpdateProjectionMatrix(Camera cam)
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

    const DirectX::XMMATRIX & Display::GetProjectionMatrix()
    {
        return m_ProjectionMatrix;
    }
}
