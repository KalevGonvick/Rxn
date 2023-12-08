#include "Rxn.h"
#include "PipelineLibraryManager.h"

namespace Rxn::Graphics::Manager
{
    PipelineLibraryManager::PipelineLibraryManager(uint32 frameCount, uint32 cbvRootSignatureIndex)
        : m_CBVRootSignatureIndex(cbvRootSignatureIndex)
        , m_DynamicConstantBuffer(sizeof(UberShaderConstantBuffer), frameCount)
    {
        WCHAR path[512];
        GetAssetsPath(path, _countof(path));
        m_CachePath = path;
    }

    PipelineLibraryManager::~PipelineLibraryManager()
    {
        WaitForThreads();

        /*for (uint32 i = 0; i < 12; i++)
        {
            m_DiskCaches[i].DestroyCache(false);
        }*/

        for (const auto & [index, pipeline]: m_PipelineLibraries) 
        {
            pipeline->m_DiskCache.DestroyCache(false);
        }

        m_PipelineLibrary.DestroyPipelineLibrary(false);
    }

    void PipelineLibraryManager::WaitForThreads()
    {
        /*for (auto &thread : m_WorkerThreads)
        {
            if (thread.threadHandle)
            {
                WaitForSingleObject(thread.threadHandle, INFINITE);
                CloseHandle(thread.threadHandle);
            }
            thread.threadHandle = nullptr;
        }*/
        RXN_LOGGER::Debug(L"Waiting on threads...");
        for (const auto &[index, pipeline] : m_PipelineLibraries)
        {
            if (pipeline->m_WorkerThread.threadHandle)
            {
                RXN_LOGGER::Debug(L"Found thread handle...");
                WaitForSingleObject(pipeline->m_WorkerThread.threadHandle, INFINITE);
                CloseHandle(pipeline->m_WorkerThread.threadHandle);
            }

            pipeline->m_WorkerThread.threadHandle = nullptr;
        }
    }
    
    void PipelineLibraryManager::Init(ComPointer<ID3D12Device> device)
    {
        m_Device = device;
        
        RXN_LOGGER::PrintLnHeader(L"Pipeline Library - Build Start");
        RXN_LOGGER::Debug(L"Building new pipeline cache.");

        RXN_LOGGER::Debug(L"Initialize all cache file mappings (file may be empty).");
        m_PipelineLibrariesSupported = m_PipelineLibrary.InitPipelineLibrary(m_Device, m_CachePath + g_cPipelineLibraryFileName);
        if (!m_PipelineLibrariesSupported)
        {
            RXN_LOGGER::Debug(L"Falling back to cached blobs.");
            m_PipelineStateObjectCachingMechanism = PSOCachingMechanism::CachedBlobs;
        }

        m_DynamicConstantBuffer.Create(m_Device);
        
    }

    void PipelineLibraryManager::BuildPipeline(ComPointer<ID3D12RootSignature> rootSignature, const uint32 effectIndex)
    {
        RXN_LOGGER::Debug(L"Initializeing cache file mapping for shader: [%d]", effectIndex);
        
        // insert new
        if (!m_PipelineLibraries.contains(effectIndex))
        {
            auto library = std::make_unique<GPU::PipelineLibrary>(effectIndex);
            m_PipelineLibraries.emplace(effectIndex, std::move(library));
        }
        
        m_PipelineLibraries.at(effectIndex)->m_WorkerThread.pipelineIndex = effectIndex;
        m_PipelineLibraries.at(effectIndex)->m_WorkerThread.m_Device = m_Device;
        m_PipelineLibraries.at(effectIndex)->m_WorkerThread.m_RootSignature = rootSignature;
        CompilePipelineStateObject(this, &m_PipelineLibraries.at(effectIndex)->m_WorkerThread);

        
    }

    PSOCachingMechanism PipelineLibraryManager::GetPipelineCachingMechanism()
    {
        return m_PipelineStateObjectCachingMechanism;
    }

