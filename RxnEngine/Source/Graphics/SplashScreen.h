#pragma once

namespace Rxn::Engine
{
    constexpr uint32 MAX_NAME_STRING_LENGTH = 256;
    class RXN_ENGINE_API SplashWindow : public Platform::Win32::Window
    {
    public:

        SplashWindow(const WString &windowClass, const WString &windowTitle, int32 width, int32 height);
        ~SplashWindow();

    public:

        void AddMessage(const wchar_t *msg);

        virtual LRESULT MessageHandler(HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam) override;
        virtual void SetupWindow() override;

    private:

        wchar_t m_pwOutputMessage[MAX_NAME_STRING_LENGTH] = L"...";
    };
}

