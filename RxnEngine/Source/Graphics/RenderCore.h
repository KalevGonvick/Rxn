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

#include <d3d12.h>
#include <dxgi1_6.h>

#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

#include <D3Dcompiler.h>
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
#include "Renderer.h"

#pragma endregion // GraphicsHeaders
/* -------------------------------------------------------- */




