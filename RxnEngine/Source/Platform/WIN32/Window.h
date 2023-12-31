/*****************************************************************//**
 * \file   Window.h
 * \brief
 *
 * \author Kalev Gonvick
 * \date   July 2023
 *********************************************************************/
#pragma once

#include "Caption.h"
//#include <Uxtheme.h>

//#pragma comment(lib,"uxtheme.lib")

namespace Rxn::Platform::Win32
{
    enum class WindowStyle : uword
    {
        STATIC = WS_OVERLAPPED,
        RESIZEABLE = WS_SIZEBOX,
        POPUP = WS_POPUP
    };

    class RXN_ENGINE_API Window : public SubComponent
    {
    public:

        Window(const WString &className, const WString &title, int32 width, int32 height);
        ~Window();

    public:

        RECT GetWindowSize();

    public:

        WString m_TitleName;
        Caption m_WindowCaption{};
        SIZE m_Size;
        uword m_WindowStyle = static_cast<uword>(WindowStyle::STATIC);

        uword m_WindowBackgroundColour = RGB(36, 36, 36);
        uword m_WindowBorderColour = RGB(46, 46, 46);
        uword m_WindowActiveBorderHighlightColour = RGB(155, 80, 255);
        uword m_WindowTitleActiveTextColour = RGB(255, 255, 255);
        uword m_WindowTitleInactiveTextColour = RGB(92, 92, 92);

        bool m_AddCloseButton = false;
        bool m_AddMinimizeButton = false;
        bool m_AddMaximizeButton = false;

        int m_Active = 0;

        /**
         * .
         */
        virtual void SetupWindow() 
        {
            RXN_LOGGER::Warn(L"Default 'SetupWindow' being used for window '%s'...", m_TitleName.c_str());
        };

        /**
         * .
         *
         */
        void RegisterComponentClass() override;

        /**
         * .
         *
         */
        void InitializeWin32() override;

        /**
         * .
         *
         * \param hWnd
         * \param msg
         * \param wParam
         * \param lParam
         * \return
         */
        virtual LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    protected:

        /**
         * .
         *
         * \param key
         */
        virtual void HandleKeyDown(uint8 key) {};

        /**
         * .
         *
         * \param key
         */
        virtual void HandleKeyUp(uint8 key) {};

        /**
         * .
         *
         */
        void Redraw();

        /**
         * Handles double click event in the non-client area.
         *
         */
        void HandleNonClientAreaDoubleClick();

        /**
         * Handles non client paint messages.
         *
         * \param region            - region that requested painting.
         */
        void HandleNonClientPaint(const HRGN &region);

        /**
         * .
         *
         */
        void HandleNonClientLeftClickDown();

        /**
         * .
         *
         */
        void HandlePaint();

        /**
         * Paints the colour of the window border.
         *
         * \param hdc               - hdc handle.
         * \param rect              - rect containing window size information.
         */
        void PaintWindowBorder(const HDC &hdc, const RECT &rect) const;

        /**
         * Paints the conditional border colour of the window if it is in active state.
         *
         * \param hdc               - hdc handle.
         * \param rect              - rect containing window size information.
         */
        void PaintWindowConditionalHighlight(const HDC &hdc, const RECT &rect) const;

        /**
         * Handles the non-client create message.
         *
         */
        void HandleNonClientCreate();

        /**
         * Handles the non-client activate message.
         *
         * \param active            - boolean if the window goes into active state or not.
         */
        void HandleNonClientActivate(const int &active);

        /**
         * Add bitmap image to the window.
         *
         * \param szFileName        - zero terminated string for filename
         * \param hWinDC            - hdc handle
         * \param x                 - image x pos
         * \param y                 - image y pos
         * \return                  - return 1 if success, 0 if failed.
         */
        int AddBitmap(const wchar_t *szFileName, const HDC &hWinDC, int x = 0, int y = 0);

        /**
         * Paints the window caption area (top bar icons, title, etc.).
         *
         * \param hdc               - hdc handle
         * \param size              - size of the window
         */
        void PaintWindowCaption(const HDC &hdc, const SIZE &size);

        /**
         * Paints the title in the non-client area.
         *
         * \param hdc               - hdc handle.
         * \param size              - size of the window.
         */
        void PaintWindowCaptionTitle(const HDC &hdc, const SIZE &size) const;

        /**
         * Paints the buttons in the non-client area.
         *
         * \param hdc               - hdc handle
         * \param size              - size of the window
         */
        void PaintWindowCaptionButtons(const HDC &hdc, const SIZE &size);

        /**
         * Modify the class style of the window.
         *
         * \param hWnd              - window handle.
         * \param flagsToDisable    - all flags you want to disable on the window class.
         * \param flagsToEnable     - all flags you want to enable on the window class.
         */
        void ModifyClassStyle(HWND hWnd, const uword flagsToDisable, const uword flagsToEnable) const;

        /**
         * Maximizes the window.
         *
         * \param hwnd              - window handle
         */
        void MaximizeWindow(HWND hwnd) const;

    };

} // Rxn::Platform::Win32