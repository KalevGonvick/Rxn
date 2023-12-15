#pragma once


namespace Rxn::Graphics
{
    class RXN_ENGINE_API RenderContext
    {   
    public:

        RenderContext();
        ~RenderContext();
        RenderContext(const RenderContext &) = delete;
        RenderContext &operator=(const RenderContext &) = delete;

    public:

        /**
         * .
         * 
         */
        static void InitRenderContext();

        /**
         * .
         * 
         * \param hwnd
         */
        static void SetHWND(const HWND &hwnd);

        /**
         * .
         * 
         * \return 
         */
        static HWND &GetHWND();

        /**
         * .
         * 
         * \return 
         */
        static ComPointer<ID3D12Device> &GetGraphicsDevice();

        /**
         * .
         * 
         * \return 
         */
        static ComPointer<IDXGIFactory4> &GetFactory();

        /**
         * .
         * 
         * \return 
         */
        static D3D_ROOT_SIGNATURE_VERSION &GetHighestRootSignatureVersion();

        /**
         * .
         * 
         * \return 
         */
        HRESULT CreateIndependantDevice();

        /**
         * .
         * 
         * \return 
         */
        HRESULT CreateFactory();

        /**
         * .
         * 
         */
        void SetHighestRootSignatureVersion();

        /**
         * .
         * 
         * \param pFactory
         * \param ppAdapter
         * \param requestHighPerformanceAdapter
         */
        void GetHardwareAdapter(IDXGIFactory1 *pFactory, IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter = false);

        /**
         * .
         * 
         */
        void CheckTearingSupport();

        static const uint32 &GetEngineSeed();

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

        uint32 m_Seed;

    };
}
