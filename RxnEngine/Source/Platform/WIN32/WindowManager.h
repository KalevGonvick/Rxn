/*****************************************************************//**
 * \file   WindowManager.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once

namespace Rxn::Platform::Win32
{
    struct WindowDesc
    {
        WString windowName;
        WString windowClassName;
        COLORREF windowBgColour;
        COLORREF windowBorderColour;
        COLORREF windowActiveBorderHighlightColour;
        WindowStyle windowStyle;
        SIZE windowSize;

        WindowDesc(const WString &winName, const WString &winClassName)
            : windowName(winName)
            , windowClassName(winClassName)
            , windowBgColour(RGB(36, 36, 36))
            , windowBorderColour(RGB(46, 46, 46))
            , windowActiveBorderHighlightColour(RGB(155, 80, 255))
            , windowStyle(WindowStyle::RESIZEABLE)
            , windowSize(SIZE(800, 600))
        {
            if (winClassName == L"")
            {
                windowClassName = winName;
            }
        }
    };

    using ManagedWindows = std::unordered_map<WString, std::shared_ptr<Window>>;

    class RXN_ENGINE_API WindowManager
    {
    public:

        WindowManager();
        ~WindowManager();

    public:

        /**
         * .
         *
         * \param window
         */
        void CreateNewWindow(WindowDesc &windowDesc);

        /**
         * .
         *
         * \param window
         */
        void AddWindow(std::shared_ptr<Window> window);

        ManagedWindows m_ManagedWindows{};

    private:

        unsigned int m_WindowCounter = 0;

    };
}
