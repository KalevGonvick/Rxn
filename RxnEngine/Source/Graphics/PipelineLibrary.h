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
#include "Pipeline.h"
#include <array>

namespace Rxn::Graphics::Mapped
{
    struct PipelineLibraryException : std::runtime_error
    {
        explicit PipelineLibraryException(const String &msg) : std::runtime_error(msg) {};
    };

    // This will be used to tell the uber shader which effect to use.
    struct UberShaderConstantBuffer
    {
        uint32 effectIndex;
    };

    
    class RXN_ENGINE_API PipelineLibrary : NonCopyable
    {
    public:

        PipelineLibrary();
        ~PipelineLibrary();

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

        static void CompilePipelineStateObject(Pipeline *pPipelineEffect);
        static bool CompileCacheCheck(Pipeline *pPipelineEffect);
        static bool GetPipelineCompileFlagSafe(Pipeline *pPipelineEffect);
        static bool GetPipelineInFlightFlagSafe(Pipeline *pPipelineEffect);
        
        static void SetPipelineInFlightFlagSafe(Pipeline *pPipelineEffect, bool flag);
        static void SetPipelineCompileFlagSafe(Pipeline *pPipelineEffect, bool flag);

        /**
         * .
         * 
         */
        void WaitForThreads();

    private:
        const WString PIPELINE_LIBRARY_FILE_NAME = L"pipelineLibrary.cache";
        const uint32 MAX_DRAWS_PER_FRAME = 256;

        std::vector<Pipeline> m_Pipelines;
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
}

