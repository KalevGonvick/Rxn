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

        void CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap);
        void CreateAllocatorPool();
        
    public:

        uint32 m_DrawIndex = 0;
        bool m_UseWarpDevice = false;
        bool m_HasTearingSupport = false;

    private:

        Manager::CommandQueueManager m_CommandQueueManager { RenderContext::GetGraphicsDevice() };
        Manager::CommandListManager m_CommandListManager { RenderContext::GetGraphicsDevice() };
        Mapped::PipelineLibrary m_PipelineLibrary { static_cast<uint32>(SwapChainBuffers::TOTAL_BUFFERS), RootParameterCB };
        Pooled::CommandAllocatorPool m_AllocatorPool { D3D12_COMMAND_LIST_TYPE_DIRECT };
        ComPointer<ID3D12CommandAllocator> m_CommandAllocators[static_cast<uint32>(SwapChainBuffers::TOTAL_BUFFERS)];

        Scene m_Scene;
        WString m_AssetsPath;
        Display m_Display;

        GPU::Fence m_Fence;

        static const uint32 TextureWidth = 256;
        static const uint32 TextureHeight = 256;
        static const uint32 TextureBytesPerPixel = 4;

    };

    class RenderPassObjects
    {
    public:

        RenderPassObjects() = delete;
        explicit RenderPassObjects(Renderer *renderer);
        ~RenderPassObjects();

    public:
        
        RenderPassObjects &PopulateNewList(ComPointer<ID3D12GraphicsCommandList> cmdList);

    private:

        int8 m_RenderPassState = 0;
        
        const int8 INIT_STATE = 1 << 1;
        const int8 FINISH_STATE = 1 << 2;

        Renderer *m_Renderer;
        ComPointer<ID3D12GraphicsCommandList> m_ActiveCmdList = nullptr;

    };
}
