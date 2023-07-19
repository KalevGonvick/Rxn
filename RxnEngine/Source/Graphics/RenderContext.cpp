#include "Rxn.h"
#include "RenderContext.h"

namespace Rxn::Graphics
{
    static RenderContext &GetContext()
    {
        static RenderContext inst;
        return inst;
    }

    RenderContext::RenderContext()
    {
    }

    RenderContext::~RenderContext() = default;

    void RenderContext::InitRenderContext()
    {
        HRESULT result;

        result = GetContext().CreateFactory();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create factory...");
            return;
        }

        result = GetContext().CreateIndependantDevice();
        if (FAILED(result))
        {
            RXN_LOGGER::Error(L"Failed to create DX12 device");
            return;
        }

        GetContext().SetHighestRootSignatureVersion();

        GetContext().m_IsInitialized = true;
    }

    void RenderContext::SetHWND(const HWND &hwnd)
    {
        GetContext().m_Hwnd = hwnd;
    }

    HWND &RenderContext::GetHWND()
    {
        return GetContext().m_Hwnd;
    }

    ComPointer<ID3D12Device> &RenderContext::GetGraphicsDevice()
    {
        return GetContext().m_Device;
    }

    ComPointer<IDXGIFactory4> &RenderContext::GetFactory()
    {
        return GetContext().m_Factory;
    }

    D3D_ROOT_SIGNATURE_VERSION &RenderContext::GetHighestRootSignatureVersion()
    {
        return GetContext().m_HighestRootSignatureVersion;
    }

    //D3D12_CPU_DESCRIPTOR_HANDLE &RenderContext::GetCachedSampler(uint64 hash)
    //{
    //    auto it = GetContext().m_SamplerCache.find(hash);
    //    if (it != GetContext().m_SamplerCache.end())
    //    {
    //        return it->second;
    //    }
    //    else
    //    {
    //        return NULL;
    //    }
    //}

    HRESULT RenderContext::CreateIndependantDevice()
    {
        if (m_UseWarpDevice)
        {
            ComPointer<IDXGIAdapter> warpAdapter;
            ThrowIfFailed(m_Factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
            return D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
        }
        else
        {
            ComPointer<IDXGIAdapter1> hardwareAdapter;
            GetHardwareAdapter(m_Factory.Get(), &hardwareAdapter);
            return D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
        }
    }

    HRESULT RenderContext::CreateFactory()
    {
        UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
        ComPointer<ID3D12Debug3> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
#endif

        return CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_Factory));
    }

    void RenderContext::SetHighestRootSignatureVersion()
    {

        m_HighestRootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &m_HighestRootSignatureVersion, sizeof(m_HighestRootSignatureVersion))))
        {
            m_HighestRootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }
    }

    void RenderContext::CheckTearingSupport()
    {
#ifndef PIXSUPPORT
        ComPointer<IDXGIFactory7> factory;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        BOOL allowTearing = FALSE;
        if (SUCCEEDED(hr))
        {
            hr = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        }

        m_HasTearingSupport = SUCCEEDED(hr) && allowTearing;
#else
        m_HasTearingSupport = TRUE;
#endif

    }

    void RenderContext::GetHardwareAdapter(IDXGIFactory1 *pFactory, IDXGIAdapter1 **ppAdapter, bool requestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;

        ComPointer<IDXGIAdapter1> adapter;
        ComPointer<IDXGIFactory7> factory7;

        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory7))))
        {
            for (UINT adapterIndex = 0; SUCCEEDED(factory7->EnumAdapterByGpuPreference(adapterIndex, requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter))); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    RXN_LOGGER::Info(L"Using software adaptor, no need to query for physical device...");
                    continue;
                }

                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    RXN_LOGGER::Info(L"Found device that supports DX12");
                    break;
                }
            }
        }

        if (adapter.Get() == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    RXN_LOGGER::Info(L"Using software adaptor, no need to setup physical device...");
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    RXN_LOGGER::Info(L"Created device successfully!");
                    break;
                }
            }
        }

        *ppAdapter = adapter.Detatch();
    }


}
