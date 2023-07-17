/*****************************************************************//**
 * \file   Window.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once
#include "SubComponent.h"
#include "Caption.h"
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

        Window(WString className, WString title);
        ~Window();

    public:

        WString m_TitleName;
        Caption m_WindowCaption;
        SIZE m_Size;
        DWORD m_WindowStyle;

        COLORREF m_WindowBackgroundColour;
        COLORREF m_WindowBorderColour;
        COLORREF m_WindowActiveBorderHighlightColour;
        COLORREF m_WindowTitleActiveTextColour;
        COLORREF m_WindowTitleInactiveTextColour;

        bool m_AddCloseButton;
        bool m_AddMinimizeButton;
        bool m_AddMaximizeButton;

        int m_Active;

        /**
         * .
         */
        virtual void SetupWindow() {};

        /**
         * .
         *
         */
        virtual void RegisterComponentClass() override;

        /**
         * .
         *
         */
        virtual void InitializeWin32() override;

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
        void PaintWindowBorder(const HDC &hdc, const RECT &rect);

        /**
         * Paints the conditional border colour of the window if it is in active state.
         *
         * \param hdc               - hdc handle.
         * \param rect              - rect containing window size information.
         */
        void PaintWindowConditionalHighlight(const HDC &hdc, const RECT &rect);

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
        void PaintWindowCaptionTitle(const HDC &hdc, const SIZE &size);

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
        void ModifyClassStyle(const HWND &hWnd, const DWORD &flagsToDisable, const DWORD &flagsToEnable);

        /**
         * Maximizes the window.
         *
         * \param hwnd              - window handle
         */
        void MaximizeWindow(const HWND &hwnd);


    };

} // Rxn::Platform::Win32