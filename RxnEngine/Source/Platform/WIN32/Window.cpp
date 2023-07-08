#include "Rxn.h"
#include "Window.h"

namespace Rxn::Platform::Win32
{
    Window::Window(RWString title, HICON icon)
        : SubComponent(title, title, icon)
        , m_iActive(0)
        , m_ulWindowStyle(WindowStyle::RESIZEABLE)
        , m_xySize(SIZE(Constants::Win32::kulDefaultWindowWidth, Constants::Win32::kulDefaultWindowHeight))
        , m_xyzulWindowBackgroundColour(RGB(36, 36, 36))
        , m_xyzulWindowBorderColour(RGB(46, 46, 46))
        , m_WindowCaption()
    {
    }

    Window::~Window()
    {
    }

    DWORD Window::GetWindowStyle()
    {
        return this->m_ulWindowStyle;
    }

    COLORREF Window::GetWindowBackgroundColour()
    {
        return this->m_xyzulWindowBackgroundColour;
    }

    COLORREF Window::GetWindowBorderColour()
    {
        return this->m_xyzulWindowBorderColour;
    }

    SIZE Window::GetSize()
    {
        return this->m_xySize;
    }

    void Window::SetWindowStyle(const DWORD &style)
    {
        RXN_LOGGER::Trace(L"Setting window style to: %d", std::to_string(style));
        this->m_ulWindowStyle = style;
    }

    void Window::SetSize(const SIZE &size)
    {
        this->m_xySize = size;
    }

    void Window::SetSize(const int &cx, const int &cy)
    {
        this->m_xySize.cx = cx;
        this->m_xySize.cy = cy;
    }

    void Window::SetVerticalSize(const int &cy)
    {
        this->m_xySize.cy = cy;
    }

    void Window::SetHorizontalSize(const int &cx)
    {
        this->m_xySize.cx = cx;
    }

    void Window::SetWindowBackgroundColour(const COLORREF &colour)
    {
        this->m_xyzulWindowBackgroundColour = colour;
    }

    void Window::SetWindowBorderColour(const COLORREF &colour)
    {
        this->m_xyzulWindowBorderColour = colour;
    }

    void Window::RegisterComponentClass()
    {
        RXN_LOGGER::Trace(L"Registering new win32 component %s", this->GetClass());

        WNDCLASSEX wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)CreateSolidBrush(this->GetWindowBackgroundColour());
        wcex.hIcon = GetIcon();
        wcex.hIconSm = GetIcon();
        wcex.lpszClassName = GetClass().c_str();
        wcex.lpszMenuName = nullptr;
        wcex.hInstance = HInstance();
        wcex.lpfnWndProc = SetupMessageHandler;

