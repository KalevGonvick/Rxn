#include "Rxn.h"
#include "Window.h"

namespace Rxn::Platform::Win32
{
    Window::Window(const WString &className, const WString &title, int32 width, int32 height)
        : SubComponent(className, nullptr)
        , m_TitleName(title)
        , m_Size(SIZE(width, height))
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
        wcex.hbrBackground = CreateSolidBrush(m_WindowBackgroundColour);
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

        m_HWnd = CreateWindow(
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

        if (!m_HWnd)
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
            HandleNonClientCreate();
            return 1;
        }
        case WM_NCACTIVATE:
        {
            HandleNonClientActivate(LOWORD(wParam) != WA_INACTIVE);
            return 1;
        }
        case WM_NCPAINT:
        {
            HandleNonClientPaint((HRGN)wParam);
            return 0;
        }
        case WM_NCMBUTTONDBLCLK:
        {
            HandleNonClientAreaDoubleClick();
            return 0;
        }
        case WM_NCLBUTTONDOWN:
        {
            HandleNonClientLeftClickDown();
            break;
        }
        case WM_SIZING:
        case WM_MOVING:
        
        {
            Redraw();
            return 1;
        }
        case WM_ENTERSIZEMOVE:
        case WM_MOVE:
        {
            Redraw();
            return 0;
        }
        case WM_SIZE:
        {
            Redraw();
            break;
        }
        case WM_TIMER:
        {
            Redraw();
            break;
        }
        default:
            break;
        }

        return SubComponent::MessageHandler(hWnd, msg, wParam, lParam);
    }

    void Window::ModifyClassStyle(const HWND &hWnd, const DWORD &flagsToDisable, const DWORD &flagsToEnable) const
    {
        DWORD style = GetWindowLong(hWnd, GCL_STYLE);
        SetClassLong(hWnd, GCL_STYLE, (style & ~flagsToDisable) | flagsToEnable);
    }

    void Window::MaximizeWindow(const HWND &hwnd) const
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
        HDC hdc = GetDCEx(m_HWnd, region, DCX_WINDOW | DCX_INTERSECTRGN | DCX_USESTYLE);

        RECT rect;
        GetWindowRect(m_HWnd, &rect);
        auto size = SIZE{ rect.right - rect.left, rect.bottom - rect.top };
        auto adjustedRect = RECT{ 0, 0, size.cx, size.cy };

        PaintWindowBorder(hdc, adjustedRect);
        PaintWindowConditionalHighlight(hdc, adjustedRect);
        PaintWindowCaption(hdc, size);

        ReleaseDC(m_HWnd, hdc);
    }

    void Window::HandleNonClientLeftClickDown()
    {
        POINT pt;
        GetCursorPos(&pt);

        RECT rect;
        GetWindowRect(m_HWnd, &rect);

        Command cmd = m_WindowCaption.GetWindowCaptionButtonClicked(pt, rect);

        switch (cmd)
        {
        using enum Rxn::Platform::Win32::Command;
        case CB_CLOSE:
        {
            SendMessage(m_HWnd, WM_CLOSE, 0, 0);
            break;
        }
        case CB_MAXIMIZE:
        {
            MaximizeWindow(m_HWnd);
            break;
        }
        case CB_MINIMIZE:
        {
            ShowWindow(m_HWnd, SW_MINIMIZE);
            break;
        }
        default:
            break;
        }

    }

    void Window::HandlePaint()
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_HWnd, &ps);

        RECT rc;
        GetClientRect(m_HWnd, &rc);

        HBRUSH brush = CreateSolidBrush(m_WindowBackgroundColour);
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);

        EndPaint(m_HWnd, &ps);
    }

    void Window::PaintWindowBorder(const HDC &hdc, const RECT &rect) const
    {
        HBRUSH brush = CreateSolidBrush(m_WindowBorderColour);
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }

    void Window::PaintWindowConditionalHighlight(const HDC &hdc, const RECT &rect) const
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
        SetWindowPos(m_HWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_FRAMECHANGED);
        InvalidateRect(m_HWnd, nullptr, false);
    }

    void Window::HandleNonClientAreaDoubleClick()
    {
        MaximizeWindow(m_HWnd);
    }

    void Window::HandleNonClientCreate()
    {
        if (m_IsInteractive)
        {
            RXN_LOGGER::Trace(L"Window %s is interactive, setting invalidate timer", m_ClassName.c_str());
            SetTimer(m_HWnd, 1, 250, nullptr);
        }


        RXN_LOGGER::Trace(L"Removing default theme from window %s.", m_ClassName.c_str());
        SetWindowTheme(m_HWnd, L"", L"");

        RXN_LOGGER::Trace(L"Adding dropshadow to window '%s'.", m_ClassName.c_str());
        ModifyClassStyle(m_HWnd, 0, CS_DROPSHADOW);

        if (m_AddCloseButton)
        {
            RXN_LOGGER::Trace(L"Window %s has a close button, pushing caption", m_ClassName.c_str());
            m_WindowCaption.AddButton(L"X", Command::CB_CLOSE);
        }


        if (m_AddMaximizeButton)
        {
            RXN_LOGGER::Trace(L"Window %s has a maximize button, pushing caption", m_ClassName.c_str());
            m_WindowCaption.AddButton(L"🗖", Command::CB_MAXIMIZE);
        }


        if (m_AddMinimizeButton)
        {
            RXN_LOGGER::Trace(L"Window %s has a minimize button, pushing caption", m_ClassName.c_str());
            m_WindowCaption.AddButton(L"🗕", Command::CB_MINIMIZE);
        }

    }

    void Window::HandleNonClientActivate(const int &active)
    {
        m_Active = active;
    }

    int Window::AddBitmap(const wchar_t *szFileName, const HDC &hWinDC, int x, int y)
    {
        BITMAP qBitmap{};
        HDC hLocalDC = CreateCompatibleDC(hWinDC);

        auto hBitmap = (HBITMAP)LoadImage(nullptr, szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

        if (hBitmap == nullptr)
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

    void Window::PaintWindowCaptionTitle(const HDC &hdc, const SIZE &size) const
    {
        if (m_WindowCaption.m_ShowTitle)
        {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, m_Active ? m_WindowTitleActiveTextColour : m_WindowTitleInactiveTextColour);
        }
    }

    void Window::PaintWindowCaptionButtons(const HDC &hdc, const SIZE &size)
    {
        for (const auto &button : m_WindowCaption.GetButtons())
        {
            //button->rect = RECT{ size.cx - button->width - button->offset, 0, size.cx - button->offset, 30 };
            button->rect.left = size.cx - button->width - button->offset;
            button->rect.right = 0;
            button->rect.bottom = 30;
        }
    }

}