    void PipelineLibraryManager::BuildCachedPipeline(const uint32 effectIndex)
    {
        //RXN_LOGGER::Debug(L"Initializeing cache file mapping for shader: [%d]", effectIndex);


        // insert new
        if (!m_PipelineLibraries.contains(effectIndex))
        {
            auto library = std::make_unique<GPU::PipelineLibrary>(effectIndex);
            m_PipelineLibraries.emplace(effectIndex, std::move(library));
        }

        //RXN_LOGGER::Debug(L"Initializeing cache file mapping for shader: [%d]", effectIndex);
        
        m_PipelineLibraries.at(effectIndex)->m_DiskCache.InitObjectCache(m_CachePath + m_CachePath + g_cCacheFileNames[effectIndex]);

        /*for (uint32 i = 0; i < BaseEffectCount; i++)
        {
            RXN_LOGGER::Debug(L" Compiling the 3D & 'Ubershader'.");
            m_WorkerThreads[i].pDevice = device;
            m_WorkerThreads[i].pRootSignature = rootSignature;
            m_WorkerThreads[i].type = EffectPipelineType(i);
            m_WorkerThreads[i].pLibrary = this;
            CompilePipelineStateObject(&m_WorkerThreads[i]);
        }*/

        //m_DynamicConstantBuffer.Create(device);
        //RXN_LOGGER::PrintLnSeperator();
    }


    void PipelineLibraryManager::SetPipelineState(ID3D12RootSignature *pRootSignature, ID3D12GraphicsCommandList *pCommandList, uint32 frameIndex, uint32 pipelineIndex)
    {

        bool isBuilt = false;
        bool isInFlight = false;

        assert(m_PipelineLibraries.contains(pipelineIndex));

        //const auto &pipeline = m_PipelineLibraries.at(pipelineIndex);
        
        {
            auto lock = std::scoped_lock(m_PipelineMutex);

            isBuilt = m_PipelineLibraries.at(pipelineIndex)->m_CompiledPipelineStateObjectFlag;
            isInFlight = m_PipelineLibraries.at(pipelineIndex)->m_InflightPipelineStateObjectFlag;

            //RXN_LOGGER::Debug(L"Built: %d Flight: %d", isBuilt, isInFlight);

        }

        
        if (!isBuilt && m_UseUberShaders)
        {
            RXN_LOGGER::Debug(L"Ubershader using effect: %d", pipelineIndex);
            auto constantData = (UberShaderConstantBuffer *)m_DynamicConstantBuffer.GetMappedMemory(frameIndex, pipelineIndex);
            constantData->effectIndex = pipelineIndex;
            pCommandList->SetGraphicsRootConstantBufferView(m_CBVRootSignatureIndex, m_DynamicConstantBuffer.GetGpuVirtualAddress(frameIndex, pipelineIndex));

            // We don't want to double compile
            if (!isInFlight)
            {
                m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.m_Device = RenderContext::GetGraphicsDevice().Get();
                m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.m_RootSignature = pRootSignature;
                m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.pipelineIndex = pipelineIndex;
                //m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.pLibrary = this;

                RXN_LOGGER::Debug(L"Compileing the pipeline state object on a background thread.");
                m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.threadHandle = CreateThread(
                    nullptr,
                    0,
                    reinterpret_cast<LPTHREAD_START_ROUTINE>(CompilePipelineStateObject),
                    reinterpret_cast<void *>(&m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread),
                    CREATE_SUSPENDED,
                    nullptr);

                if (m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.threadHandle)
                {
                    ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
                }

                ResumeThread(m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.threadHandle);

                {
                    auto lock = std::scoped_lock(m_PipelineMutex);

                    m_PipelineLibraries.at(pipelineIndex)->m_InflightPipelineStateObjectFlag = true;
                }
            }
        }
        else if (!isBuilt && !m_UseUberShaders)
        {
            RXN_LOGGER::Debug(L"Not using ubershaders... This will take a long time and cause a hitch as the CPU is stalled!");
            m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.m_Device = RenderContext::GetGraphicsDevice().Get();
            m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.m_RootSignature = pRootSignature;
            m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.pipelineIndex = pipelineIndex;
            //m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread.pLibrary = this;

            CompilePipelineStateObject(this, &m_PipelineLibraries.at(pipelineIndex)->m_WorkerThread);
        }
        

        pCommandList->SetPipelineState(m_PipelineLibraries.at(pipelineIndex)->m_PipelineState.Get());

        m_DrawIndex++;
    }

