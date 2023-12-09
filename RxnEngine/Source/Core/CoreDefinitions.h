#pragma once

#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable:4238) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4239) // A non-const reference may only be bound to an lvalue; assignment operator takes a reference to non-const
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

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
/*  WindowsDefinitions                                      */
/* -------------------------------------------------------- */
#pragma region WindowsDefinitions

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#ifndef WIN32_LEAN_AND_MEAN

#define WIN32_LEAN_AND_MEAN

#endif // WIN32_LEAN_AND_MEAN

#define HInstance() GetModuleHandle(NULL)

#define WM_OUTPUTMESSAGE (WM_USER + 0x0001)

#pragma endregion // WindowsDefinitions
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/*  ExternalLibs                                            */
/* -------------------------------------------------------- */
#pragma region ExternalLibs

#include <windowsx.h>
#include <Windows.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <list>
#include <queue>
#include <array>
#include <vector>
#include <wrl.h>
#include <shellapi.h>
#include <mcx.h>

#pragma endregion // ExternalLibs
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/*  RxnConstants/TypeDefs                                   */
/* -------------------------------------------------------- */
#pragma region RxnConstants/TypeDefs

/* Singleton Accessors */
#define RXN_LOGGER Rxn::Common::Logger
#define RXN_CONTEXT Rxn::Engine::EngineContext
#define RXN_TIME_UTIL Rxn::Common::Time

/* Strings */
using String = std::string;
using WString = std::wstring;

/* Numbers */
using int_word = long;
using uint_word = unsigned long;
using int64 = long long;
using int32 = int;
using int16 = short;
using int8 = signed char;

using int64_fast = long long;
using int32_fast = int;
using int16_fast = int;
using int8_fast = signed char;

using uint64 = unsigned long long;
using uint32 = unsigned int;
using uint16 = unsigned short;
using uint8 = unsigned char;

using uint64_fast = unsigned long long;
using uint32_fast = unsigned int;
using uint16_fast = unsigned int;
using uint8_fast = unsigned char;

using float128 = long double;
using float64 = double;
using float32 = float;

#ifndef DCX_USESTYLE

constexpr int DCX_USESTYLE = 0x00010000;

#endif

/* Vectors */
using Vector2 = struct
{
    int32 x;
    int32 y;
};

using Vector3 = struct
{
    int32 x;
    int32 y;
    int32 z;
};

using Vector = struct
{
    int32 x;
    int32 y;
    int32 z;
    int32 w;
};

using FVector2 = struct
{
    float32 x;
    float32 y;
};

using FVector3 = struct
{
    float32 x;
    float32 y;
    float32 z;
};

using FVector = struct
{
    float32 x;
    float32 y;
    float32 z;
    float32 w;
};

namespace Rxn::Constants
{

    const uint16 MAX_NAME_STRING_LENGTH = 256;
    const uint16 MAX_DIR_LENGTH = 1024;
    const uint16 MAX_LOG_LINE_LENGTH = 4096;


    namespace Win32
    {
        const uint16 DEFAULT_WINDOW_WIDTH = 800;
        const uint16 DEFAULT_WINDOW_HEIGHT = 600;

        const WString RENDER_VIEW_WINDOW_KEY = L"_SIMULATEDVIEW";
        const WString SPLASH_SCREEN_WINDOW_KEY = L"_SPLASH";
    }

}

#pragma endregion // RxnConstants/TypeDefs
/* -------------------------------------------------------- */


