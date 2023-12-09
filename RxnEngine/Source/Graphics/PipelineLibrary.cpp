#include "Rxn.h"
#include "PipelineLibrary.h"

namespace Rxn::Graphics::Mapped
{
    PipelineLibrary::PipelineLibrary(uint32 frameCount, uint32 cbvRootSignatureIndex)
        : m_CBVRootSignatureIndex(cbvRootSignatureIndex)
        , m_DynamicConstantBuffer(sizeof(UberShaderConstantBuffer), m_MaxDrawsPerFrame, frameCount)
    {
        WCHAR path[512];
        GetAssetsPath(path, _countof(path));
        m_CachePath = path;
    }

    PipelineLibrary::~PipelineLibrary()
    {
        WaitForThreads();

        for (uint32 i = 0; i < EffectPipelineTypeCount; i++)
        {
            m_DiskCaches[i].DestroyCache(false);
        }

        m_PipelineLibrary.DestroyPipelineLibrary(false);
    }

    void PipelineLibrary::WaitForThreads()
    {
        for (auto &thread : m_WorkerThreads)
        {
            if (thread.threadHandle)
            {
                WaitForSingleObject(thread.threadHandle, INFINITE);
                CloseHandle(thread.threadHandle);
            }
            thread.threadHandle = nullptr;
        }
    }

    void PipelineLibrary::Build(ID3D12Device *pDevice, ID3D12RootSignature *pRootSignature)
    {
        RXN_LOGGER::PrintLnHeader(L"Pipeline Library - Build Start");
        RXN_LOGGER::Debug(L"Building new pipeline cache.");

        RXN_LOGGER::Debug(L"Initialize all cache file mappings (file may be empty).");
        m_PipelineLibrariesSupported = m_PipelineLibrary.InitPipelineLibrary(pDevice, m_CachePath + g_cPipelineLibraryFileName);
        for (uint32 i = 0; i < EffectPipelineTypeCount; i++)
        {
            RXN_LOGGER::Debug(L"Initializeing cache file mapping for shader: [%d]", i);
            m_DiskCaches[i].InitObjectCache(m_CachePath + g_cCacheFileNames[i]);
        }

        if (!m_PipelineLibrariesSupported)
        {
            RXN_LOGGER::Debug(L"Falling back to cached blobs.");
            m_PipelineStateObjectCachingMechanism = PSOCachingMechanism::CachedBlobs;
        }

        for (uint32 i = 0; i < BaseEffectCount; i++)
        {
            RXN_LOGGER::Debug(L" Compiling the 3D & 'Ubershader'.");
            m_WorkerThreads[i].pDevice = pDevice;
            m_WorkerThreads[i].pRootSignature = pRootSignature;
            m_WorkerThreads[i].type = EffectPipelineType(i);
            m_WorkerThreads[i].pLibrary = this;
            CompilePipelineStateObject(&m_WorkerThreads[i]);
        }

        m_DynamicConstantBuffer.Create(pDevice);
        RXN_LOGGER::PrintLnSeperator();
    }

    
    void PipelineLibrary::SetPipelineState(ID3D12RootSignature *pRootSignature, ID3D12GraphicsCommandList *pCommandList, _In_range_(0, EffectPipelineTypeCount - 1) EffectPipelineType type, uint32 frameIndex)
    {
        assert(m_DrawIndex < m_MaxDrawsPerFrame);
        //assert(m_WorkerThreads[type].threadHandle != 0);

        bool isBuilt = false;
        bool isInFlight = false;

        {
            auto lock = std::scoped_lock(m_FlagsMutex);

            isBuilt = m_CompiledPipelineStateObjectFlags[type];
            isInFlight = m_InflightPipelineStateObjectFlags[type];
        }

        if (type > BaseUberShader)
        {
            if (!isBuilt && m_UseUberShaders)
            {
                RXN_LOGGER::Debug(L"Ubershader using effect: %d", type);
                auto constantData = (UberShaderConstantBuffer *)m_DynamicConstantBuffer.GetMappedMemory(m_DrawIndex, frameIndex);
                constantData->effectIndex = type;
                pCommandList->SetGraphicsRootConstantBufferView(m_CBVRootSignatureIndex, m_DynamicConstantBuffer.GetGpuVirtualAddress(m_DrawIndex, frameIndex));

                // We don't want to double compile
                if (!isInFlight)
                {
                    m_WorkerThreads[type].pDevice = RenderContext::GetGraphicsDevice().Get();
                    m_WorkerThreads[type].pRootSignature = pRootSignature;
                    m_WorkerThreads[type].type = type;
                    m_WorkerThreads[type].pLibrary = this;

                    RXN_LOGGER::Debug(L"Compileing the pipeline state object on a background thread.");
                    m_WorkerThreads[type].threadHandle = CreateThread(
                        nullptr,
                        0,
                        reinterpret_cast<LPTHREAD_START_ROUTINE>(CompilePipelineStateObject),
                        reinterpret_cast<void *>(&m_WorkerThreads[type]),
                        CREATE_SUSPENDED,
                        nullptr);

                    if (!m_WorkerThreads[type].threadHandle)
                    {
                        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
                    }

                    ResumeThread(m_WorkerThreads[type].threadHandle);

                    {
                        auto lock = std::scoped_lock(m_FlagsMutex);

                        m_InflightPipelineStateObjectFlags[type] = true;
                    }
                }

                type = BaseUberShader;
            }
            else if (!isBuilt && !m_UseUberShaders)
            {
                RXN_LOGGER::Debug(L"Not using ubershaders... This will take a long time and cause a hitch as the CPU is stalled!");
                m_WorkerThreads[type].pDevice = RenderContext::GetGraphicsDevice().Get();
                m_WorkerThreads[type].pRootSignature = pRootSignature;
                m_WorkerThreads[type].type = type;
                m_WorkerThreads[type].pLibrary = this;

                CompilePipelineStateObject(&m_WorkerThreads[type]);
            }
        }
        else
        {
            assert(isBuilt);
        }

        pCommandList->SetPipelineState(m_PipelineStates[type].Get());

        m_DrawIndex++;
    }

