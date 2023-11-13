/*****************************************************************//**
 * \file   Renderer.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once
#include "DynamicConstantBuffer.h"
#include "PipelineLibrary.h"
#include "Camera.h"
#include "Display.h"
#include "CommandQueueManager.h"
#include "CommandListManager.h"
#include "CommandAllocatorPool.h"
#include "SwapChain.h"
#include "Scene.h"
#include "Fence.h"

namespace Rxn::Graphics
{

    class RXN_ENGINE_API Renderer
    {
    public:

        Renderer(int32 width, int32 height);
        ~Renderer();

    protected:

        inline float GetRandomColour()
        {
            return (rand() % 100) / 100.0f;
        }

        inline WString GetAssetFullPath(LPCWSTR assetName)
        {
            return m_AssetsPath + assetName;
        }

        virtual void InitializeRender() = 0;
        virtual void ShutdownRender() = 0;
        virtual void RenderPass() = 0;
        virtual void PreRenderPass() = 0;
        virtual void PostRenderPass() = 0;

        void CreateVertexBufferResource();
        void CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap);

        std::vector<uint8> GenerateTextureData() const;

        void ToggleEffect(Mapped::EffectPipelineType type);

        bool m_UseWarpDevice = false;
        bool m_HasTearingSupport = false;

        uint32 m_DrawIndex = 0;

        Manager::CommandQueueManager m_CommandQueueManager;
        Manager::CommandListManager m_CommandListManager;
        
        Pooled::CommandAllocatorPool m_AllocatorPool;
        ComPointer<ID3D12CommandAllocator> m_CommandAllocators[SwapChainBuffers::TOTAL_BUFFERS];
        
        Scene m_Scene;
        WString m_AssetsPath;
        Display m_Display;
        
        Mapped::PipelineLibrary m_PipelineLibrary;

        GPU::Fence m_Fence;

        bool m_Initialized;
        std::vector<bool> m_EnabledEffects;

    private:

        static const uint32 MaxDrawsPerFrame = 256;
        static const uint32 TextureWidth = 256;
        static const uint32 TextureHeight = 256;
        static const uint32 TextureBytesPerPixel = 4;

    };
}
