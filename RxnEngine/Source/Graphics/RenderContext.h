#pragma once


namespace Rxn::Graphics
{


    class RXN_ENGINE_API RenderContext
    {
    public:

        RenderContext();
        ~RenderContext();

    public:

        static void InitRenderContext();
        static void SetHWND(const HWND &hwnd);
        static HWND &GetHWND();
        static ComPointer<ID3D12Device> &GetGraphicsDevice();
        static ComPointer<IDXGIFactory4> &GetFactory();
        static D3D_ROOT_SIGNATURE_VERSION &GetHighestRootSignatureVersion();

        HRESULT CreateIndependantDevice();
        HRESULT CreateFactory();
        void SetHighestRootSignatureVersion();

        _Use_decl_annotations_;
        void GetHardwareAdapter(_In_ IDXGIFactory1 *pFactory, _Outptr_result_maybenull_ IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter = false);
        void CheckTearingSupport();

    public:

        HWND m_Hwnd;

    private:


        bool m_IsInitialized;
        bool m_HasTearingSupport;
        bool m_UseWarpDevice;

        std::unordered_map<uint64, D3D12_CPU_DESCRIPTOR_HANDLE> m_SamplerCache;

        ComPointer<ID3D12Device> m_Device;
        ComPointer<IDXGIFactory4> m_Factory;

        D3D_ROOT_SIGNATURE_VERSION m_HighestRootSignatureVersion;

    };
}
