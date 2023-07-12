#include "Rxn.h"
#include "SplashScreen.h"


namespace Rxn::Engine::SplashScreen
{

}

namespace Rxn::Engine
{
    SplashWindow::SplashWindow(WString windowTitle, WString windowClass)
        : Platform::Win32::Window(windowTitle, windowClass)
        , m_pwOutputMessage(L"...")
    {
    }

    SplashWindow::~SplashWindow() = default;

    void SplashWindow::AddMessage(const wchar_t *msg)
    {
        PostMessage(m_pHWnd, WM_OUTPUTMESSAGE, (WPARAM)msg, 0);
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

            /*if (Engine::Runtime::GetEngineMode() != Engine::Runtime::EngineRuntimeMode::PRODUCTION)
            {
                WString engineModeText = Engine::Runtime::GetEngineModeString() + L"Mode";
                SetTextAlign(hdc, TA_RIGHT);
                TextOut(hdc, this->GetSize().cx - 15, 15, engineModeText.c_str(), wcslen(engineModeText.c_str()));
            }*/

            SetTextAlign(hdc, TA_CENTER);

            TextOut(hdc, m_Size.cx / 2, m_Size.cy - 30, m_pwOutputMessage, wcslen(m_pwOutputMessage));
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