#pragma once

namespace Rxn::Engine
{
    class RXN_ENGINE_API Simulation : public Platform::Win32::IApplication, public Platform::Win32::Window
    {
    public:
        Simulation();
        ~Simulation();


        virtual void PreInitialize() override;

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
    };

}