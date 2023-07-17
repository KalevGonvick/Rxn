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
#include "Core/StepTimer.h"
#include "Engine/Camera.h"
#include "Renderable.h"
#include "Shape.h"
#include "Quad.h"

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



    class RXN_ENGINE_API RenderFramework
    {
    public:

        RenderFramework(int width, int height);
        ~RenderFramework();

        void DX12_Destroy();
        void DX12_Render();
        void DX12_Update();

    protected:

        inline float GetRandomColour()
        {
            return (rand() % 100) / 100.0f;
        }

        inline std::wstring GetAssetFullPath(LPCWSTR assetName)
        {
            return m_AssetsPath + assetName;
        }

        HRESULT DX12_LoadPipeline();
        HRESULT DX12_CreateFactory();

        HRESULT DX12_CreateCommantQueue();
        HRESULT DX12_CreateDescriptorHeaps();
        HRESULT DX12_CreateFrameResources();
        HRESULT DX12_CreateRootSignature();
        HRESULT DX12_LoadAssets();
        HRESULT DX12_CreateCommandList();
        HRESULT DX12_CreateVertexBufferResource();

        HRESULT DX12_CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap);

        std::vector<UINT8> GenerateTextureData();

        HRESULT DX12_CreateFrameSyncObjects();

        void DX12_MoveToNextFrame();
        void DX12_WaitForGPUFence();
        void DX12_ReleaseD3DResources();
        void DX12_RestoreD3DResources();

        void DX12_ToggleEffect(Mapped::EffectPipelineType type);


        void DX12_PopulateCommandList();

        bool m_UseWarpDevice;
        bool m_HasTearingSupport;

        UINT m_DrawIndex;
        ComPointer<ID3D12Device> m_Device;
        ComPointer<IDXGIFactory4> m_Factory;
        ComPointer<ID3D12CommandQueue> m_CommandQueue;
        ComPointer<ID3D12GraphicsCommandList> m_CommandList;
        ComPointer<IDXGISwapChain4> m_SwapChain;


        // app resources
        //ComPointer<ID3D12Resource> m_VertexIndexBuffer;
        //D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

        ComPointer<ID3D12Resource> m_Texture;
        //std::list<std::shared_ptr<Basic::Renderable>> m_Renderables;
        Basic::Shape m_Shape;
        Basic::Quad m_Quad;

        /*D3D12_VERTEX_BUFFER_VIEW m_CubeVbv;
        D3D12_VERTEX_BUFFER_VIEW m_QuadVbv;
        D3D12_INDEX_BUFFER_VIEW m_CubeIbv;*/

        WString m_AssetsPath;

        CD3DX12_VIEWPORT m_Viewport;
        CD3DX12_RECT m_ScissorRect;
        DirectX::XMMATRIX m_ProjectionMatrix;

        Core::StepTimer m_Timer;
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
        bool m_IsRenderReady;


        ComPointer<ID3D12Resource> m_RenderTargets[Constants::Graphics::BUFFER_COUNT];
        ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        UINT m_RTVDescriptorSize;
        ComPointer<ID3D12RootSignature> m_RootSignature;

    private:

        ComPointer<ID3D12Resource> m_IntermediateRenderTarget;
        ComPointer<ID3D12CommandAllocator> m_CommandAllocators[Constants::Graphics::BUFFER_COUNT];

        ComPointer<ID3D12DescriptorHeap> m_SRVHeap;
        UINT m_SRVDescriptorSize;

        static const UINT MaxDrawsPerFrame = 256;
        static const UINT TextureWidth = 256;
        static const UINT TextureHeight = 256;
        static const UINT TextureBytesPerPixel = 4;

        _Use_decl_annotations_;
        void DX12_GetHardwareAdapter(_In_ IDXGIFactory1 *pFactory, _Outptr_result_maybenull_ IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter = false);
        void DX12_CheckTearingSupport();
        HRESULT DX12_CreateIndependantDevice();

    };
}
