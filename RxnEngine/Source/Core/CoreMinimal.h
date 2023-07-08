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


/* -------------------------------------------------------- */
/*  Windows Platform                                        */
/* -------------------------------------------------------- */
#pragma region Windows

#ifdef WIN32

#define NOMINMAX

#include <d3d12.h>
#include <dxgi1_6.h>

#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

#include "Platform/WIN32/SubComponent.h"
#include "Platform/WIN32/Window.h"
#include "Platform/WIN32/IApplication.h"

#endif

#pragma endregion
/* -------------------------------------------------------- */

