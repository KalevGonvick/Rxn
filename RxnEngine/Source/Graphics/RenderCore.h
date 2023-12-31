/*****************************************************************//**
 * \file   RenderCore.h
 * \brief
 *
 * \author Kalev Gonvick
 * \date   July 2023
 *********************************************************************/
#pragma once

 /* -------------------------------------------------------- */
 /*  GraphicsExternalLibs                                    */
 /* -------------------------------------------------------- */
#pragma region GraphicsExternalLibs

#include <d3d12.h>
#include <dxgi1_6.h>

#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"


#pragma endregion // GraphicsExternalLibs
/* -------------------------------------------------------- */


/* -------------------------------------------------------- */
/*  GraphicsHeaders                                         */
/* -------------------------------------------------------- */
#pragma region GraphicsHeaders

#include "GraphicsCommon.h"
#include "ComPointer.h"
#include "RenderContext.h"

#pragma endregion // GraphicsHeaders
/* -------------------------------------------------------- */