    // TODO - This function is nonsense. Move this function to the modern world with std::thread instead.
    void PipelineLibrary::CompilePipelineStateObject(CompilePipelineStateObjectThreadData *pDataPackage)
    {
        PipelineLibrary *pLibrary = pDataPackage->pLibrary;
        ID3D12Device *pDevice = pDataPackage->pDevice;
        ID3D12RootSignature *pRootSignature = pDataPackage->pRootSignature;
        EffectPipelineType type = pDataPackage->type;

        bool useCache = CompileCacheCheck(pDataPackage);

        D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc = {};
        baseDesc.pRootSignature = pRootSignature;
        baseDesc.SampleMask = UINT_MAX;
        baseDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        baseDesc.NumRenderTargets = 1;
        baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        baseDesc.SampleDesc.Count = 1;
        baseDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        baseDesc.DepthStencilState.DepthEnable = FALSE;
        baseDesc.DepthStencilState.StencilEnable = FALSE;

        baseDesc.InputLayout = g_cEffectShaderData[type].inputLayout;
        baseDesc.VS = g_cEffectShaderData[type].VS;
        baseDesc.PS = g_cEffectShaderData[type].PS;
        baseDesc.DS = g_cEffectShaderData[type].DS;
        baseDesc.HS = g_cEffectShaderData[type].HS;
        baseDesc.GS = g_cEffectShaderData[type].GS;

        if (useCache && (pLibrary->m_PipelineStateObjectCachingMechanism == PSOCachingMechanism::PipelineLibraries))
        {
            assert(pLibrary->m_PipelineLibrary.IsMapped());
            ID3D12PipelineLibrary *pPipelineLibrary = pLibrary->m_PipelineLibrary.GetPipelineLibrary();

            // Note: Load*Pipeline() will auto-name PSOs for you based on the provided name. However, this sample overrides those names.
            HRESULT hr = pPipelineLibrary->LoadGraphicsPipeline(g_cEffectNames[type], &baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineStates[type]));
            if (E_INVALIDARG == hr)
            {
                RXN_LOGGER::Debug(L"A pipeline state object' with the specified name '%s' does not exist, or the input desc does not match the data in the library.", g_cEffectNames[type]);
                RXN_LOGGER::Debug(L"Createing the PSO, and storeing it in the library for next time.");
                ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineStates[type])));

                hr = pPipelineLibrary->StorePipeline(g_cEffectNames[type], pLibrary->m_PipelineStates[type].Get());
                if (E_INVALIDARG == hr)
                {
                    RXN_LOGGER::Debug(L"A PSO with the specified name '%s' already exists in the library.", g_cEffectNames[type]);
                    throw std::runtime_error("A PSO with the specified name already exists in the library.");
                }
                else
                {
                    ThrowIfFailed(hr);
                }
            }
            else
            {
                ThrowIfFailed(hr);
            }
        }
        else if (useCache && (pLibrary->m_PipelineStateObjectCachingMechanism == PSOCachingMechanism::CachedBlobs))
        {
            assert(pLibrary->m_DiskCaches[type].IsMapped());
            size_t size = pLibrary->m_DiskCaches[type].GetCachedBlobSize();
            if (size == 0)
            {
                RXN_LOGGER::Trace(L"File size is 0... The disk cache needs to be refreshed...");
                ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineStates[type])));

                ComPointer<ID3DBlob> blob;
                pLibrary->m_PipelineStates[type]->GetCachedBlob(&blob);
                pLibrary->m_DiskCaches[type].Update(blob.Get());
            }
            else
            {
                RXN_LOGGER::Trace(L"Reading the blob data from disk instead of re-compiling shaders!");
                baseDesc.CachedPSO.pCachedBlob = pLibrary->m_DiskCaches[type].GetCachedBlob();
                baseDesc.CachedPSO.CachedBlobSizeInBytes = pLibrary->m_DiskCaches[type].GetCachedBlobSize();

                HRESULT hr = pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineStates[type]));


                if (FAILED(hr))
                {
                    RXN_LOGGER::Trace(L"Compilation failed.... The cache is probably stale. (old drivers etc.)");
                    baseDesc.CachedPSO = {};
                    ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineStates[type])));

                    ComPointer<ID3DBlob> blob;
                    pLibrary->m_PipelineStates[type]->GetCachedBlob(&blob);
                    pLibrary->m_DiskCaches[type].Update(blob.Get());
                }
            }
        }
        else
        {
            ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineStates[type])));
        }

        WCHAR name[50];
        if (swprintf_s(name, L"m_PipelineStates[%s]", g_cEffectNames[type]) > 0)
        {
            SetName(pLibrary->m_PipelineStates[type].Get(), name);
        }

        SetThreadCompileFinishFlags(pDataPackage);
    }

    bool PipelineLibrary::CompileCacheCheck(CompilePipelineStateObjectThreadData *pDataPackage)
    {
        
        auto lock = std::scoped_lock(pDataPackage->pLibrary->m_FlagsMutex);
        return pDataPackage->pLibrary->m_UseDiskLibraries;
        
    }

    void PipelineLibrary::SetThreadCompileFinishFlags(CompilePipelineStateObjectThreadData *pDataPackage)
    {
        auto lock = std::scoped_lock(pDataPackage->pLibrary->m_FlagsMutex);

        pDataPackage->pLibrary->m_CompiledPipelineStateObjectFlags[pDataPackage->type] = true;
        pDataPackage->pLibrary->m_InflightPipelineStateObjectFlags[pDataPackage->type] = false;
    }

    void PipelineLibrary::EndFrame()
    {
        m_DrawIndex = 0;
    }

    void PipelineLibrary::ClearPSOCache()
    {
        WaitForThreads();

        for (size_t i = PostBlit; i < EffectPipelineTypeCount; i++)
        {
            if (m_PipelineStates[i])
            {
                m_PipelineStates[i] = nullptr;
                m_CompiledPipelineStateObjectFlags[i] = false;
                m_InflightPipelineStateObjectFlags[i] = false;
            }
        }

        RXN_LOGGER::Debug(L"Clear the disk caches.");
        for (size_t i = 0; i < EffectPipelineTypeCount; i++)
        {
            m_DiskCaches[i].DestroyCache(true);
        }

        m_PipelineLibrary.DestroyPipelineLibrary(true);
    }

    void PipelineLibrary::ToggleUberShader()
    {
        m_UseUberShaders = !m_UseUberShaders;
    }

    void PipelineLibrary::ToggleDiskLibrary()
    {
        {
            auto lock = std::scoped_lock(m_FlagsMutex);

            m_UseDiskLibraries = !m_UseDiskLibraries;
        }

        WaitForThreads();
    }

    void PipelineLibrary::SwitchPSOCachingMechanism()
    {
        {
            auto lock = std::scoped_lock(m_FlagsMutex);

            uint32 newMechanism = static_cast<uint32>(m_PipelineStateObjectCachingMechanism) + 1;
            newMechanism = newMechanism % PSOCachingMechanism::PSOCachingMechanismCount;

            RXN_LOGGER::Debug(L"Don't allow Pipeline Libraries if they're not available.");
            if (!m_PipelineLibrariesSupported && (newMechanism == PSOCachingMechanism::PipelineLibraries))
            {
                newMechanism++;
                newMechanism = newMechanism % PSOCachingMechanism::PSOCachingMechanismCount;
            }

            m_PipelineStateObjectCachingMechanism = static_cast<PSOCachingMechanism>(newMechanism);
        }

        WaitForThreads();
    }

    void PipelineLibrary::DestroyShader(EffectPipelineType type)
    {
        WaitForThreads();

        if (m_PipelineStates[type])
        {
            m_PipelineStates[type] = nullptr;
            m_CompiledPipelineStateObjectFlags[type] = false;
            m_InflightPipelineStateObjectFlags[type] = false;
        }
    }

    bool PipelineLibrary::UberShadersEnabled() const
    {
        return m_UseUberShaders;
    }

    bool PipelineLibrary::DiskCacheEnabled() const
    {
        return m_UseDiskLibraries;
    }

    PSOCachingMechanism PipelineLibrary::GetPSOCachingMechanism() const
    {
        return m_PipelineStateObjectCachingMechanism;
    }


}


