/*****************************************************************//**
 * \file   PipelineLibrary.h
 * \brief  
 * 
 * \author kalev
 * \date   October 2023
 *********************************************************************/
#pragma once
#include "DynamicConstantBuffer.h"
#include "MemoryMappedPipelineLibrary.h"
#include "MemoryMappedPipelineStateObjectCache.h"
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

#include <array>

namespace Rxn::Graphics::Mapped
{
    struct PipelineLibraryException : std::runtime_error
    {
        explicit PipelineLibraryException(const String &msg) : std::runtime_error(msg) {};
    };

    enum class PSOCachingMechanism : uint32
    {
        CachedBlobs                 = 1,
        PipelineLibraries           = 2,
        PSOCachingMechanismCount    = 3
    };

    struct GraphicsShaderSet
    {
        D3D12_INPUT_LAYOUT_DESC     inputLayout;
        D3D12_SHADER_BYTECODE       VS;
        D3D12_SHADER_BYTECODE       PS;
        D3D12_SHADER_BYTECODE       DS;
        D3D12_SHADER_BYTECODE       HS;
        D3D12_SHADER_BYTECODE       GS;
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

    // This will be used to tell the uber shader which effect to use.
    struct UberShaderConstantBuffer
    {
        uint32 effectIndex;
    };

    
    class PipelineEffectTemplate;
    class RXN_ENGINE_API PipelineLibrary : NonCopyable
    {
    public:

        PipelineLibrary();
        ~PipelineLibrary();

        friend class PipelineEffectTemplate;

    public:

        /**
         * .
         * 
         * \param pDevice
         * \param pRootSignature
         * \param swapChainBuffers
         */
        void Build(ID3D12Device8 *pDevice, ID3D12RootSignature *pRootSignature, uint32 cbvRootSignatureIndex, uint32 maxPipelineEffects, uint32 swapChainBuffers);

        /**
         * .
         * 
         * \param pRootSignature
         * \param pCommandList
         * \param type
         * \param frameIndex
         */
        void SetPipelineState(ID3D12RootSignature *pRootSignature, ID3D12GraphicsCommandList6 *pCommandList, uint32 pipelineIndex, uint32 frameIndex);
        
        /**
         * .
         * 
         */
        void ClearPSOCache();
        
        /**
         * .
         * 
         */
        void ToggleUberShader();

        /**
         * .
         * 
         */
        void ToggleDiskLibrary();
        
        /**
         * .
         * 
         */
        void SwitchPSOCachingMechanism();
        
        /**
         * .
         * 
         * \param type
         */
        void DestroyPipelineEffect(uint32 index);

        /**
         * .
         * 
         * \return 
         */
        bool UberShadersEnabled() const;

        /**
         * .
         * 
         * \return 
         */
        bool DiskCacheEnabled() const;

        /**
         * .
         * 
         * \return 
         */
        PSOCachingMechanism GetPSOCachingMechanism() const;

    private:

        /**
         * .
         *
         * \param pCommandList
         */
        void FallbackToCompiledPipeline(ID3D12GraphicsCommandList6 *pCommandList);

        static void CompilePipelineStateObject(PipelineEffectTemplate *pPipelineEffect);
        static bool CompileCacheCheck(PipelineEffectTemplate *pPipelineEffect);
        static bool GetPipelineCompileFlagSafe(PipelineEffectTemplate *pPipelineEffect);
        static bool GetPipelineInFlightFlagSafe(PipelineEffectTemplate *pPipelineEffect);
        
        static void SetPipelineInFlightFlagSafe(PipelineEffectTemplate *pPipelineEffect, bool flag);
        static void SetPipelineCompileFlagSafe(PipelineEffectTemplate *pPipelineEffect, bool flag);

        /**
         * .
         * 
         */
        void WaitForThreads();

    private:
        const WString PIPELINE_LIBRARY_FILE_NAME = L"pipelineLibrary.cache";
        const uint32 MAX_DRAWS_PER_FRAME = 256;

        std::vector<PipelineEffectTemplate> m_PipelineEffects;
        MemoryMappedPipelineLibrary m_PipelineLibrary;

