#include "Rxn.h"
#include "SplashScreen.h"


namespace Rxn::Engine::SplashScreen
{

    SplashWindow *m_SplashWindow;

    void Open()
    {
        if (m_SplashWindow != nullptr)
            return;

        m_SplashWindow = new SplashWindow();
    }

    void Close()
    {
        return void RXN_ENGINE_API();
    }

    void AddMessage(const wchar_t *msg)
    {
        PostMessage(m_SplashWindow->GetHandle(), WM_OUTPUTMESSAGE, (WPARAM)msg, 0);
    }

}

namespace Rxn::Engine
{
    SplashWindow::SplashWindow()
        : Platform::Win32::Window(L"SplashScreen", 0)
    {
        wcscpy_s(m_pwOutputMessage, L"SplashScreen Starting...");

        this->Platform::Win32::Window::SetWindowStyle(Platform::Win32::WindowStyle::POPUP);
        this->Platform::Win32::Window::SetSize(500, 600);
        this->Platform::Win32::Window::RegisterComponentClass();
        this->Platform::Win32::Window::Initialize();


    }

    SplashWindow::~SplashWindow()
    {
    }

    LRESULT SplashWindow::MessageHandler(HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_PAINT:
        {
            HBITMAP hbitmap;
            HDC hdc, hmemdc;
            PAINTSTRUCT ps;

            hdc = BeginPaint(hWnd, &ps);

            Platform::Win32::Window::AddBitmap(Engine::EngineContext::GetEngineSettings().GetSplashScreenURL(), hdc, 0, 0);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));

            if (Engine::Runtime::GetEngineMode() != Engine::Runtime::EngineRuntimeMode::PRODUCTION)
            {
                WString engineModeText = Engine::Runtime::GetEngineModeString() + L"Mode";
                SetTextAlign(hdc, TA_RIGHT);
                TextOut(hdc, this->GetSize().cx - 15, 15, engineModeText.c_str(), wcslen(engineModeText.c_str()));
            }

            SetTextAlign(hdc, TA_CENTER);

            TextOut(hdc, this->GetSize().cx / 2, this->GetSize().cy - 30, m_pwOutputMessage, wcslen(m_pwOutputMessage));
            EndPaint(hWnd, &ps);

        }
        break;
        case WM_OUTPUTMESSAGE:
        {
            WCHAR *msg = (WCHAR *)wParam;
            wcscpy_s(m_pwOutputMessage, msg);
            Redraw();

        }
        return 0;

        }
        return Window::MessageHandler(hWnd, msg, wParam, lParam);
    }
}