    // TODO
    // A lot of this can be moved to PipelineLibrary class itself
    void PipelineLibraryManager::CompilePipelineStateObject(PipelineLibraryManager *pipelineManager, GPU::CompilePipelineStateObjectThreadData *pDataPackage)
    {

        

        pDataPackage->Compile(pipelineManager);

        ////PipelineLibraryManager *pLibrary = pDataPackage->pLibrary;
        //ID3D12Device *pDevice = pDataPackage->pDevice;
        //ID3D12RootSignature *pRootSignature = pDataPackage->pRootSignature;
        //uint32 pipelineIndex = pDataPackage->pipelineIndex;
        //
        //bool useCache = false;
        //{
        //    auto lock = std::scoped_lock(pLibrary->m_PipelineMutex);
        //    useCache = pLibrary->m_UseDiskLibraries;
        //}

        //D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc = {};
        //baseDesc.pRootSignature = pRootSignature;
        //baseDesc.SampleMask = UINT_MAX;
        //baseDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        //baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        //baseDesc.NumRenderTargets = 1;
        //baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        //baseDesc.SampleDesc.Count = 1;
        //baseDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        //baseDesc.DepthStencilState.DepthEnable = FALSE;
        //baseDesc.DepthStencilState.StencilEnable = FALSE;

        //baseDesc.InputLayout = g_cEffectShaderData[pipelineIndex].inputLayout;
        //baseDesc.VS = g_cEffectShaderData[pipelineIndex].VS;
        //baseDesc.PS = g_cEffectShaderData[pipelineIndex].PS;
        //baseDesc.DS = g_cEffectShaderData[pipelineIndex].DS;
        //baseDesc.HS = g_cEffectShaderData[pipelineIndex].HS;
        //baseDesc.GS = g_cEffectShaderData[pipelineIndex].GS;

        //if (useCache && (pLibrary->m_PipelineStateObjectCachingMechanism == PSOCachingMechanism::PipelineLibraries))
        //{
        //    assert(pLibrary->m_PipelineLibrary.IsMapped());
        //    ID3D12PipelineLibrary *pPipelineLibrary = pLibrary->m_PipelineLibrary.GetPipelineLibrary();

        //    // Note: Load*Pipeline() will auto-name PSOs for you based on the provided name. However, this sample overrides those names.
        //    HRESULT hr = pPipelineLibrary->LoadGraphicsPipeline(g_cEffectNames[pipelineIndex], &baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_PipelineState));
        //    if (E_INVALIDARG == hr)
        //    {
        //        RXN_LOGGER::Debug(L"A pipeline state object' with the specified name '%s' does not exist, or the input desc does not match the data in the library.", g_cEffectNames[pipelineIndex]);
        //        RXN_LOGGER::Debug(L"Createing the PSO, and storeing it in the library for next time.");
        //        ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_PipelineState)));
        //        ThrowIfFailed(pPipelineLibrary->StorePipeline(g_cEffectNames[pipelineIndex], pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_PipelineState.Get()));
        //    }
        //    else
        //    {
        //        ThrowIfFailed(hr);
        //    }
        //}
        //else if (useCache && (pLibrary->m_PipelineStateObjectCachingMechanism == PSOCachingMechanism::CachedBlobs))
        //{
        //    assert(pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_DiskCache.IsMapped());
        //    size_t size = pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_DiskCache.GetCachedBlobSize();
        //    if (size == 0)
        //    {
        //        RXN_LOGGER::Debug(L"File size is 0... The disk cache needs to be refreshed...");
        //        //ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_PipelineState)));

        //        ComPointer<ID3DBlob> blob;
        //        pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_PipelineState->GetCachedBlob(&blob);
        //        pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_DiskCache.Update(blob.Get());
        //    }
        //    else
        //    {
        //        RXN_LOGGER::Debug(L"Reading the blob data from disk instead of re-compiling shaders!");
        //        baseDesc.CachedPSO.pCachedBlob = pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_DiskCache.GetCachedBlob();
        //        baseDesc.CachedPSO.CachedBlobSizeInBytes = pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_DiskCache.GetCachedBlobSize();

        //        HRESULT hr = pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_PipelineState));


        //        if (FAILED(hr))
        //        {
        //            RXN_LOGGER::Debug(L"Compilation failed.... The cache is probably stale. (old drivers etc.)");
        //            baseDesc.CachedPSO = {};
        //            ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_PipelineState)));

        //            ComPointer<ID3DBlob> blob;
        //            pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_PipelineState->GetCachedBlob(&blob);
        //            pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_DiskCache.Update(blob.Get());
        //        }
        //    }
        //}
        //else
        //{
        //    ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_PipelineState)));
        //}

        //WCHAR name[50];
        //if (swprintf_s(name, L"m_PipelineStates[%s]", g_cEffectNames[pipelineIndex]) > 0)
        //{
        //    SetName(pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_PipelineState.Get(), name);
        //}

        //{
        //    auto lock = std::scoped_lock(pLibrary->m_PipelineMutex);

        //    pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_CompiledPipelineStateObjectFlag = true;
        //    pLibrary->m_PipelineLibraries.at(pipelineIndex)->m_InflightPipelineStateObjectFlag = false;
        //}
    }

