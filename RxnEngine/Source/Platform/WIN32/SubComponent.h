#pragma once
#include <list>
#include <memory>

namespace Rxn::Platform::Win32
{
    class RXN_ENGINE_API SubComponent
    {
    public:

        SubComponent(WString className, HICON icon);
        ~SubComponent();

    public:

        virtual void RegisterComponentClass() = 0;
        virtual void Initialize() = 0;

        WString const &GetClass();
        void AddChildComponent(std::shared_ptr<SubComponent> child);

        HICON GetIcon();
        const HWND &GetParentHandle();

        const std::shared_ptr<SubComponent> GetParent();

    protected:

        void InitializeChildren();

        static  LRESULT CALLBACK SetupMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static  LRESULT AssignMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        virtual LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        HWND m_pHWnd;

        std::list<std::shared_ptr<SubComponent>> m_ChildComponents;
        std::shared_ptr<SubComponent> m_Parent;

    private:

        bool m_isInitialized;
        WString m_ClassName;
        HICON m_Icon;

    };


} // Rxn::Platform::Win32