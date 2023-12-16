#include "Rxn.h"
#include "PipelineLibrary.h"
#include <filesystem>

namespace Rxn::Graphics::Mapped
{
    PipelineLibrary::PipelineLibrary(uint32 frameCount, uint32 cbvRootSignatureIndex)
        : m_CBVRootSignatureIndex(cbvRootSignatureIndex)
        , m_DynamicConstantBuffer(sizeof(UberShaderConstantBuffer), m_MaxDrawsPerFrame, frameCount)
    {
        WCHAR path[512];
        GetAssetsPath(path, _countof(path));
        m_CachePath = path;

        m_PipelineEffects.push_back(g_Normal3D);
        m_PipelineEffects.push_back(g_GenericPostEffect);
        m_PipelineEffects.push_back(g_Blit);
        m_PipelineEffects.push_back(g_Invert);
        m_PipelineEffects.push_back(g_Grayscale);
        m_PipelineEffects.push_back(g_EdgeDetect);
        m_PipelineEffects.push_back(g_Blur);
        m_PipelineEffects.push_back(g_Warp);
        m_PipelineEffects.push_back(g_Pixelate);
        m_PipelineEffects.push_back(g_Distort);
        m_PipelineEffects.push_back(g_Wave);
        m_PipelineEffects.push_back(g_Additional);
    }

    PipelineLibrary::~PipelineLibrary()
    {
        WaitForThreads();

        /*for (uint32 i = 0; i < EffectPipelineTypeCount; i++)
        {
            m_DiskCaches[i].DestroyFile(false);
        }*/
        uint32 x = 0;
        for (auto &effect : m_PipelineEffects)
        {
            m_DiskCaches[x].DestroyFile(false);
            x++;
        }

        m_PipelineLibrary.DestroyPipelineLibrary(false);
    }

