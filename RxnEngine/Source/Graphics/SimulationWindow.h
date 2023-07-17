/*****************************************************************//**
 * \file   SimulationWindow.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once

namespace Rxn::Graphics
{
    class RXN_ENGINE_API SimulationWindow
        : public RenderFramework
        , public Platform::Win32::Window
    {
    public:

        SimulationWindow(WString windowTitle, WString windowClass, int width, int height);
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
        LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

        virtual void SetupWindow() override;

    protected:

        virtual void HandleKeyDown(uint8 key) override;
        virtual void HandleKeyUp(uint8 key) override;

    private:

        HRESULT CreatePipelineSwapChain();
        HRESULT OnSizeChange();

        void DestroySwapChainResources();


    };
}
