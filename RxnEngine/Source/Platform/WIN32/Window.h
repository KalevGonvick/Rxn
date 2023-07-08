#pragma once
#include "SubComponent.h"
#include <Uxtheme.h>
#pragma comment(lib,"uxtheme.lib")

namespace Rxn::Platform::Win32
{
    enum WindowStyle : DWORD
    {
        STATIC = WS_OVERLAPPED,
        RESIZEABLE = WS_SIZEBOX,
        POPUP = WS_POPUP
    };

    class RXN_ENGINE_API Window : public SubComponent
    {
    public:

        Window(std::wstring title, HICON icon);
        ~Window();

    public:

        DWORD GetWindowStyle();
        COLORREF GetWindowBackgroundColour();
        COLORREF GetWindowBorderColour();
        SIZE GetSize();

        void SetWindowStyle(const DWORD &style);
        void SetSize(const SIZE &size);
        void SetSize(const int &cx, const  int &cy);
        void SetVerticalSize(const int &cy);
        void SetHorizontalSize(const int &cx);
        void SetWindowBackgroundColour(const COLORREF &colour);
        void SetWindowBorderColour(const COLORREF &colour);

        virtual void RegisterComponentClass() override;
        virtual void Initialize() override;
        virtual LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    protected:

        void Redraw();

        void HandleNonClientPaint(const HRGN &region);
        void PaintWindowBorder(const HDC &hdc, const RECT &rect);
        void PaintWindowConditionalHighlight(const HDC &hdc, const RECT &rect);
        void HandleNonClientCreate();
        void HandleNonClientActivate(const int &active);

        void SetActive(const int &active);
        int GetActive();

        int AddBitmap(const wchar_t *szFileName, const HDC &hWinDC, int x = 0, int y = 0);
        void ModifyClassStyle(const HWND &hWnd, const DWORD &flagsToDisable, const DWORD &flagsToEnable);

        SIZE m_xySize;
        DWORD m_ulWindowStyle;
        COLORREF m_xyzulWindowBackgroundColour;
        COLORREF m_xyzulWindowBorderColour;
        int m_iActive;
    };

} // Rxn::Platform::Win32