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
#define RXN_CONTEXT Rxn::Engine::EngineContext

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
#include <string>
using String = std::string;
using WString = std::wstring;

namespace Rxn::Constants
{

    const unsigned short MAX_NAME_STRING_LENGTH = 256;
    const unsigned short MAX_DIR_LENGTH = 1024;
    const unsigned short MAX_LOG_LINE_LENGTH = 4096;


    namespace Win32
    {
        const unsigned short DEFAULT_WINDOW_WIDTH = 800;
        const unsigned short DEFAULT_WINDOW_HEIGHT = 600;

        const WString RENDER_VIEW_WINDOW_KEY = L"_SIMULATEDVIEW";
        const WString SPLASH_SCREEN_WINDOW_KEY = L"_SPLASH";
    }

    namespace Graphics
    {
        const unsigned short SLIDE_COUNT = 2;
    }

}

#pragma endregion // RxnConstants/TypeDefs
/* -------------------------------------------------------- */


