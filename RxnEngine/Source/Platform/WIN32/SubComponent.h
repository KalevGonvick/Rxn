#pragma once

namespace Rxn::Platform::Win32
{
    class RXN_ENGINE_API SubComponent
    {
    public:
        
        SubComponent(std::wstring className, std::wstring classTitle, HICON icon);
        ~SubComponent();

    public:
        
        virtual void RegisterComponentClass() = 0;
        virtual void Initialize() = 0;
        HWND GetHandle();

    protected:

        void SetHandle(HWND hHWnd);
        std::wstring& GetClass();
        std::wstring& GetTitle();
        HICON GetIcon();

        static  LRESULT CALLBACK SetupMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static  LRESULT AssignMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        
        virtual LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    protected:

        HWND m_pHWnd;

    
    private:
        
        std::wstring m_pszClass;
        std::wstring m_pszTitle;
        HICON m_hIcon;
        


    };

} // Rxn::Platform::Win32