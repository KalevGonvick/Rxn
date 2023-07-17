#include "Rxn.h"
#include "SubComponent.h"

namespace Rxn::Platform::Win32
{
    SubComponent::SubComponent(WString className, HICON icon)
        : m_ClassName(className)
        , m_isInitialized(false)
        , m_IsInteractive(true)
        , m_ChildComponents()
        , m_Parent(nullptr)
        , m_Icon(icon)
        , m_pHWnd(nullptr)
    {
    }

    SubComponent::~SubComponent() = default;

    const WString SubComponent::GetClass()
    {
        return m_ClassName;
    }

    void SubComponent::AddChildComponent(std::shared_ptr<SubComponent> child)
    {
        if (child->m_Parent != nullptr && child->m_Parent->GetClass() == GetClass())
        {
            m_ChildComponents.push_back(child);
        }
        else if (child->m_Parent != nullptr && child->m_Parent->GetClass() != GetClass())
        {
            RXN_LOGGER::Error(L"Attempting to add child class that already has a different parent.");
        }
        else
        {
            child->m_Parent = std::shared_ptr<SubComponent>(this);
            m_ChildComponents.push_back(child);
        }
    }

    HICON SubComponent::GetIcon()
    {
        return m_Icon;
    }

    const HWND SubComponent::GetParentHandle()
    {
        if (GetParent() != nullptr)
        {
            return GetParent()->m_pHWnd;
        }
        else
        {
            return nullptr;
        }
    }

    const std::shared_ptr<SubComponent> SubComponent::GetParent()
    {
        return m_Parent;
    }


    LRESULT SubComponent::SetupMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_NCCREATE)
        {
            const CREATESTRUCTW *const pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);

            Win32::SubComponent *const pWnd = static_cast<Win32::SubComponent *>(pCreate->lpCreateParams);

            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
            SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&AssignMessageHandler));

            pWnd->m_pHWnd = hWnd;
            pWnd->m_isInitialized = true;
            pWnd->InitializeChildren();

            return pWnd->MessageHandler(hWnd, msg, wParam, lParam);
        }

        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    void SubComponent::InitializeChildren()
    {
        for (auto &child : m_ChildComponents)
        {
            child->RegisterComponentClass();
            child->InitializeWin32();
        }
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

