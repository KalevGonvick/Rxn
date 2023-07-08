#pragma once

/* -------------------------------------------------------- */
/*  DLL                                                     */
/* -------------------------------------------------------- */
#pragma region DLL

#ifdef BUILD_DLL
#define RXN_ENGINE_API __declspec(dllexport)
#else
#define RXN_ENGINE_API __declspec(dllimport)
#endif

#pragma endregion // DLL
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/*  CppDefines                                              */
/* -------------------------------------------------------- */
#pragma region CppDefines

#define RXN_LOGGER Rxn::Common::Logger

#pragma endregion // CppDefines
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/*  WindowsDefinitions                                      */
/* -------------------------------------------------------- */
#pragma region WindowsDefinitions

#define HInstance() GetModuleHandle(NULL)
#define WM_OUTPUTMESSAGE (WM_USER + 0x0001)

#ifndef DCX_USESTYLE
#define DCX_USESTYLE 0x00010000
#endif

#pragma endregion // WindowsDefinitions
/* -------------------------------------------------------- */


/* -------------------------------------------------------- */
/*  RxnConstants/TypeDefs                                   */
/* -------------------------------------------------------- */
#pragma region RxnConstants/TypeDefs

using String = std::string;
using WString = std::wstring;

namespace Rxn::Constants
{


    const unsigned short kusMaxNameString = 256;
    const unsigned short knMaxDirLen = 1024;
    const unsigned short knMaxLogLine = 4096;


    namespace Win32
    {
        const unsigned long kulDefaultWindowWidth = 800;
        const unsigned long kulDefaultWindowHeight = 600;
    }

}

#pragma endregion // RxnConstants/TypeDefs
/* -------------------------------------------------------- */


