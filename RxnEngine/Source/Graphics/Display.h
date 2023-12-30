/*****************************************************************//**
 * \file   Display.h
 * \brief  
 * 
 * \author kalev
 * \date   December 2023
 *********************************************************************/
#pragma once
#include "SwapChain.h"
#include "Fence.h"
#include "Camera.h"

namespace Rxn::Graphics
{
    class Display
    {
    public:

        Display(int32 width, int32 height);
        ~Display();

    public:

        RXN_ENGINE_API void Init(ID3D12CommandQueue *pCmdQueue, bool hasTearingSupport);
        RXN_ENGINE_API void RotateSwapChainMarker(ID3D12CommandQueue *pCmdQueue);
        RXN_ENGINE_API void WaitForDisplay(ID3D12CommandQueue *pCmdQueue);
        RXN_ENGINE_API bool IsSizeEqual(uint32 width, uint32 height) const;

        /**
         * .
         * 
         * \param newWidth
         * \param newHeight
         */
        RXN_ENGINE_API void HandleSizeChange(uint32 newWidth, uint32 newHeight);

        /**
         * .
         * 
         */
        RXN_ENGINE_API void TurnOverSwapChainBuffer();

        /**
         * .
         * 
         * \param cam
         */
        RXN_ENGINE_API void UpdateProjectionMatrix(Camera &cam);

        /**
         * .
         * 
         * \param width
         */
        RXN_ENGINE_API void SetWidth(uint32 width);
        
        /**
         * .
         * 
         * \param height
         */
        RXN_ENGINE_API void SetHeight(uint32 height);
        
        /**
         * .
         * 
         * \return 
         */
        RXN_ENGINE_API uint32 GetHeight() const;
        
        /**
         * .
         * 
         * \return 
         */
        RXN_ENGINE_API uint32 GetWidth() const;
        
        /**
         * .
         * 
         * \return 
         */
        RXN_ENGINE_API uint32 GetFrameIndex() const;
        
        /**
         * .
         * 
         * \return 
         */
        RXN_ENGINE_API GPU::SwapChain &GetSwapChain();
        
        /**
         * .
         * 
         * \return 
         */
        RXN_ENGINE_API CD3DX12_VIEWPORT &GetViewPort();
        
        /**
         * .
         * 
         * \return 
         */
        RXN_ENGINE_API CD3DX12_RECT &GetScissorRect();
        
        /**
         * .
         * 
         * \return 
         */
        RXN_ENGINE_API const DirectX::XMMATRIX &GetProjectionMatrix() const;

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
        GPU::Fence m_Fence;
    };
}
