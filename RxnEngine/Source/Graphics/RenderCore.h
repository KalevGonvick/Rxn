/*****************************************************************//**
 * \file   RenderCore.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once

 /* -------------------------------------------------------- */
 /*  GraphicsExternalLibs                                    */
 /* -------------------------------------------------------- */
#pragma region GraphicsExternalLibs
#ifndef RXN_GraphicsExternalLibs
#define RXN_GraphicsExternalLibs

#include <d3d12.h>
#include <dxgi1_6.h>

#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"


#endif
#pragma endregion // GraphicsExternalLibs
/* -------------------------------------------------------- */


/* -------------------------------------------------------- */
/*  GraphicsHeaders                                         */
/* -------------------------------------------------------- */
#pragma region GraphicsHeaders
#ifndef RXN_GraphicsHeaders
#define RXN_GraphicsHeaders

#include "GraphicsCommon.h"
#include "ComPointer.h"
#include "RenderContext.h"

#endif
#pragma endregion // GraphicsHeaders
/* -------------------------------------------------------- */