        bool m_UseUberShaders = true;
        bool m_UseDiskLibraries = true;
        bool m_PipelineLibrariesSupported;

        PSOCachingMechanism m_PipelineStateObjectCachingMechanism = PSOCachingMechanism::PipelineLibraries;

        WString m_CachePath;
        uint32 m_CbvRootSignatureIndex = 0;
        std::mutex m_FlagsMutex;
        Buffer::DynamicConstantBuffer m_DynamicConstantBuffer;
    };

    class RXN_ENGINE_API PipelineEffectTemplate
    {
    public:
        
        PipelineEffectTemplate(const String &effectName, const GraphicsShaderSet &graphicsShaderSet, bool baseEffect, bool diskCache)
            : m_EffectName(effectName)
            , m_GraphicsShaderSet(graphicsShaderSet)
            , m_EffectFileName("pipelineLibrary.cache")
            , m_BaseEffect(baseEffect)
            , m_UseDiskCache(diskCache)
        {};

        PipelineEffectTemplate(const String &effectName, const String &fileName, const GraphicsShaderSet &graphicsShaderSet, bool baseEffect, bool diskCache)
            : m_EffectName(effectName)
            , m_EffectFileName(fileName)
            , m_GraphicsShaderSet(graphicsShaderSet)
            , m_BaseEffect(baseEffect)
            , m_UseDiskCache(diskCache)
        {};

        ~PipelineEffectTemplate() = default;

        // TODO - remove friend for codebase maintainability reasons...
        friend class PipelineLibrary;

    public:

        const String &GetEffectName()
        {
            return m_EffectName;
        }

        const String &GetEffectFileName()
        {
            return m_EffectFileName;
        }

        const GraphicsShaderSet &GetGraphicsShaderSet()
        {
            return m_GraphicsShaderSet;
        }

        bool IsBaseEffect()
        {
            return m_BaseEffect;
        }

        bool IsCompiled()
        {
            return compileFlag;
        }

        bool IsInFlight()
        {
            return flightFlag;
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC CreatePipelineStateDesc()
        {
            D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc = {};
            baseDesc.pRootSignature = rootSignature;
            baseDesc.SampleMask = UINT_MAX;
            baseDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            baseDesc.NumRenderTargets = 1;
            baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            baseDesc.SampleDesc.Count = 1;
            baseDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            baseDesc.DepthStencilState.DepthEnable = FALSE;
            baseDesc.DepthStencilState.StencilEnable = FALSE;


            baseDesc.InputLayout = GetGraphicsShaderSet().inputLayout;
            baseDesc.VS = GetGraphicsShaderSet().VS;
            baseDesc.PS = GetGraphicsShaderSet().PS;
            baseDesc.DS = GetGraphicsShaderSet().DS;
            baseDesc.HS = GetGraphicsShaderSet().HS;
            baseDesc.GS = GetGraphicsShaderSet().GS;
            return baseDesc;
        };



    private:

        const String m_EffectName;
        const String m_EffectFileName;
        const GraphicsShaderSet m_GraphicsShaderSet;
        const bool m_BaseEffect;
        const bool m_UseDiskCache;

        MemoryMappedPipelineStateObjectCache diskCache;
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


    inline const PipelineEffectTemplate g_Normal3D{
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

    inline const PipelineEffectTemplate g_GenericPostEffect{
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

    inline const PipelineEffectTemplate g_Blit{
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

    inline const PipelineEffectTemplate g_Invert{
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

    inline const PipelineEffectTemplate g_Grayscale{
        "Grayscale",
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

    inline const PipelineEffectTemplate g_EdgeDetect{
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

    inline const PipelineEffectTemplate g_Blur{
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

    inline const PipelineEffectTemplate g_Warp{
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

    inline const PipelineEffectTemplate g_Pixelate{
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

    inline const PipelineEffectTemplate g_Distort{
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

    inline const PipelineEffectTemplate g_Wave{
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

    inline const PipelineEffectTemplate g_Additional{
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

