/*****************************************************************//**
 * \file   SubComponent.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
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

        /**
         * .
         *
         */
        virtual void RegisterComponentClass() = 0;

        /**
         * .
         *
         */
        virtual void InitializeWin32() = 0;

        /**
         * .
         *
         * \return
         */
        const HWND GetParentHandle();

        /**
         * .
         *
         * \return
         */
        const std::shared_ptr<SubComponent> GetParent();

        /**
         * .
         *
         * \return
         */
        HICON GetIcon();

        /**
         * .
         *
         * \return
         */
        const WString GetClass();

        /**
         * .
         *
         * \param child
         */
        void AddChildComponent(std::shared_ptr<SubComponent> child);

        /**
         * .
         */
        bool m_IsInteractive;

        /**
         * .
         */
        HWND m_pHWnd;

    protected:

        /**
         * .
         *
         */
        void InitializeChildren();

        /**
         * .
         *
         * \param hWnd
         * \param msg
         * \param wParam
         * \param lParam
         * \return
         */
        static  LRESULT CALLBACK SetupMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        /**
         * .
         *
         * \param hWnd
         * \param msg
         * \param wParam
         * \param lParam
         * \return
         */
        static  LRESULT AssignMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        /**
         * .
         *
         * \param hWnd
         * \param msg
         * \param wParam
         * \param lParam
         * \return
         */
        virtual LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


        std::list<std::shared_ptr<SubComponent>> m_ChildComponents;
        std::shared_ptr<SubComponent> m_Parent;
        bool m_isInitialized;
        WString m_ClassName;
        HICON m_Icon;

    };


} // Rxn::Platform::Win32