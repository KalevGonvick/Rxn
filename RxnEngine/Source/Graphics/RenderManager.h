#pragma once

namespace Rxn::Graphics
{
    class RXN_ENGINE_API RenderManager
    {
    public:

        RenderManager(int width, int height, WString windowClassName);
        ~RenderManager();

    public:

        /*void OnDestroy();
        void OnRender();
        void OnInit();*/
    private:

        //void LoadPipeline();
        //void LoadAssets();

        //_Use_decl_annotations_;
        //void GetHardwareAdapter(_In_ IDXGIFactory1 *pFactory, _Outptr_result_maybenull_ IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter = false);

        //void CheckTearingSupport();
        //void WaitForPreviousFrame();
        //void PopulateCommandList();

        //struct Vertex
        //{
        //    DirectX::XMFLOAT3 position;
        //    DirectX::XMFLOAT4 color;
        //};

        //inline std::wstring GetAssetFullPath(LPCWSTR assetName)
        //{
        //    return m_AssetsPath + assetName;
        //}

        //bool m_UseWarpDevice;
        //bool m_HasTearingSupport;

        ////HWND m_Hwnd;
        //WString m_WindowClassName;

        //ComPointer<ID3D12Device> m_Device;

        //ComPointer<ID3D12CommandQueue> m_CommandQueue;
        //ComPointer<ID3D12CommandAllocator> m_CommandAllocator;
        //ComPointer<ID3D12GraphicsCommandList> m_CommandList;
        //ComPointer<IDXGISwapChain4> m_SwapChain;

        //ComPointer<ID3D12RootSignature> m_RootSignature;
        //ComPointer<ID3D12PipelineState> m_PipelineState;

        //ComPointer<ID3D12Resource> m_RenderTargets[Constants::Graphics::SLIDE_COUNT];
        //ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        //UINT m_RTVDescriptorSize;

        //ComPointer<ID3D12Resource> m_VertexBuffer;
        //D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
        //std::wstring m_AssetsPath;

        //CD3DX12_VIEWPORT m_Viewport;
        //CD3DX12_RECT m_ScissorRect;
        //UINT m_Width;
        //UINT m_Height;
        //float m_AspectRatio;

        //UINT m_FrameIndex;
        //HANDLE m_FenceEvent;
        //ComPointer<ID3D12Fence> m_Fence;
        //UINT64 m_FenceValue;
    };
}
