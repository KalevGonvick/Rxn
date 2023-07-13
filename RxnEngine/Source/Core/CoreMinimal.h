#pragma once

/* -------------------------------------------------------- */
/*  Definitions                                             */
/* -------------------------------------------------------- */
#pragma region Definitions

#include "CoreDefinitions.h"
#include "Core/Math/Math.h"

#pragma endregion
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/*  Singleton                                               */
/* -------------------------------------------------------- */
#pragma region Singletons

#include "Common/Logger.h"
#include "Common/Time.h"
#include "Core/RxnBinaryHandler.h"

#pragma endregion
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/*  Engine                                                  */
/* -------------------------------------------------------- */
#pragma region Engine

#include "Engine/RxnEngine.h"
#include "Engine/EngineSettings.h"
#include "Engine/EngineContext.h"

#pragma endregion
/* -------------------------------------------------------- */

#pragma region Graphics

#include "Graphics/RenderCore.h"

#pragma endregion

/* -------------------------------------------------------- */
/*  Windows Platform                                        */
/* -------------------------------------------------------- */
#pragma region Windows

#ifdef WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "Platform/WIN32/SubComponent.h"
#include "Platform/WIN32/Window.h"
#include "Platform/WIN32/IApplication.h"

#include <wrl.h>

#endif

#pragma endregion
/* -------------------------------------------------------- */

