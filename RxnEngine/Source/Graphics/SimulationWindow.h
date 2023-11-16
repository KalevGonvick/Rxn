/*****************************************************************//**
 * \file   SimulationWindow.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once

#include "Renderer.h"

namespace Rxn::Graphics
{
    class RXN_ENGINE_API SimulationWindow
        : public Renderer
        , public Platform::Win32::Window
    {
    public:

        SimulationWindow(const WString &windowTitle, const WString &windowClass, int width, int height);
        ~SimulationWindow();

    public:

        /**
         * MessageHandler - Handles WIN32 messages.
         *
         * \param hWnd      - handle
         * \param msg       - message to be processed.
         * \param wParam    - additional wparam from msg (optional)
         * \param lParam    - additional lparam from msg (optional)
         * \return
         */
        LRESULT MessageHandler(HWND hWnd, uint32 msg, WPARAM wParam, LPARAM lParam) override;

        void SetupWindow() override;

        void UpdateSimulation();
        void InitializeRender() override;
        void ShutdownRender() override;
        void RenderPass() override;
        void PreRenderPass() override;


    protected:

        /**
         * Handles key down events.
         *
         * \param key - key code.
         */
        void HandleKeyDown(uint8 key) override;

        /**
         * Handles key up events.
         *
         * \param key - key code.
         */
        void HandleKeyUp(uint8 key) override;

    private:

        void OnSizeChange();
        void DestroySwapChainResources();
        void ResetFrameCommandObjects(ComPointer<ID3D12GraphicsCommandList> frameCmdList, ComPointer<ID3D12CommandAllocator> frameCmdAllocator) const;
        void UpdateShaderParameters(ComPointer<ID3D12GraphicsCommandList> frameCmdList, const uint32 frameIndex);
        uint32 GetFPS() const;

    private:

        float64 m_LastDrawTime = 0;
        uint32 m_FrameCount = 0;

    };
}
