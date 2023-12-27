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

        void InitializeRender();
        virtual void ShutdownRender() = 0;
        virtual void RenderPass() = 0;
        virtual void PreRenderPass() = 0;

        std::vector<uint8> GenerateTextureData() const;
        ComPointer<ID3D12RootSignature> &GetRootSignature();
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

        bool m_UseWarpDevice = false;
        bool m_HasTearingSupport = false;

    private:

        Manager::CommandQueueManager m_CommandQueueManager { RenderContext::GetGraphicsDevice() };
        Manager::CommandListManager m_CommandListManager { RenderContext::GetGraphicsDevice() };
        Mapped::PipelineLibrary m_PipelineLibrary { 2, 1 };
        Pooled::CommandAllocatorPool m_AllocatorPool { D3D12_COMMAND_LIST_TYPE_DIRECT };
        ComPointer<ID3D12CommandAllocator> m_CommandAllocators[2];
        ComPointer<ID3D12RootSignature> m_RootSignature;

        Scene m_Scene;
        WString m_AssetsPath;
        Display m_Display;

        GPU::Fence m_Fence;

        static const uint32 TextureWidth = 256;
        static const uint32 TextureHeight = 256;
        static const uint32 TextureBytesPerPixel = 4;

    };

    class SceneRenderContext
    {
    public:

        SceneRenderContext() = delete;
        explicit SceneRenderContext(Scene &scene, Display &display);
        ~SceneRenderContext();

    public:
        
        void FrameStart(ID3D12GraphicsCommandList6 *cmdList, ID3D12RootSignature *rootSignature);
        void ClearRtv(ID3D12GraphicsCommandList6 *cmdList, const CD3DX12_CPU_DESCRIPTOR_HANDLE &rtvHandle) const;
        void AddBarrier(uint32 barrierIndex, ID3D12Resource *pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
        void SetRootDescriptorTable(ID3D12GraphicsCommandList6 *cmdList, uint32 rootParameterIndex);
        void ExecuteBarrier(ID3D12GraphicsCommandList6 *cmdList, uint32 barrierIndex) const;
        void ExecuteBarriers(ID3D12GraphicsCommandList6 *cmdList) const;
        void SwapBarrier(uint32 barrierIndex);
        void FrameEnd(ComPointer<ID3D12GraphicsCommandList6> cmdList) const;

    private:

        Scene &m_Scene;
        Display &m_Display;
        std::vector<std::pair<uint32, D3D12_RESOURCE_BARRIER>> m_Barriers;
    };
}
