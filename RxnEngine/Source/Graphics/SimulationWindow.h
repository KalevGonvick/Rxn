#pragma once

namespace Rxn::Graphics
{
    class SimulationWindow : public Platform::Win32::Window
    {
    public:
        SimulationWindow(WString windowTitle, WString windowClass, int width, int height);
        ~SimulationWindow();

        /**
         * MessageHandler - Handles WIN32 messages.
         *
         * \param hWnd      - handle
         * \param msg       - message to be processed.
         * \param wParam    - additional wparam from msg (optional)
         * \param lParam    - additional lparam from msg (optional)
         * \return
         */
        LRESULT MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;


        virtual void SetupWindow() override;

        void DX12_Destroy();
        void DX12_Render();

        void DX12_LoadPipeline();
        void DX12_LoadAssets();

    private:



        _Use_decl_annotations_;
        void DX12_GetHardwareAdapter(_In_ IDXGIFactory1 *pFactory, _Outptr_result_maybenull_ IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter = false);

        void DX12_CheckTearingSupport();
        void DX12_WaitForPreviousFrame();
        void DX12_PopulateCommandList();

        struct Vertex
        {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT4 color;
        };

        inline std::wstring GetAssetFullPath(LPCWSTR assetName)
        {
            return m_AssetsPath + assetName;
        }

        bool m_UseWarpDevice;
        bool m_HasTearingSupport;

        ComPointer<ID3D12Device> m_Device;

        ComPointer<ID3D12CommandQueue> m_CommandQueue;
        ComPointer<ID3D12CommandAllocator> m_CommandAllocator;
        ComPointer<ID3D12GraphicsCommandList> m_CommandList;
        ComPointer<IDXGISwapChain4> m_SwapChain;

        ComPointer<ID3D12RootSignature> m_RootSignature;
        ComPointer<ID3D12PipelineState> m_PipelineState;

        ComPointer<ID3D12Resource> m_RenderTargets[Constants::Graphics::SLIDE_COUNT];
        ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
        UINT m_RTVDescriptorSize;

        ComPointer<ID3D12Resource> m_VertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

        WString m_AssetsPath;

        CD3DX12_VIEWPORT m_Viewport;
        CD3DX12_RECT m_ScissorRect;

        UINT m_Width;
        UINT m_Height;

        float m_AspectRatio;

        UINT m_FrameIndex;
        HANDLE m_FenceEvent;

        ComPointer<ID3D12Fence> m_Fence;

        UINT64 m_FenceValue;

        bool m_IsRenderReady;
    };
}
