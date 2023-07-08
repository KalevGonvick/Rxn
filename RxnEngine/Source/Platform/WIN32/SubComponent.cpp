#include "Rxn.h"
#include "SubComponent.h"

namespace Rxn::Platform::Win32
{
    SubComponent::SubComponent(WString className, WString classTitle, HICON icon)
        : m_pszClass(className)
        , m_pszTitle(classTitle)
        , m_hIcon(icon)
        , m_pHWnd(nullptr)
    {
    }

    SubComponent::~SubComponent() = default;

    WString &SubComponent::GetClass()
    {
        return this->m_pszClass;
    }

    WString &SubComponent::GetTitle()
    {
        return this->m_pszTitle;
    }

    HICON SubComponent::GetIcon()
    {
        return this->m_hIcon;
    }

    HWND SubComponent::GetHandle()
    {
        return this->m_pHWnd;
    }

    void SubComponent::SetHandle(HWND hWnd)
    {
        this->m_pHWnd = hWnd;
    }

    LRESULT SubComponent::SetupMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_NCCREATE)
        {
            const CREATESTRUCTW *const pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);

            Win32::SubComponent *const pWnd = static_cast<Win32::SubComponent *>(pCreate->lpCreateParams);

            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
            SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&AssignMessageHandler));
            pWnd->SetHandle(hWnd);
            return pWnd->MessageHandler(hWnd, msg, wParam, lParam);
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    LRESULT SubComponent::AssignMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Win32::SubComponent *const pWnd = reinterpret_cast<SubComponent *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        return pWnd->MessageHandler(hWnd, msg, wParam, lParam);
    }

    LRESULT SubComponent::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }


} // Rxn::Platform::Win32

