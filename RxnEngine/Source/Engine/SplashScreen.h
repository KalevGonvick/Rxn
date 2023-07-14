#pragma once

#include "Platform/WIN32/Window.h"

namespace Rxn::Engine
{
    class RXN_ENGINE_API SplashWindow : public Platform::Win32::Window
    {
    public:

        SplashWindow(WString windowClass, WString windowTitle);
        ~SplashWindow();

        virtual void Render() override {};

    public:

        void AddMessage(const wchar_t *msg);

        virtual LRESULT MessageHandler(HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam) override;
        virtual void SetupWindow() override;

    private:

        wchar_t m_pwOutputMessage[Constants::MAX_NAME_STRING_LENGTH];
    };
}

