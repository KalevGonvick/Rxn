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
        : public Renderer
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
        LRESULT MessageHandler(HWND hWnd, uint32 msg, WPARAM wParam, LPARAM lParam) override;

        virtual void SetupWindow() override;

        void UpdateSimulation();
        virtual void InitializeRender() override;
        virtual void ShutdownRender() override;
        virtual void RenderPass() override;
        virtual void PreRenderPass() override;
        virtual void PostRenderPass() override;


    protected:

        /**
         * Handles key down events.
         *
         * \param key - key code.
         */
        virtual void HandleKeyDown(uint8 key) override;

        /**
         * Handles key up events.
         *
         * \param key - key code.
         */
        virtual void HandleKeyUp(uint8 key) override;

    private:

        HRESULT OnSizeChange();
        float64 m_LastDrawTime;
        uint32 m_FrameCount;
        void DestroySwapChainResources();
        uint32 GetFPS();

    };
}
