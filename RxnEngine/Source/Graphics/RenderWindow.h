/*****************************************************************//**
 * \file   RenderWindow.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once

#include "Renderer.h"

namespace Rxn::Graphics
{
    class RXN_ENGINE_API RenderWindow
        : public Renderer
        , public Platform::Win32::Window
    {
    public:

        RenderWindow(const WString &windowTitle, const WString &windowClass, int32 width, int32 height);
        ~RenderWindow();

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
        void RenderPass() override;

        void UpdateSimulation();


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

        /**
         * .
         * 
         */
        void OnSizeChange();

        /**
         * .
         * 
         * \return 
         */
        uint64 GetFPS() const;

    private:

        float64 m_LastDrawTime = 0;
        uint32 m_FrameCount = 0;

    };
}
