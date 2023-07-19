/*****************************************************************//**
 * \file   RenderFramework.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
#pragma once
#include "DynamicConstantBuffer.h"
#include "PipelineLibrary.h"
#include "Engine/Camera.h"
#include "Renderable.h"
#include "Shape.h"
#include "Quad.h"
#include "CommandQueueManager.h"

namespace Rxn::Graphics
{
    enum RootParameters : UINT32
    {
        RootParameterUberShaderCB = 0,
        RootParameterCB,
        RootParameterSRV,
        RootParametersCount
    };

    struct DrawConstantBuffer
    {
        DirectX::XMMATRIX worldViewProjection;
    };

    struct Resolution
    {
        uint32 Width;
        uint32 Height;
    };

    class RXN_ENGINE_API Renderer
    {
    public:

        Renderer(int width, int height);
        ~Renderer();

    protected:

        inline float GetRandomColour()
        {
            return (rand() % 100) / 100.0f;
        }

        inline std::wstring GetAssetFullPath(LPCWSTR assetName)
        {
            return m_AssetsPath + assetName;
        }

        virtual void InitializeRender() = 0;
        virtual void ShutdownRender() = 0;
        virtual void RenderPass() = 0;
        virtual void PreRenderPass() = 0;
        virtual void PostRenderPass() = 0;

        HRESULT CreateDescriptorHeaps();
        HRESULT CreateCommandAllocators();
        HRESULT CreateRootSignature();
        HRESULT CreateCommandList();
        HRESULT CreateVertexBufferResource();
        HRESULT CreatePipelineSwapChain();
        HRESULT CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap);
        HRESULT CreateFrameSyncObjects();
        HRESULT MoveToNextFrame();


        std::vector<UINT8> GenerateTextureData();
        void WaitForBufferedFence();
        void WaitForSingleFrame();
        void ToggleEffect(Mapped::EffectPipelineType type);


        void PopulateCommandList();

        bool m_UseWarpDevice;
        bool m_HasTearingSupport;

        UINT m_DrawIndex;

        CommandQueueManager m_CommandQueueManager;
        ComPointer<ID3D12GraphicsCommandList> m_CommandList;
        ComPointer<IDXGISwapChain4> m_SwapChain;

        ComPointer<ID3D12Resource> m_Texture;
        //std::list<std::shared_ptr<Basic::Renderable>> m_Renderables;
        Basic::Shape m_Shape;
        Basic::Quad m_Quad;

        WString m_AssetsPath;

        CD3DX12_VIEWPORT m_Viewport;
        CD3DX12_RECT m_ScissorRect;
        DirectX::XMMATRIX m_ProjectionMatrix;

        Engine::Camera m_Camera;

        UINT m_Width;
        UINT m_Height;

        float m_AspectRatio;

        Buffer::DynamicConstantBuffer m_DynamicConstantBuffer;
        Mapped::PipelineLibrary m_PipelineLibrary;
        bool m_EnabledEffects[Mapped::EffectPipelineTypeCount];

        UINT m_FrameIndex;
        HANDLE m_FenceEvent;
        HANDLE m_SwapChainEvent;
        ComPointer<ID3D12Fence> m_Fence;
        UINT64 m_FenceValues[Constants::Graphics::BUFFER_COUNT];

        bool m_Initialized;


        ComPointer<ID3D12Resource> m_RenderTargets[Constants::Graphics::BUFFER_COUNT];
        ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        UINT m_RTVDescriptorSize;
        ComPointer<ID3D12RootSignature> m_RootSignature;
        ComPointer<ID3D12Resource> m_IntermediateRenderTarget;
        ComPointer<ID3D12DescriptorHeap> m_SRVHeap;

        std::array<Resolution, 2> const m_Resolutions = { { { 1280, 720 }, { 1920, 1080 } } };
        ComPointer<ID3D12CommandAllocator> m_CommandAllocators[Constants::Graphics::BUFFER_COUNT];
        UINT m_SRVDescriptorSize;
    private:

        static const UINT MaxDrawsPerFrame = 256;
        static const UINT TextureWidth = 256;
        static const UINT TextureHeight = 256;
        static const UINT TextureBytesPerPixel = 4;

    };
}