        RegisterClassEx(&wcex);
    }

    void Window::Initialize()
    {

        /* get desktop */
        RECT desktop;
        const HWND hDesktop = GetDesktopWindow();
        GetWindowRect(hDesktop, &desktop);

        this->m_pHWnd = CreateWindow(
            this->GetClass().c_str(),
            this->GetTitle().c_str(),
            this->GetWindowStyle(),
            ((desktop.right / 2) - (GetSize().cx / 2)),
            ((desktop.bottom / 2) - (GetSize().cy / 2)),
            GetSize().cx,
            GetSize().cy,
            nullptr,
            nullptr,
            HInstance(),
            (void *)this
        );

        if (!this->m_pHWnd)
        {
            RXN_LOGGER::Error(L"Failed to create window %s.", this->GetTitle().c_str());
            return;
        }

        ShowWindow(this->m_pHWnd, SW_SHOW);
        UpdateWindow(this->m_pHWnd);
    }

    LRESULT Window::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_NCCREATE:
        {
            this->HandleNonClientCreate();
            return 1;
        }

        case WM_NCACTIVATE:
        {
            this->HandleNonClientActivate(LOWORD(wParam) != WA_INACTIVE);
            return 1;
        }
        case WM_NCPAINT:
        {
            this->HandleNonClientPaint((HRGN)wParam);
            return 0;
        }
        case WM_PAINT:
        {
            break;
        }
        case WM_SIZING:
        case WM_MOVING:
        {
            this->Redraw();
            return 1;
        }
        case WM_ENTERSIZEMOVE:
        case WM_MOVE:
        case WM_SIZE:
        {
            this->Redraw();
            return 0;
        }
        case WM_TIMER:
        {
            this->Redraw();
            break;
        }
        }

        return SubComponent::MessageHandler(hWnd, msg, wParam, lParam);
    }

    void Window::ModifyClassStyle(const HWND &hWnd, const DWORD &flagsToDisable, const DWORD &flagsToEnable)
    {
        DWORD style = GetWindowLong(hWnd, GCL_STYLE);
        SetClassLong(hWnd, GCL_STYLE, (style & ~flagsToDisable) | flagsToEnable);
    }

    void Window::HandleNonClientPaint(const HRGN &region)
    {
        HDC hdc = GetDCEx(GetHandle(), region, DCX_WINDOW | DCX_INTERSECTRGN | DCX_USESTYLE);

        RECT rect;
        GetWindowRect(GetHandle(), &rect);
        SIZE size = SIZE{ rect.right - rect.left, rect.bottom - rect.top };
        RECT adjustedRect = RECT{ 0, 0, size.cx, size.cy };

        /* this was in the tutorial, but I don't think it does anything */
        //HBITMAP bitMapHandle = CreateCompatibleBitmap(hdc, size.cx, size.cy);
        //HANDLE hOld = SelectObject(hdc, bitMapHandle);

        this->PaintWindowBorder(hdc, adjustedRect);
        this->PaintWindowConditionalHighlight(hdc, adjustedRect);

        this->PaintWindowCaption(hdc, rect, size);

        /* this was in the tutorial, but I don't think it does anything */
        //BitBlt(hdc, 0, 0, size.cx, size.cy, hdc, 0, 0, SRCCOPY);
        //SelectObject(hdc, hOld);
        //DeleteObject(bitMapHandle);

        //DeleteObject(brush);
        ReleaseDC(GetHandle(), hdc);
    }

    void Window::PaintWindowBorder(const HDC &hdc, const RECT &rect)
    {
        HBRUSH brush = CreateSolidBrush(this->GetWindowBorderColour());
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }

    void Window::PaintWindowConditionalHighlight(const HDC &hdc, const RECT &rect)
    {
        if (this->GetActive())
        {
            HBRUSH brush = CreateSolidBrush(RGB(230, 10, 255));
            FrameRect(hdc, &rect, brush);
            DeleteObject(brush);
        }
    }

    void Window::Redraw()
    {
        /* reset window */
        SetWindowPos(GetHandle(), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_FRAMECHANGED);
        SendMessage(GetHandle(), WM_PAINT, 0, 0);
    }

    void Window::HandleNonClientCreate()
    {
        /* This is probably not correct, but this is what the tutorial did haha. */
        //SetTimer(GetHandle(), 1, 144, 0);
        RXN_LOGGER::Trace(L"Setting window theme.");
        SetWindowTheme(GetHandle(), L"", L"");

        RXN_LOGGER::Trace(L"Modyfying class style.");
        this->ModifyClassStyle(GetHandle(), 0, CS_DROPSHADOW);
    }

    void Window::HandleNonClientActivate(const int &active)
    {
        SetActive(active);
    }

    void Window::SetCaption(const Caption &caption)
    {
        this->m_WindowCaption = caption;
    }

    void Window::SetActive(const int &active)
    {
        this->m_iActive = active;
    }

    int Window::GetActive()
    {
        return this->m_iActive;
    }

    Caption &Window::GetCaption()
    {
        return this->m_WindowCaption;
    }

    int Window::AddBitmap(const wchar_t *szFileName, const HDC &hWinDC, int x, int y)
    {
        BITMAP qBitmap{};
        HDC hLocalDC = CreateCompatibleDC(hWinDC);

        HBITMAP hBitmap = (HBITMAP)LoadImage(0, szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

        if (hBitmap == 0)
        {
            RXN_LOGGER::Error(L"Failed to load image from URL %s", szFileName);
            return 0;
        }
        GetObject(reinterpret_cast<HGDIOBJ>(hBitmap), sizeof(BITMAP), reinterpret_cast<LPVOID>(&qBitmap));

        SelectObject(hLocalDC, hBitmap);

        if (!BitBlt(hWinDC, x, y, qBitmap.bmWidth, qBitmap.bmHeight, hLocalDC, 0, 0, SRCCOPY))
        {
            RXN_LOGGER::Error(L"Failed Blit for image URL %s.", szFileName);
            return 0;
        }

        ::DeleteDC(hLocalDC);
        ::DeleteObject(hBitmap);
        return 1;
    }

    void Window::PaintWindowCaption(const HDC &hdc, const RECT &rect, const SIZE &size)
    {
        RXN_LOGGER::Trace(L"Setting window text with to %s", this->GetTitle().c_str());

        if (this->GetCaption().ShowTitle())
        {
            RECT adjustedRect = RECT{ 0, 0, size.cx, 30 };
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, GetActive() ? RGB(255, 255, 255) : RGB(92, 92, 92));
            DrawText(hdc, this->GetTitle().c_str(), wcslen(this->GetTitle().c_str()), &adjustedRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
        }

        RWString closeText = L"X";
        int buttonWidth = 50;
        RECT buttonRect = RECT{ size.cx - buttonWidth, 0, size.cx, 30 };

        DrawText(hdc, closeText.c_str(), wcslen(closeText.c_str()), &buttonRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
    }
}

