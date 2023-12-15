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

        std::vector<uint8> GenerateTextureData() const;
        Scene &GetScene();
        GPU::Fence &GetFence();
        Display &GetDisplay();
        
        Manager::CommandQueueManager &GetCommandQueueManager();
        Manager::CommandListManager &GetCommandListManager();
        Mapped::PipelineLibrary &GetPipelineLibrary();

        ComPointer<ID3D12CommandAllocator> &GetCommandAllocator(const uint32 frameIndex);
        Pooled::CommandAllocatorPool &GetCommandAllocatorPool();
        
        const uint32 &GetDrawIndex() const;

        void CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap);
        void IncrementDrawIndex();
        void ResetDrawIndex();
        void CreateAllocatorPool();
        
        void InitScene();
        
        

    protected:

        static void InitRendererDisplay(Renderer &renderer, const String &cmdQueueHashKey);
        static void InitGpuFence(Renderer &renderer, const String &cmdQueueHashKey);
        static void InitCommandQueues(Renderer &renderer, const String &cmdQueueHashKey);
        static void InitCommandList(Renderer &renderer, const String &cmdQueueHashKey, const String &newCmdListHashKey);
        static void InitCachedPipeline(Renderer &renderer, ComPointer<ID3D12Device> device);
        static void InitCommandAllocator(Renderer &renderer, uint32 bufferIndex);

    private:

        bool m_UseWarpDevice = false;
        bool m_HasTearingSupport = false;

        uint32 m_DrawIndex = 0;

        Manager::CommandQueueManager m_CommandQueueManager { RenderContext::GetGraphicsDevice() };
        Manager::CommandListManager m_CommandListManager { RenderContext::GetGraphicsDevice() };
        Mapped::PipelineLibrary m_PipelineLibrary { SwapChainBuffers::TOTAL_BUFFERS, RootParameterCB };
        Pooled::CommandAllocatorPool m_AllocatorPool { D3D12_COMMAND_LIST_TYPE_DIRECT };
        ComPointer<ID3D12CommandAllocator> m_CommandAllocators[SwapChainBuffers::TOTAL_BUFFERS];

        Scene m_Scene;
        WString m_AssetsPath;
        Display m_Display;

        GPU::Fence m_Fence;

        static const uint32 MaxDrawsPerFrame = 256;
        static const uint32 TextureWidth = 256;
        static const uint32 TextureHeight = 256;
        static const uint32 TextureBytesPerPixel = 4;

    };
}
