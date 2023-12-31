/*****************************************************************//**
 * \file   Pipeline.h
 * \brief  
 * 
 * \author Kalev Gonvick
 * \date   December 2023
 *********************************************************************/
#pragma once

#include "MemoryMappedPipelineStateObjectCache.h"
#include "PipelineDescriptor.h"
#include "../Content/Shaders/SimpleVertexShader.hlsl.h"
#include "../Content/Shaders/SimplePixelShader.hlsl.h"
#include "../Content/Shaders/QuadVertexShader.hlsl.h"
#include "../Content/Shaders/InvertPixelShader.hlsl.h"
#include "../Content/Shaders/BlitPixelShader.hlsl.h"
#include "../Content/Shaders/GrayScalePixelShader.hlsl.h"
#include "../Content/Shaders/EdgeDetectPixelShader.hlsl.h"
#include "../Content/Shaders/BlurPixelShader.hlsl.h"
#include "../Content/Shaders/WarpPixelShader.hlsl.h"
#include "../Content/Shaders/PixelatePixelShader.hlsl.h"
#include "../Content/Shaders/DistortPixelShader.hlsl.h"
#include "../Content/Shaders/WavePixelShader.hlsl.h"
#include "../Content/Shaders/UberPixelShader.hlsl.h"
#include "../Content/Shaders/AdditionalPixelShader.hlsl.h"

namespace Rxn::Graphics
{
    enum class PSOCachingMechanism : uint32
    {
        CachedBlobs = 1,
        PipelineLibraries = 2,
        PSOCachingMechanismCount = 3
    };

    inline const std::vector<D3D12_INPUT_ELEMENT_DESC>  g_SimpleInputElementDescs = {
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        }
    };

    inline const std::vector<D3D12_INPUT_ELEMENT_DESC>  g_QuadInputElementDescs = {
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        }
    };

    static const D3D12_INPUT_LAYOUT_DESC g_cForwardRenderInputLayout = { g_SimpleInputElementDescs.data() , static_cast<uint32>(g_SimpleInputElementDescs.size()) };
    static const D3D12_INPUT_LAYOUT_DESC g_cQuadInputLayout = { g_QuadInputElementDescs.data(), static_cast<uint32>(g_QuadInputElementDescs.size()) };

    class RXN_ENGINE_API Pipeline
    {
    public:

        Pipeline(const String &effectName, const GraphicsShaderSet &graphicsShaderSet, bool baseEffect, bool diskCache);
        Pipeline(const String &effectName, const String &fileName, const GraphicsShaderSet &graphicsShaderSet, bool baseEffect, bool diskCache);
        ~Pipeline();

    public:

        const String effectName;
        const String effectFileName;
        const GraphicsShaderSet graphicsShaderSet;
        const bool baseEffect;
        const bool useDiskCache;

        Mapped::MemoryMappedPipelineStateObjectCache diskCache;
        GraphcisPipelineDescriptor pipelineDescriptor{};

        ID3D12PipelineState *pipelineState = nullptr;
        ID3D12PipelineLibrary *pipelineLibrary = nullptr;
        std::mutex *pipelineMutex = nullptr;
        PSOCachingMechanism pipelineStateObjectCachingMechanism = PSOCachingMechanism::PipelineLibraries;
        ID3D12Device8 *device = nullptr;
        ID3D12RootSignature *rootSignature = nullptr;
        HANDLE threadHandle = INVALID_HANDLE_VALUE;

        bool compileFlag = false;
        bool flightFlag = false;
    };

    inline const Pipeline g_Normal3D{
        "Normal 3D",
        "normal3dPSO.cache",
        {
            g_cForwardRenderInputLayout,
            CD3DX12_SHADER_BYTECODE(g_SimpleVertexShader, sizeof(g_SimpleVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_SimplePixelShader, sizeof(g_SimplePixelShader)),
            {},
            {},
            {},
        },
        true,
        false
    };

    inline const Pipeline g_GenericPostEffect{
        "Generic post effect",
        "ubershaderPSO.cache",
        {
             g_cQuadInputLayout,
             CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
             CD3DX12_SHADER_BYTECODE(g_UberPixelShader, sizeof(g_UberPixelShader)),
             {},
             {},
             {},
        },
        true,
        false
    };

    inline const Pipeline g_Blit{
        "Blit",
        "blitEffectPSO.cache",
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_BlitPixelShader, sizeof(g_BlitPixelShader)),
            {},
            {},
            {},
        },
        true,
        false
    };

    inline const Pipeline g_Invert{
        "Invert",
        "invertEffectPSO.cache",
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_InvertPixelShader, sizeof(g_InvertPixelShader)),
            {},
            {},
            {},
        },
        true,
        false
    };

    inline const Pipeline g_Grayscale{
        "Gray scale",
        "grayscaleEffectPSO.cache",
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_GrayScalePixelShader, sizeof(g_GrayScalePixelShader)),
            {},
            {},
            {},
        },
        true,
        false
    };

    inline const Pipeline g_EdgeDetect{
        "Edge detect",
        "edgeDetectEffectPSO.cache",
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_EdgeDetectPixelShader, sizeof(g_EdgeDetectPixelShader)),
            {},
            {},
            {},
        },
        true,
        false
    };

    inline const Pipeline g_Blur{
        "Blur",
        "blurEffectPSO.cache",
        {
             g_cQuadInputLayout,
             CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
             CD3DX12_SHADER_BYTECODE(g_BlurPixelShader, sizeof(g_BlurPixelShader)),
             {},
             {},
             {},
        },
        true,
        false
    };

    inline const Pipeline g_Warp{
        "Warp",
        "warpEffectPSO.cache",
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_WarpPixelShader, sizeof(g_WarpPixelShader)),
            {},
            {},
            {},
        },
        true,
        false
    };

    inline const Pipeline g_Pixelate{
        "Pixelate",
        "pixelateEffectPSO.cache",
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_PixelatePixelShader, sizeof(g_PixelatePixelShader)),
            {},
            {},
            {},
        },
        true,
        false
    };

    inline const Pipeline g_Distort{
        "Distort",
        "distortEffectPSO.cache",
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_DistortPixelShader, sizeof(g_DistortPixelShader)),
            {},
            {},
            {},
        },
        true,
        false
    };

    inline const Pipeline g_Wave{
        "Wave",
        "waveEffectPSO.cache",
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_WavePixelShader, sizeof(g_WavePixelShader)),
            {},
            {},
            {},
        },
        true,
        false
    };

    inline const Pipeline g_Additional{
        "Additional",
        "additionalPixelShaderPSO.cache",
        {
             g_cQuadInputLayout,
             CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
             CD3DX12_SHADER_BYTECODE(g_AdditionalPixelShader, sizeof(g_AdditionalPixelShader)),
             {},
             {},
             {},
        },
        true,
        false
    };
}