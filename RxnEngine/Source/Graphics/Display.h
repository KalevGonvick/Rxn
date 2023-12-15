/*****************************************************************//**
 * \file   Display.h
 * \brief  
 * 
 * \author kalev
 * \date   December 2023
 *********************************************************************/
#pragma once
#include "SwapChain.h"
#include "Camera.h"

namespace Rxn::Graphics
{
    class RXN_ENGINE_API Display
    {
    public:

        Display(int32 width, int32 height);
        ~Display();

    public:

        /**
         * .
         * 
         * \param newWidth
         * \param newHeight
         */
        void HandleSizeChange(uint32 newWidth, uint32 newHeight);

        /**
         * .
         * 
         */
        void TurnOverSwapChainBuffer();

        /**
         * .
         * 
         * \param cam
         */
        void UpdateProjectionMatrix(Camera cam);

        /**
         * .
         * 
         * \param frameCmdList
         */
        void SetRasterizerView(ComPointer<ID3D12GraphicsCommandList> frameCmdList);

        void SetWidth(uint32 width);
        void SetHeight(uint32 height);
        uint32 GetHeight() const;
        uint32 GetWidth() const;
        uint32 GetFrameIndex() const;
        GPU::SwapChain &GetSwapChain();
        CD3DX12_VIEWPORT &GetViewPort();
        CD3DX12_RECT &GetScissorRect();
        const DirectX::XMMATRIX &GetProjectionMatrix() const;

    private:

        CD3DX12_VIEWPORT m_Viewport;
        CD3DX12_RECT m_ScissorRect;
        DirectX::XMMATRIX m_ProjectionMatrix;

        uint32 m_Width;
        uint32 m_Height;
        uint32 m_FrameIndex = 0;

        float32 m_Fov = 0.8f;
        float32 m_AspectRatio;

        GPU::SwapChain m_SwapChain;
        std::array<Resolution, 2> const m_Resolutions = { 
            { 
                { 1280U, 720U }, { 1920U, 1080U } 
            } 
        };
    };
}
