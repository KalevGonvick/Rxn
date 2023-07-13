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
        virtual void InitializeWin32() = 0;
        const HWND GetParentHandle();
        const std::shared_ptr<SubComponent> GetParent();
        HICON GetIcon();

        const WString GetClass();

        void AddChildComponent(std::shared_ptr<SubComponent> child);

        HWND m_pHWnd;

    protected:

        void InitializeChildren();

        static  LRESULT CALLBACK SetupMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static  LRESULT AssignMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        virtual LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



        std::list<std::shared_ptr<SubComponent>> m_ChildComponents;
        std::shared_ptr<SubComponent> m_Parent;

        bool m_isInitialized;
        WString m_ClassName;
        HICON m_Icon;



    };


} // Rxn::Platform::Win32