#pragma once

namespace Rxn::Graphics
{
    class RenderFramework
    {
    public:

        RenderFramework(int width, int height);
        ~RenderFramework();

    protected:

        struct Vertex
        {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT2 uv;
        };

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
        HRESULT DX12_CreateVertexBufferResource();

        HRESULT DX12_CreateTextureUploadHeap(ComPointer<ID3D12Resource> &textureUploadHeap);

        std::vector<UINT8> GenerateTextureData();

        HRESULT DX12_CreateFrameSyncObjects();

        void DX12_MoveToNextFrame();
        void DX12_WaitForGPUFence();

        void DX12_CheckTearingSupport();
        void DX12_PopulateCommandList();

        bool m_UseWarpDevice;
        bool m_HasTearingSupport;

        ComPointer<ID3D12Device> m_Device;
        ComPointer<IDXGIFactory4> m_Factory;

        ComPointer<ID3D12CommandQueue> m_CommandQueue;

        ComPointer<ID3D12GraphicsCommandList> m_CommandList;
        ComPointer<IDXGISwapChain4> m_SwapChain;

        ComPointer<ID3D12RootSignature> m_RootSignature;
        ComPointer<ID3D12PipelineState> m_PipelineState;

        ComPointer<ID3D12Resource> m_RenderTargets[Constants::Graphics::SLIDE_COUNT];
        ComPointer<ID3D12CommandAllocator> m_CommandAllocators[Constants::Graphics::SLIDE_COUNT];

        ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        ComPointer<ID3D12DescriptorHeap> m_SRVHeap;
        UINT m_RTVDescriptorSize;

        // app resources
        ComPointer<ID3D12Resource> m_VertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
        ComPointer<ID3D12Resource> m_Texture;

        WString m_AssetsPath;

        CD3DX12_VIEWPORT m_Viewport;
        CD3DX12_RECT m_ScissorRect;

        UINT m_Width;
        UINT m_Height;

        float m_AspectRatio;

        UINT m_FrameIndex;
        HANDLE m_FenceEvent;

        ComPointer<ID3D12Fence> m_Fence;
        //UINT64 m_FenceValue;
        UINT64 m_FenceValues[Constants::Graphics::SLIDE_COUNT];

        bool m_IsRenderReady;

        static const UINT TextureWidth = 256;
        static const UINT TextureHeight = 256;
        static const UINT TexturePixelSize = 4;    // The number of bytes used to represent a pixel in the texture.

        _Use_decl_annotations_;
        void DX12_GetHardwareAdapter(_In_ IDXGIFactory1 *pFactory, _Outptr_result_maybenull_ IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter = false);



    };
}
