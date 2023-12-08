#pragma once
#include "DynamicConstantBuffer.h"
#include "MemoryMappedPipelineLibrary.h"
#include "MemoryMappedPipelineStateObjectCache.h"
#include "PipelineLibrary.h"
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
    enum PSOCachingMechanism
    {
        CachedBlobs,

        // Enables applications to explicitly group PSOs which are expected to share data. Recommended over Cached Blobs.
        PipelineLibraries,

        PSOCachingMechanismCount
    };

    enum class EffectPipelineType
    {
        // These always get compiled at startup.
        BaseNormal3DRender = 0,
        BaseUberShader = 1,

        // These are compiled a la carte.
        PostBlit = 2,
        PostInvert = 3,
        PostGrayScale = 4,
        PostEdgeDetect = 5,
        PostBlur = 6,
        PostWarp = 7,
        PostPixelate = 8,
        PostDistort = 9,
        PostWave = 10,
        PostAdditional = 11,
        EffectPipelineTypeCount = 12
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

    static const GraphicsShaderSet g_cEffectShaderData[12] =
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

    const static wchar_t *g_cCacheFileNames[12] =
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

    static const wchar_t *g_cEffectNames[12] =
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


    


}

namespace Rxn::Graphics::Manager
{
    

    class RXN_ENGINE_API PipelineLibraryManager
    {
    public:
        friend class PipelineLibrary;
    public:

        PipelineLibraryManager(uint32 frameCount, uint32 cbvRootSignatureIndex);
        ~PipelineLibraryManager();

    public:

        void BuildCachedPipeline(const uint32 effectIndex);
        void EndFrame();
        void ClearPSOCache();
        void ToggleUberShader();
        void ToggleDiskLibrary();
        void SwitchPSOCachingMechanism();
        void DestroyPipeline(uint32 pipelineIndex);

        bool UberShadersEnabled() const;
        bool DiskCacheEnabled() const;
        PSOCachingMechanism GetPSOCachingMechanism() const;
        void SetPipelineState(ID3D12RootSignature *pRootSignature, ID3D12GraphicsCommandList *pCommandList, uint32 frameIndex, uint32 pipelineIndex);

        void Init(ComPointer<ID3D12Device> device);

        void BuildPipeline(ComPointer<ID3D12RootSignature> rootSignature, const uint32 effectIndex);
        PSOCachingMechanism GetPipelineCachingMechanism();
        
    private:

        // This will be used to tell the uber shader which effect to use.
        struct UberShaderConstantBuffer
        {
            uint32 effectIndex;
        };

    private:

        static void CompilePipelineStateObject(PipelineLibraryManager *pipelineManager, GPU::CompilePipelineStateObjectThreadData *pDataPackage);
        void WaitForThreads();

    private:

        ComPointer<ID3D12Device> m_Device;
        std::unordered_map<uint32, std::unique_ptr<GPU::PipelineLibrary>> m_PipelineLibraries;

        //ComPointer<ID3D12PipelineState> m_PipelineStates[12];

        //Mapped::MemoryMappedPipelineStateObjectCache m_DiskCaches[12];     // Cached blobs.
        
        Mapped::MemoryMappedPipelineLibrary m_PipelineLibrary;                                  // Pipeline Library.

        


        //CompilePipelineStateObjectThreadData m_WorkerThreads[12];
        std::mutex m_PipelineMutex;
        bool m_UseUberShaders = true;
        bool m_UseDiskLibraries = true;
        bool m_PipelineLibrariesSupported;
        //bool m_CompiledPipelineStateObjectFlags[12];
        //bool m_InflightPipelineStateObjectFlags[12];

        PSOCachingMechanism m_PipelineStateObjectCachingMechanism = PSOCachingMechanism::PipelineLibraries;

        WString m_CachePath;

        uint32 m_CBVRootSignatureIndex;
        uint32 m_DrawIndex = 0;

        Buffer::DynamicConstantBuffer m_DynamicConstantBuffer;
    };
}