    void PipelineLibrary::WaitForThreads()
    {
        for (auto &thread : m_WorkerThreads)
        {
            if (thread.threadHandle)
            {
                auto index = static_cast<uint32>(thread.type);
                RXN_LOGGER::Trace(L"Waiting on the compile thread to finish for shader effect: '%s'.", m_PipelineEffects.at(index).GetEffectName().c_str());
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

        RXN_LOGGER::Trace(L"Initialize all cache file mappings (file may be empty).");
        m_PipelineLibrariesSupported = m_PipelineLibrary.InitPipelineLibrary(pDevice, m_CachePath + PIPELINE_LIBRARY_FILE_NAME);
        /*for (uint32 i = 0; i < EffectPipelineTypeCount; i++)
        {
            RXN_LOGGER::Trace(L"Initializeing cache file mapping for shader: [%d]", i);
            m_DiskCaches[i].InitFile(m_CachePath + g_cCacheFileNames[i]);
        }*/
        uint32 x = 0;
        for (auto &effect : m_PipelineEffects)
        {
            WString wName = std::filesystem::path(effect.GetEffectName().c_str()).wstring();
            RXN_LOGGER::Trace(L"Initializeing cache file mapping for shader: [%d]", x);
            m_DiskCaches[x].InitFile(m_CachePath + wName.c_str());
            x++;
        }

        if (!m_PipelineLibrariesSupported)
        {
            RXN_LOGGER::Trace(L"Falling back to cached blobs.");
            m_PipelineStateObjectCachingMechanism = PSOCachingMechanism::CachedBlobs;
        }

        /*for (uint32 i = 0; i < BaseEffectCount; i++)
        {
            RXN_LOGGER::Trace(L" Compiling the 3D & 'Ubershader'.");
            m_WorkerThreads[i].pDevice = pDevice;
            m_WorkerThreads[i].pRootSignature = pRootSignature;
            m_WorkerThreads[i].type = EffectPipelineType(i);
            m_WorkerThreads[i].pLibrary = this;
            CompilePipelineStateObject(&m_WorkerThreads[i]);
        }*/
        uint32 i = 0;
        for (auto &effect : m_PipelineEffects)
        {
            if (effect.IsBaseEffect())
            {
                WString name = std::filesystem::path(effect.GetEffectName()).wstring();
                RXN_LOGGER::Debug(L"Compiling base effect: %s", name.c_str());
                m_WorkerThreads[i].pDevice = pDevice;
                m_WorkerThreads[i].pRootSignature = pRootSignature;
                m_WorkerThreads[i].type = EffectPipelineType(i);
                m_WorkerThreads[i].pLibrary = this;
                CompilePipelineStateObject(&m_WorkerThreads[i]);
            }
            i++;
        }

        m_DynamicConstantBuffer.Create(pDevice);
        RXN_LOGGER::PrintLnSeperator();
    }

    
    void PipelineLibrary::SetPipelineState(ID3D12RootSignature *pRootSignature, ID3D12GraphicsCommandList *pCommandList, _In_range_(0, EffectPipelineTypeCount - 1) EffectPipelineType type, uint32 frameIndex)
    {
        assert(m_DrawIndex < m_MaxDrawsPerFrame);

        bool isBuilt = false;
        bool isInFlight = false;

        {
            auto lock = std::scoped_lock(m_FlagsMutex);

            isBuilt = m_CompiledPipelineStateObjectFlags[type];
            isInFlight = m_InflightPipelineStateObjectFlags[type];
        }


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
        auto index = static_cast<uint32>(type);
        WString wPipelienEffectName = std::filesystem::path(pLibrary->m_PipelineEffects.at(index).GetEffectName()).wstring();
        //RXN_LOGGER::Debug(L"Compiling effect: '%s'", g_cEffectNames[type]);
        RXN_LOGGER::Debug(L"Compiling effect: '%s'", wPipelienEffectName.c_str());

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

        baseDesc.InputLayout = pLibrary->m_PipelineEffects.at(index).GetGraphicsShaderSet().inputLayout;
        baseDesc.VS = pLibrary->m_PipelineEffects.at(index).GetGraphicsShaderSet().VS;
        baseDesc.PS = pLibrary->m_PipelineEffects.at(index).GetGraphicsShaderSet().PS;
        baseDesc.DS = pLibrary->m_PipelineEffects.at(index).GetGraphicsShaderSet().DS;
        baseDesc.HS = pLibrary->m_PipelineEffects.at(index).GetGraphicsShaderSet().HS;
        baseDesc.GS = pLibrary->m_PipelineEffects.at(index).GetGraphicsShaderSet().GS;

        if (useCache && (pLibrary->m_PipelineStateObjectCachingMechanism == PSOCachingMechanism::PipelineLibraries))
        {
            assert(pLibrary->m_PipelineLibrary.IsMapped());
            ID3D12PipelineLibrary *pPipelineLibrary = pLibrary->m_PipelineLibrary.GetPipelineLibrary().Get();

            // Note: Load*Pipeline() will auto-name PSOs for you based on the provided name. However, this sample overrides those names.
            //HRESULT hr = pPipelineLibrary->LoadGraphicsPipeline(g_cEffectNames[type], &baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineStates[type]));
           
            HRESULT hr = pPipelineLibrary->LoadGraphicsPipeline(wPipelienEffectName.c_str(), &baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineStates[type]));
            if (E_INVALIDARG == hr)
            {
                RXN_LOGGER::Trace(L"A pipeline state object' with the specified name '%s' does not exist, or the input desc does not match the data in the library.", wPipelienEffectName.c_str());
                ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineStates[type])));

                hr = pPipelineLibrary->StorePipeline(wPipelienEffectName.c_str(), pLibrary->m_PipelineStates[type].Get());
                if (E_INVALIDARG == hr)
                {
                    RXN_LOGGER::Error(L"A PSO with the specified name '%s' already exists in the library.", wPipelienEffectName.c_str());
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
            size_t size = pLibrary->m_DiskCaches[type].GetSize();
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
                baseDesc.CachedPSO.pCachedBlob = pLibrary->m_DiskCaches[type].GetData();
                baseDesc.CachedPSO.CachedBlobSizeInBytes = pLibrary->m_DiskCaches[type].GetSize();

                HRESULT hr = pDevice->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pLibrary->m_PipelineStates[type]));


                if (FAILED(hr))
                {
                    RXN_LOGGER::Error(L"Compilation failed.... The cache is probably stale. (old drivers etc.)");
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
        if (swprintf_s(name, L"m_PipelineStates[%s]",wPipelienEffectName.c_str()) > 0)
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
        RXN_LOGGER::Debug(L"Clearing pipeline caches...");
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

        for (size_t i = 0; i < EffectPipelineTypeCount; i++)
        {
            m_DiskCaches[i].DestroyFile(true);
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
        RXN_LOGGER::Debug(L"Switching pipeline caching mechanism...");
        {
            auto lock = std::scoped_lock(m_FlagsMutex);

            uint32 newMechanism = static_cast<uint32>(m_PipelineStateObjectCachingMechanism) + 1;
            newMechanism = newMechanism % PSOCachingMechanism::PSOCachingMechanismCount;

            
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
        //RXN_LOGGER::Debug(L"Destroying shader effect '%s'", g_cEffectNames[type]);
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


