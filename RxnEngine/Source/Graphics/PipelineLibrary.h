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

namespace Rxn::Graphics::Mapped
{
    enum PSOCachingMechanism
    {
        CachedBlobs,

        // Enables applications to explicitly group PSOs which are expected to share data. Recommended over Cached Blobs.
        PipelineLibraries,

        PSOCachingMechanismCount
    };

    enum EffectPipelineType : uint32
    {
        // These always get compiled at startup.
        BaseNormal3DRender,
        BaseUberShader,

        // These are compiled a la carte.
        PostBlit,
        PostInvert,
        PostGrayScale,
        PostEdgeDetect,
        PostBlur,
        PostWarp,
        PostPixelate,
        PostDistort,
        PostWave,
        PostAdditional,
        EffectPipelineTypeCount
    };

    struct GraphicsShaderSet
    {
        D3D12_INPUT_LAYOUT_DESC inputLayout;
        D3D12_SHADER_BYTECODE VS;
        D3D12_SHADER_BYTECODE PS;
        D3D12_SHADER_BYTECODE DS;
        D3D12_SHADER_BYTECODE HS;
        D3D12_SHADER_BYTECODE GS;
    };

    static const D3D12_INPUT_ELEMENT_DESC g_cSimpleInputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    static const D3D12_INPUT_ELEMENT_DESC g_cQuadInputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    static const D3D12_INPUT_LAYOUT_DESC g_cForwardRenderInputLayout = { g_cSimpleInputElementDescs, _countof(g_cSimpleInputElementDescs) };
    static const D3D12_INPUT_LAYOUT_DESC g_cQuadInputLayout = { g_cQuadInputElementDescs, _countof(g_cQuadInputElementDescs) };

    static const GraphicsShaderSet g_cEffectShaderData[EffectPipelineTypeCount] =
    {
        {
            g_cForwardRenderInputLayout,
            CD3DX12_SHADER_BYTECODE(g_SimpleVertexShader, sizeof(g_SimpleVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_SimplePixelShader, sizeof(g_SimplePixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_UberPixelShader, sizeof(g_UberPixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_BlitPixelShader, sizeof(g_BlitPixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_InvertPixelShader, sizeof(g_InvertPixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_GrayScalePixelShader, sizeof(g_GrayScalePixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_EdgeDetectPixelShader, sizeof(g_EdgeDetectPixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_BlurPixelShader, sizeof(g_BlurPixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_WarpPixelShader, sizeof(g_WarpPixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_PixelatePixelShader, sizeof(g_PixelatePixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_DistortPixelShader, sizeof(g_DistortPixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_WavePixelShader, sizeof(g_WavePixelShader)),
            {},
            {},
            {},
        },
        {
            g_cQuadInputLayout,
            CD3DX12_SHADER_BYTECODE(g_QuadVertexShader, sizeof(g_QuadVertexShader)),
            CD3DX12_SHADER_BYTECODE(g_AdditionalPixelShader, sizeof(g_AdditionalPixelShader)),
            {},
            {},
            {},
        },
    };

    static const wchar_t *g_cPipelineLibraryFileName = L"pipelineLibrary.cache";

    static const wchar_t *g_cCacheFileNames[EffectPipelineTypeCount] =
    {
        L"normal3dPSO.cache",
        L"ubershaderPSO.cache",
        L"blitEffectPSO.cache",
        L"invertEffectPSO.cache",
        L"grayscaleEffectPSO.cache",
        L"edgeDetectEffectPSO.cache",
        L"blurEffectPSO.cache",
        L"warpEffectPSO.cache",
        L"pixelateEffectPSO.cache",
        L"distortEffectPSO.cache",
        L"waveEffectPSO.cache",
        L"additionalPixelShaderPSO.cache"
    };

    static const wchar_t *g_cEffectNames[EffectPipelineTypeCount] =
    {
        L"Normal 3D",
        L"Generic post effect",
        L"Blit",
        L"Invert",
        L"Grayscale",
        L"Edge detect",
        L"Blur",
        L"Warp",
        L"Pixelate",
        L"Distort",
        L"Wave",
        L"Additional"
    };

    class RXN_ENGINE_API PipelineLibrary
    {
    public:

        PipelineLibrary(uint32 frameCount, uint32 cbvRootSignatureIndex);
        ~PipelineLibrary();

    public:

        void Build(ID3D12Device *pDevice, ID3D12RootSignature *pRootSignature);
        void SetPipelineState(ID3D12RootSignature *pRootSignature, ID3D12GraphicsCommandList *pCommandList, _In_range_(0, EffectPipelineTypeCount - 1) EffectPipelineType type, uint32 frameIndex);

        void EndFrame();
        void ClearPSOCache();
        void ToggleUberShader();
        void ToggleDiskLibrary();
        void SwitchPSOCachingMechanism();
        void DestroyShader(EffectPipelineType type);

        bool UberShadersEnabled() const;
        bool DiskCacheEnabled() const;
        PSOCachingMechanism GetPSOCachingMechanism() const;

    private:

        struct CompilePipelineStateObjectThreadData
        {
            PipelineLibrary *pLibrary;
            ID3D12Device *pDevice;
            ID3D12RootSignature *pRootSignature;
            EffectPipelineType type;

            HANDLE threadHandle;
        };

        // This will be used to tell the uber shader which effect to use.
        struct UberShaderConstantBuffer
        {
            uint32 effectIndex;
        };

    private:

        static void CompilePipelineStateObject(CompilePipelineStateObjectThreadData *pDataPackage);
        static bool CompileCacheCheck(CompilePipelineStateObjectThreadData *pDataPackage);
        static void SetThreadCompileFinishFlags(CompilePipelineStateObjectThreadData *pDataPackage);
        void WaitForThreads();

    private:

        

        ComPointer<ID3D12PipelineState> m_PipelineStates[EffectPipelineTypeCount];

        MemoryMappedPipelineStateObjectCache m_DiskCaches[EffectPipelineTypeCount];     // Cached blobs.
        MemoryMappedPipelineLibrary m_PipelineLibrary;                                  // Pipeline Library.

        //HANDLE m_FlagsMutex;
        std::mutex m_FlagsMutex;

        CompilePipelineStateObjectThreadData m_WorkerThreads[EffectPipelineTypeCount]{};

        bool m_UseUberShaders = true;
        bool m_UseDiskLibraries = true;
        bool m_PipelineLibrariesSupported;
        bool m_CompiledPipelineStateObjectFlags[EffectPipelineTypeCount]{};
        bool m_InflightPipelineStateObjectFlags[EffectPipelineTypeCount]{};

        PSOCachingMechanism m_PipelineStateObjectCachingMechanism = PSOCachingMechanism::PipelineLibraries;

        WString m_CachePath;
        static const uint32 BaseEffectCount = 2;
        uint32 m_CBVRootSignatureIndex = 0;
        uint32 m_MaxDrawsPerFrame = 256;
        uint32 m_DrawIndex = 0;

        Buffer::DynamicConstantBuffer m_DynamicConstantBuffer;
    };
}

