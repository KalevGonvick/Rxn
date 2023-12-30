#include "Rxn.h"
#include "SplashScreen.h"

namespace Rxn::Engine
{
    SplashWindow::SplashWindow(const WString &windowTitle, const WString &windowClass, int32 width, int32 height)
        : Platform::Win32::Window(windowTitle, windowClass, width, height)
    {}

    SplashWindow::~SplashWindow() = default;

    void SplashWindow::AddMessage(const wchar_t *msg)
    {
        PostMessage(m_HWnd, WM_OUTPUTMESSAGE, (WPARAM)msg, 0);
    }

    LRESULT SplashWindow::MessageHandler(HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_PAINT:
        {
            HDC hdc;
            PAINTSTRUCT ps;

            hdc = BeginPaint(hWnd, &ps);

            Platform::Win32::Window::AddBitmap(Engine::EngineContext::GetEngineSettings().GetSplashScreenURL(), hdc, 0, 0);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));

            SetTextAlign(hdc, TA_CENTER);

            TextOut(hdc, m_Size.cx / 2, m_Size.cy - 30, m_pwOutputMessage, static_cast<int32>(wcslen(m_pwOutputMessage)));
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_OUTPUTMESSAGE:
        {
            const WCHAR *cMsg = (WCHAR *)wParam;
            wcscpy_s(m_pwOutputMessage, cMsg);
            Redraw();
            return 0;
        }
        default:
        {
            break;
        }
       
        }
        return Window::MessageHandler(hWnd, msg, wParam, lParam);
    }
    void SplashWindow::SetupWindow()
    {
        m_WindowStyle = Platform::Win32::WindowStyle::POPUP;
        m_IsInteractive = false;

        RegisterComponentClass();
        InitializeWin32();

        ShowWindow(m_HWnd, SW_SHOW);
        UpdateWindow(m_HWnd);
    }
}