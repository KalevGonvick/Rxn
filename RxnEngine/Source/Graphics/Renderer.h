/*****************************************************************//**
 * \file   Renderer.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once
#include "DynamicConstantBuffer.h"
#include "Camera.h"
#include "Display.h"
#include "CommandQueuePool.h"
#include "CommandListPool.h"
#include "CommandAllocatorPool.h"
#include "Scene.h"

namespace Rxn::Graphics
{
    struct RendererException : std::runtime_error
    {
        explicit RendererException(const String &msg) : std::runtime_error(msg) {};
    };

    class RXN_ENGINE_API Renderer
    {
    public:

        Renderer(int32 width, int32 height);
        ~Renderer();

    protected:

        inline float32 GetRandomColour()
        {
            return (rand() % 100) / 100.0f;
        }

        void InitializeRender();
        virtual void RenderPass() = 0;

        Scene &GetScene();
        Display &GetDisplay();
        
        Pooled::CommandQueuePool &GetCommandQueuePool();
        Pooled::CommandListPool &GetCommandListPool();
        Pooled::CommandAllocatorPool &GetCommandAllocatorPool();
        
    public:

        bool m_UseWarpDevice = false;
        bool m_HasTearingSupport = false;

    private:

        Pooled::CommandQueuePool m_Cmd_Q_Pool { D3D12_COMMAND_LIST_TYPE_DIRECT };
        Pooled::CommandListPool m_Cmd_L_Pool { D3D12_COMMAND_LIST_TYPE_DIRECT };
        Pooled::CommandAllocatorPool m_Cmd_A_Pool { D3D12_COMMAND_LIST_TYPE_DIRECT };
        
        Scene m_Scene;
        Display m_Display;
    };

    class SceneRenderContext
    {
    public:

        SceneRenderContext() = delete;
        explicit SceneRenderContext(Scene &scene, Display &display);
        ~SceneRenderContext();

    public:
        
        void FrameStart(ID3D12GraphicsCommandList6 *cmdList);
        void ClearRtv(ID3D12GraphicsCommandList6 *cmdList, uint32 rtvIndex) const;
        void AddBarrier(uint32 barrierIndex, ID3D12Resource *pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
        void SetRootDescriptorTable(ID3D12GraphicsCommandList6 *cmdList, uint32 rootParameterIndex);
        void ExecuteBarrier(ID3D12GraphicsCommandList6 *cmdList, uint32 barrierIndex) const;
        void ExecuteBarriers(ID3D12GraphicsCommandList6 *cmdList) const;
        void SwapBarrier(uint32 barrierIndex);
        void SetPipelineState(ID3D12GraphicsCommandList6 *cmdList, uint32 pipelineIndex);
        void PresentFrame(ID3D12CommandQueue *pCmdQueue) const;
        
    private:

        Scene &m_Scene;
        Display &m_Display;
        std::vector<std::pair<uint32, D3D12_RESOURCE_BARRIER>> m_Barriers;
    };
}
