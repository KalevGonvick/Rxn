﻿#include "Rxn.h"
#include "Window.h"

namespace Rxn::Platform::Win32
{
    Window::Window(WString className, WString title)
        : SubComponent(className, 0)
        , m_TitleName(title)
        , m_Active(0)
        , m_WindowStyle(WindowStyle::RESIZEABLE)
        , m_Size(SIZE(Constants::Win32::DEFAULT_WINDOW_WIDTH, Constants::Win32::DEFAULT_WINDOW_HEIGHT))
        , m_WindowBackgroundColour(RGB(36, 36, 36))
        , m_WindowBorderColour(RGB(46, 46, 46))
        , m_WindowActiveBorderHighlightColour(RGB(155, 80, 255))
        , m_WindowTitleActiveTextColour(RGB(255, 255, 255))
        , m_WindowTitleInactiveTextColour(RGB(92, 92, 92))
        , m_AddCloseButton(false)
        , m_AddMaximizeButton(false)
        , m_AddMinimizeButton(false)
        , m_WindowCaption()
    {}

    Window::~Window() = default;

    void Window::RegisterComponentClass()
    {
        RXN_LOGGER::Trace(L"Registering new win32 component %s", m_ClassName.c_str());

        WNDCLASSEX wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)CreateSolidBrush(m_WindowBackgroundColour);
        wcex.hIcon = GetIcon();
        wcex.hIconSm = GetIcon();
        wcex.lpszClassName = m_ClassName.c_str();
        wcex.lpszMenuName = nullptr;
        wcex.hInstance = HInstance();
        wcex.lpfnWndProc = SetupMessageHandler;

