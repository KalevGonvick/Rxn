#pragma once
#include "DynamicConstantBuffer.h"
#include "PipelineLibrary.h"
#include "Core/StepTimer.h"
#include "Engine/Camera.h"

namespace Rxn::Graphics
{

    enum RootParameters : UINT32
    {
        RootParameterUberShaderCB = 0,
        RootParameterCB,
        RootParameterSRV,
        RootParametersCount
    };

    struct VertexPositionUV
    {
        DirectX::XMFLOAT4 position;
        DirectX::XMFLOAT2 uv;
    };

    struct VertexPositionColour
    {
        DirectX::XMFLOAT4 position;
        DirectX::XMFLOAT3 colour;
    };

    struct DrawConstantBuffer
    {
        DirectX::XMMATRIX worldViewProjection;
    };

    class RenderFramework
    {
    public:

        RenderFramework(int width, int height);
        ~RenderFramework();

    protected:

        inline float GetRandomColour()
        {
            return (rand() % 100) / 100.0f;
        }

        inline std::wstring GetAssetFullPath(LPCWSTR assetName)
        {
            return m_AssetsPath + assetName;
        }

        void DX12_Destroy();
        void DX12_Render();

        HRESULT DX12_LoadPipeline();
        HRESULT DX12_CreateFactory();
        HRESULT DX12_CreateIndependantDevice();
        HRESULT DX12_CreateCommantQueue();
        HRESULT DX12_CreateDescriptorHeaps();
        HRESULT DX12_CreateFrameResources();
        HRESULT DX12_CreateRootSignature();
        HRESULT DX12_LoadAssets();
        HRESULT DX12_CreateNewPSO();
        HRESULT DX12_CreateCommandList();
        HRESULT DX12_CreateVertexBufferResource(ComPointer<ID3D12Resource> &vertexIndexBufferUpload);

        HRESULT DX12_CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap);

        std::vector<UINT8> GenerateTextureData();

        HRESULT DX12_CreateFrameSyncObjects();

        void DX12_MoveToNextFrame();
        void DX12_WaitForGPUFence();

        void DX12_CheckTearingSupport();
        void DX12_PopulateCommandList();

        bool m_UseWarpDevice;
        bool m_HasTearingSupport;

        UINT m_DrawIndex;

        ComPointer<ID3D12Device> m_Device;
        ComPointer<IDXGIFactory4> m_Factory;

        ComPointer<ID3D12CommandQueue> m_CommandQueue;

        ComPointer<ID3D12GraphicsCommandList> m_CommandList;
        ComPointer<IDXGISwapChain4> m_SwapChain;

        ComPointer<ID3D12RootSignature> m_RootSignature;
        ComPointer<ID3D12PipelineState> m_PipelineState;

        ComPointer<ID3D12Resource> m_RenderTargets[Constants::Graphics::SLIDE_COUNT];
        ComPointer<ID3D12Resource> m_IntermediateRenderTarget;
        ComPointer<ID3D12CommandAllocator> m_CommandAllocators[Constants::Graphics::SLIDE_COUNT];

        ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        ComPointer<ID3D12DescriptorHeap> m_SRVHeap;
        UINT m_RTVDescriptorSize;
        UINT m_SRVDescriptorSize;

        // app resources
        ComPointer<ID3D12Resource> m_VertexIndexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
        ComPointer<ID3D12Resource> m_Texture;
        D3D12_VERTEX_BUFFER_VIEW m_CubeVbv;
        D3D12_VERTEX_BUFFER_VIEW m_QuadVbv;
        D3D12_INDEX_BUFFER_VIEW m_CubeIbv;

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
        UINT64 m_FenceValues[Constants::Graphics::SLIDE_COUNT];
        bool m_IsRenderReady;

        static const UINT MaxDrawsPerFrame = 256;
        static const UINT TextureWidth = 256;
        static const UINT TextureHeight = 256;
        static const UINT TextureBytesPerPixel = 4;

        _Use_decl_annotations_;
        void DX12_GetHardwareAdapter(_In_ IDXGIFactory1 *pFactory, _Outptr_result_maybenull_ IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter = false);



    };
}
