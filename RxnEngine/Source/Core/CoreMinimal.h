#pragma once

/* -------------------------------------------------------- */
/*  Definitions                                             */
/* -------------------------------------------------------- */
#pragma region Definitions
#ifndef RXN_Definitions
#define RXN_Definitions

#include "CoreDefinitions.h"
#include "Core/Math/Math.h"

#endif
#pragma endregion
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/*  Singleton                                               */
/* -------------------------------------------------------- */
#pragma region Singletons
#ifndef RXN_Singletons
#define RXN_Singletons

#include "Common/Logger.h"
#include "Common/Time.h"
#include "Core/RxnBinaryHandler.h"

#endif // !RXN_Singletons
#pragma endregion
/* -------------------------------------------------------- */

/* -------------------------------------------------------- */
/*  Engine                                                  */
/* -------------------------------------------------------- */
#pragma region Engine
#ifndef RXN_Engine
#define RXN_Engine

#include "Engine/RxnEngine.h"
#include "Engine/EngineSettings.h"
#include "Engine/EngineContext.h"

#endif
#pragma endregion
/* -------------------------------------------------------- */

#pragma region Graphics
#ifndef RXN_Graphics
#define RXN_Graphics

#include "Graphics/RenderCore.h"

#endif // !RXN_Graphics
#pragma endregion

/* -------------------------------------------------------- */
/*  Windows Platform                                        */
/* -------------------------------------------------------- */
#pragma region Windows
#ifndef RXN_Windows
#define RXN_Windows

#ifdef WIN32

#include "Platform/WIN32/SubComponent.h"
#include "Platform/WIN32/Window.h"
#include "Platform/WIN32/IApplication.h"

#endif
#endif

#pragma endregion
/* -------------------------------------------------------- */