        RegisterClassEx(&wcex);
    }

    void Window::InitializeWin32()
    {

        RECT desktop;
        const HWND hDesktop = GetDesktopWindow();
        GetWindowRect(hDesktop, &desktop);

        HWND parent = GetParentHandle();

        m_pHWnd = CreateWindow(
            m_ClassName.c_str(),
            m_TitleName.c_str(),
            m_WindowStyle,
            ((desktop.right / 2) - (m_Size.cx / 2)),
            ((desktop.bottom / 2) - (m_Size.cy / 2)),
            m_Size.cx,
            m_Size.cy,
            parent,
            nullptr,
            HInstance(),
            (void *)this
        );

        if (!m_pHWnd)
        {
            RXN_LOGGER::Error(L"Failed to create window %s.", m_TitleName.c_str());
            return;
        }
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
        //case WM_PAINT:
        //{
        //    //this->HandlePaint();
        //    break;
        //}
        //case WM_NCMBUTTONDBLCLK:
        //{
        //    //this->HandleNonClientAreaDoubleClick();
        //    return 0;
        //}
        case WM_NCLBUTTONDOWN:
        {
            this->HandleNonClientLeftClickDown();
            break;
        }
        //case WM_SIZING:
        //case WM_MOVING:
        //{
        //    this->Redraw();
        //    return 1;
        //}
        case WM_ENTERSIZEMOVE:
        case WM_MOVE:
        case WM_SIZE:
        {
            this->Redraw();
            return 0;
        }
        //case WM_TIMER:
        //{
        //    this->Redraw();
        //    return 0;
        //}
        }

        return SubComponent::MessageHandler(hWnd, msg, wParam, lParam);
    }

    void Window::ModifyClassStyle(const HWND &hWnd, const DWORD &flagsToDisable, const DWORD &flagsToEnable)
    {
        DWORD style = GetWindowLong(hWnd, GCL_STYLE);
        SetClassLong(hWnd, GCL_STYLE, (style & ~flagsToDisable) | flagsToEnable);
    }

    void Window::MaximizeWindow(const HWND &hwnd)
    {
        WINDOWPLACEMENT wPos{};
        GetWindowPlacement(hwnd, &wPos);

        if (wPos.showCmd == SW_MAXIMIZE)
        {
            ShowWindow(hwnd, SW_NORMAL);
        }
        else
        {
            ShowWindow(hwnd, SW_MAXIMIZE);
        }
    }

    void Window::HandleNonClientPaint(const HRGN &region)
    {
        HDC hdc = GetDCEx(m_pHWnd, region, DCX_WINDOW | DCX_INTERSECTRGN | DCX_USESTYLE);

        RECT rect;
        GetWindowRect(m_pHWnd, &rect);
        SIZE size = SIZE{ rect.right - rect.left, rect.bottom - rect.top };
        RECT adjustedRect = RECT{ 0, 0, size.cx, size.cy };

        PaintWindowBorder(hdc, adjustedRect);
        PaintWindowConditionalHighlight(hdc, adjustedRect);
        PaintWindowCaption(hdc, size);

        ReleaseDC(m_pHWnd, hdc);
    }

    void Window::HandleNonClientLeftClickDown()
    {
        POINT pt;
        GetCursorPos(&pt);

        RECT rect;
        GetWindowRect(m_pHWnd, &rect);

        Command cmd = m_WindowCaption.GetWindowCaptionButtonClicked(pt, rect);

        switch (cmd)
        {
        case Command::CB_CLOSE:
        {
            RXN_LOGGER::Info(L"Clicked max button...");
            //SendMessage(m_pHWnd, WM_CLOSE, 0, 0);
            break;
        }
        case Command::CB_MAXIMIZE:
        {
            RXN_LOGGER::Info(L"Clicked max button...");
            //MaximizeWindow(m_pHWnd);
            break;
        }
        case Command::CB_MINIMIZE:
        {
            RXN_LOGGER::Info(L"Clicked min button...");
            //ShowWindow(m_pHWnd, SW_MINIMIZE);
            break;
        }
        case Command::CB_NOP:
        default:
            break;
        }

    }

    void Window::HandlePaint()
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_pHWnd, &ps);

        RECT rc;
        GetClientRect(m_pHWnd, &rc);

        HBRUSH brush = CreateSolidBrush(m_WindowBackgroundColour);
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);

        EndPaint(m_pHWnd, &ps);
    }

    void Window::PaintWindowBorder(const HDC &hdc, const RECT &rect)
    {
        HBRUSH brush = CreateSolidBrush(m_WindowBorderColour);
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }

    void Window::PaintWindowConditionalHighlight(const HDC &hdc, const RECT &rect)
    {
        if (m_Active)
        {
            HBRUSH brush = CreateSolidBrush(m_WindowActiveBorderHighlightColour);
            FrameRect(hdc, &rect, brush);
            DeleteObject(brush);
        }
    }

    void Window::Redraw()
    {
        /* reset window */
        SetWindowPos(m_pHWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_FRAMECHANGED);
        SendMessage(m_pHWnd, WM_PAINT, 0, 0);
    }

    void Window::HandleNonClientAreaDoubleClick()
    {
        MaximizeWindow(m_pHWnd);
    }

    void Window::HandleNonClientCreate()
    {
        //SetTimer(m_pHWnd, 1, 15, 0);

        RXN_LOGGER::Trace(L"Removing default windows theme.");
        SetWindowTheme(m_pHWnd, L"", L"");

        RXN_LOGGER::Trace(L"Adding dropshadow.");
        ModifyClassStyle(m_pHWnd, 0, CS_DROPSHADOW);

        if (m_AddCloseButton)
            m_WindowCaption.AddButton(L"X", Command::CB_CLOSE);

        if (m_AddMaximizeButton)
            m_WindowCaption.AddButton(L"🗖", Command::CB_MAXIMIZE);

        if (m_AddMinimizeButton)
            m_WindowCaption.AddButton(L"🗕", Command::CB_MINIMIZE);
    }

    void Window::HandleNonClientActivate(const int &active)
    {
        m_Active = active;
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

    void Window::PaintWindowCaption(const HDC &hdc, const SIZE &size)
    {
        this->PaintWindowCaptionTitle(hdc, size);
        this->PaintWindowCaptionButtons(hdc, size);
    }

    void Window::PaintWindowCaptionTitle(const HDC &hdc, const SIZE &size)
    {
        if (m_WindowCaption.ShowTitle())
        {
            RECT adjustedRect = RECT{ 0, 0, size.cx, 30 };
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, m_Active ? m_WindowTitleActiveTextColour : m_WindowTitleInactiveTextColour);
            DrawText(hdc, m_TitleName.c_str(), wcslen(m_TitleName.c_str()), &adjustedRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
        }
    }

    void Window::PaintWindowCaptionButtons(const HDC &hdc, const SIZE &size)
    {
        for (auto &button : m_WindowCaption.GetButtons())
        {
            button->rect = RECT{ size.cx - button->width - button->offset, 0, size.cx, 30 };
            DrawText(hdc, button->txt.c_str(), wcslen(button->txt.c_str()), &button->rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
        }
    }

}

