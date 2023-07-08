#pragma once

#include "Platform/WIN32/Window.h"

namespace Rxn::Engine::SplashScreen
{
    void RXN_ENGINE_API Open();
    void RXN_ENGINE_API Close();
    void RXN_ENGINE_API AddMessage(const wchar_t * msg);

}

namespace Rxn::Engine
{
    class RXN_ENGINE_API SplashWindow : public Platform::Win32::Window
    {
    public:

        SplashWindow();
        ~SplashWindow();

    public:

        virtual LRESULT MessageHandler(HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam) override;

    private:

        wchar_t m_pwOutputMessage[Constants::kusMaxNameString];
    };
}

