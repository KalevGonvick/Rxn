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

        HRESULT CreateRootSignature();
        HRESULT CreateVertexBufferResource();
        HRESULT CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap);

        std::vector<uint8> GenerateTextureData();

        void ToggleEffect(Mapped::EffectPipelineType type);

        bool m_UseWarpDevice;
        bool m_HasTearingSupport;

        uint32 m_DrawIndex;

        Manager::CommandQueueManager m_CommandQueueManager;
        Manager::CommandListManager m_CommandListManager;
        
        /*ComPointer<ID3D12Resource> m_Texture;
        Basic::Shape m_Shape;
        Basic::Quad m_Quad;*/
        Scene m_Scene;
        WString m_AssetsPath;
        Display m_Display;
        
        /*Camera m_Camera;
        Buffer::DynamicConstantBuffer m_DynamicConstantBuffer;*/
        Mapped::PipelineLibrary m_PipelineLibrary;

        GPU::Fence m_Fence;

        bool m_Initialized;
        bool m_EnabledEffects[Mapped::EffectPipelineTypeCount];

        //ComPointer<ID3D12Resource> m_RenderTargets[SwapChainBuffers::TOTAL_BUFFERS];
        //ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        //uint32 m_RTVDescriptorSize;

        //ComPointer<ID3D12RootSignature> m_RootSignature;
        //ComPointer<ID3D12Resource> m_IntermediateRenderTarget;
        //ComPointer<ID3D12DescriptorHeap> m_SRVHeap;

        ComPointer<ID3D12CommandAllocator> m_CommandAllocators[SwapChainBuffers::TOTAL_BUFFERS];
        //uint32 m_SRVDescriptorSize;

    private:

        static const uint32 MaxDrawsPerFrame = 256;
        static const uint32 TextureWidth = 256;
        static const uint32 TextureHeight = 256;
        static const uint32 TextureBytesPerPixel = 4;

    };
}