    void PipelineLibraryManager::EndFrame()
    {
        m_DrawIndex = 0;
    }

    void PipelineLibraryManager::ClearPSOCache()
    {
        WaitForThreads();

        // THIS IS NOT GOOD
        // 2 to 12 because the first two are compiled on startup
        // this should be reworked!!!
        /*for (size_t i = 2; i < 12; i++)
        {
            if (m_PipelineStates[i])
            {
                m_PipelineStates[i] = nullptr;
                m_CompiledPipelineStateObjectFlags[i] = false;
                m_InflightPipelineStateObjectFlags[i] = false;
            }
        }

        RXN_LOGGER::Debug(L"Clear the disk caches.");
        for (size_t i = 0; i < 12; i++)
        {
            m_DiskCaches[i].DestroyCache(true);
        }*/

        for (const auto &[index, pipeline] : m_PipelineLibraries)
        {
            pipeline->m_PipelineState.Release();
            pipeline->m_CompiledPipelineStateObjectFlag = false;
            pipeline->m_InflightPipelineStateObjectFlag = false;
            pipeline->m_DiskCache.DestroyCache(true);
        }

        m_PipelineLibrary.DestroyPipelineLibrary(true);
        m_PipelineLibraries.clear();
    }

    void PipelineLibraryManager::ToggleUberShader()
    {
        m_UseUberShaders = !m_UseUberShaders;
    }

    void PipelineLibraryManager::ToggleDiskLibrary()
    {
        {
            auto lock = std::scoped_lock(m_PipelineMutex);

            m_UseDiskLibraries = !m_UseDiskLibraries;
        }

        WaitForThreads();
    }

    void PipelineLibraryManager::SwitchPSOCachingMechanism()
    {
        {
            auto lock = std::scoped_lock(m_PipelineMutex);

            uint32 newMechanism = static_cast<uint32>(m_PipelineStateObjectCachingMechanism) + 1;
            newMechanism = newMechanism % PSOCachingMechanism::PSOCachingMechanismCount;

            if (!m_PipelineLibrariesSupported && (newMechanism == PSOCachingMechanism::PipelineLibraries))
            {
                RXN_LOGGER::Warn(L"Pipeline library is not available...");
                newMechanism++;
                newMechanism = newMechanism % PSOCachingMechanism::PSOCachingMechanismCount;
            }

            m_PipelineStateObjectCachingMechanism = static_cast<PSOCachingMechanism>(newMechanism);
        }

        WaitForThreads();
    }

    void PipelineLibraryManager::DestroyPipeline(uint32 pipelineIndex)
    {
        WaitForThreads();
        /*if (m_PipelineStates[type])
        {
            m_PipelineStates[type] = nullptr;
            m_CompiledPipelineStateObjectFlags[type] = false;
            m_InflightPipelineStateObjectFlags[type] = false;
        }*/
        if (m_PipelineLibraries.contains(pipelineIndex))
        {
            m_PipelineLibraries.at(pipelineIndex)->m_PipelineState = nullptr;
            m_PipelineLibraries.at(pipelineIndex)->m_CompiledPipelineStateObjectFlag = false;
            m_PipelineLibraries.at(pipelineIndex)->m_InflightPipelineStateObjectFlag = false;
        }
        
    }

    bool PipelineLibraryManager::UberShadersEnabled() const
    {
        return m_UseUberShaders;
    }

    bool PipelineLibraryManager::DiskCacheEnabled() const
    {
        return m_UseDiskLibraries;
    }

    PSOCachingMechanism PipelineLibraryManager::GetPSOCachingMechanism() const
    {
        return m_PipelineStateObjectCachingMechanism;
    }

}


