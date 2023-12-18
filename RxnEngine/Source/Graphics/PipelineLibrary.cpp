#include "Rxn.h"
#include "PipelineLibrary.h"
#include <filesystem>
#include <mutex>

namespace Rxn::Graphics::Mapped
{
    PipelineLibrary::PipelineLibrary(uint32 frameCount, uint32 cbvRootSignatureIndex)
        : m_CbvRootSignatureIndex(cbvRootSignatureIndex)
        , m_DynamicConstantBuffer(sizeof(UberShaderConstantBuffer), MAX_DRAWS_PER_FRAME, frameCount)
    {

        wchar_t path[512];
        Core::Strings::GetAssetsPath(path, _countof(path));
        m_CachePath = path;

        /* Push back some preset pipeline effects... */
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
        for (auto &effect : m_PipelineEffects)
        {
            effect.diskCache.DestroyFile(false);
        }

        m_PipelineLibrary.DestroyPipelineLibrary(false);
    }

    void PipelineLibrary::WaitForThreads()
    {
        for (auto &effect : m_PipelineEffects)
        {
            if (effect.threadHandle)
            {
                WaitForSingleObject(effect.threadHandle, INFINITE);
                CloseHandle(effect.threadHandle);
            }
        }
    }

    void PipelineLibrary::Build(ID3D12Device *pDevice, ID3D12RootSignature *pRootSignature)
    {
        RXN_LOGGER::PrintLnHeader(L"Pipeline Library - Build Start");

        m_PipelineLibrariesSupported = m_PipelineLibrary.InitPipelineLibrary(pDevice, m_CachePath + PIPELINE_LIBRARY_FILE_NAME);

        for (auto &effect : m_PipelineEffects)
        {
            effect.pipelineLibrary = (this);
        }

        for (auto &effect : m_PipelineEffects)
        {
            WString wName = std::filesystem::path(effect.GetEffectName().c_str()).wstring();
            effect.diskCache.InitFile(m_CachePath + wName.c_str());
        }

        if (!m_PipelineLibrariesSupported)
        {
            RXN_LOGGER::Trace(L"Falling back to cached blobs.");
            m_PipelineStateObjectCachingMechanism = PSOCachingMechanism::CachedBlobs;
        }

        for (auto &effect : m_PipelineEffects)
        {
            if (effect.IsBaseEffect())
            {
                WString name = std::filesystem::path(effect.GetEffectName()).wstring();
                RXN_LOGGER::Debug(L"Compiling base effect: %s", name.c_str());
                
                effect.device = pDevice;
                effect.rootSignature = pRootSignature;
                effect.pipelineLibrary = this;
                CompilePipelineStateObject(&effect);
            }
        }

        m_DynamicConstantBuffer.Create(pDevice);
        RXN_LOGGER::PrintLnSeperator();
    }

    
    void PipelineLibrary::SetPipelineState(ID3D12RootSignature *pRootSignature, ID3D12GraphicsCommandList *pCommandList, uint32 pipelineIndex, uint32 frameIndex)
    {
        assert(m_DrawIndex < MAX_DRAWS_PER_FRAME);

        auto &pipelineEffect = m_PipelineEffects.at(pipelineIndex);
        bool isBuilt = GetPipelineCompileFlagSafe(&pipelineEffect);
        bool isInFlight = GetPipelineInFlightFlagSafe(&pipelineEffect);


        if (!isBuilt && m_UseUberShaders)
        {
            RXN_LOGGER::Debug(L"Ubershader using effect: %d", pipelineIndex);
            auto constantData = (UberShaderConstantBuffer *)m_DynamicConstantBuffer.GetMappedMemory(m_DrawIndex, frameIndex);
            constantData->effectIndex = pipelineIndex;
            pCommandList->SetGraphicsRootConstantBufferView(m_CbvRootSignatureIndex, m_DynamicConstantBuffer.GetGpuVirtualAddress(m_DrawIndex, frameIndex));

            // We don't want to double compile
            if (!isInFlight)
            {
                pipelineEffect.device = RenderContext::GetGraphicsDevice().Get();
                pipelineEffect.rootSignature = pRootSignature;
                pipelineEffect.pipelineLibrary = this;

                RXN_LOGGER::Debug(L"Compileing the pipeline state object on a background thread.");

                pipelineEffect.threadHandle = (CreateThread(
                    nullptr,
                    0,
                    reinterpret_cast<LPTHREAD_START_ROUTINE>(CompilePipelineStateObject),
                    reinterpret_cast<void *>(&m_PipelineEffects.at(pipelineIndex)),
                    CREATE_SUSPENDED,
                    nullptr));

                if (!pipelineEffect.threadHandle)
                {
                    ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
                }

                if (pipelineEffect.threadHandle)
                {
                    ResumeThread(pipelineEffect.threadHandle);
                }

                SetPipelineInFlightFlagSafe(&pipelineEffect, true);
            }

            pCommandList->SetPipelineState(m_PipelineEffects.at(2).pipelineState.Get());
        }
        else if (!isBuilt && !m_UseUberShaders)
        {
            RXN_LOGGER::Debug(L"Not using ubershaders... This will take a long time and cause a hitch as the CPU is stalled!");
            pipelineEffect.device = RenderContext::GetGraphicsDevice().Get();
            pipelineEffect.rootSignature = pRootSignature;
            pipelineEffect.pipelineLibrary = this;

            CompilePipelineStateObject(&pipelineEffect);
            pCommandList->SetPipelineState(pipelineEffect.pipelineState.Get());
        }
        else
        {
            pCommandList->SetPipelineState(pipelineEffect.pipelineState.Get());
        }
    
        

        m_DrawIndex++;
    }

    void PipelineLibrary::FallbackToCompiledPipeline(ID3D12GraphicsCommandList *pCommandList)
    {
        for (auto &effect : m_PipelineEffects)
        {
            /* Fall backt to pipeline effect in ready to use state. */
            if (effect.compileFlag && !effect.flightFlag && effect.pipelineState)
            {
                pCommandList->SetPipelineState(effect.pipelineState.Get());
                break;
            }
        }
    }

    void PipelineLibrary::CompilePipelineStateObject(PipelineEffectTemplate *pPipelineEffect)
    {

        WString wPipelienEffectName = std::filesystem::path(pPipelineEffect->GetEffectName()).wstring();
        RXN_LOGGER::Debug(L"Compiling effect: '%s'", wPipelienEffectName.c_str());

        bool useCache = CompileCacheCheck(pPipelineEffect);

        D3D12_GRAPHICS_PIPELINE_STATE_DESC baseDesc = {};
        baseDesc.pRootSignature = pPipelineEffect->rootSignature;
        baseDesc.SampleMask = UINT_MAX;
        baseDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        baseDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        baseDesc.NumRenderTargets = 1;
        baseDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        baseDesc.SampleDesc.Count = 1;
        baseDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        baseDesc.DepthStencilState.DepthEnable = FALSE;
        baseDesc.DepthStencilState.StencilEnable = FALSE;

        baseDesc.InputLayout = pPipelineEffect->GetGraphicsShaderSet().inputLayout;
        baseDesc.VS = pPipelineEffect->GetGraphicsShaderSet().VS;
        baseDesc.PS = pPipelineEffect->GetGraphicsShaderSet().PS;
        baseDesc.DS = pPipelineEffect->GetGraphicsShaderSet().DS;
        baseDesc.HS = pPipelineEffect->GetGraphicsShaderSet().HS;
        baseDesc.GS = pPipelineEffect->GetGraphicsShaderSet().GS;

        if (useCache && (pPipelineEffect->pipelineLibrary->GetPSOCachingMechanism() == PSOCachingMechanism::PipelineLibraries))
        {
            ID3D12PipelineLibrary *pPipelineLibrary = pPipelineEffect->pipelineLibrary->m_PipelineLibrary.GetPipelineLibrary().Get();
            HRESULT hr = pPipelineLibrary->LoadGraphicsPipeline(wPipelienEffectName.c_str(), &baseDesc, IID_PPV_ARGS(&pPipelineEffect->pipelineState));
            if (E_INVALIDARG == hr)
            {
                RXN_LOGGER::Trace(L"A pipeline state object' with the specified name '%s' does not exist. Creating new pipeline state...", wPipelienEffectName.c_str());
                ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pPipelineEffect->pipelineState)));

                hr = pPipelineLibrary->StorePipeline(wPipelienEffectName.c_str(), pPipelineEffect->pipelineState.Get());
                if (E_INVALIDARG == hr)
                {
                    RXN_LOGGER::Error(L"A PSO with the specified name '%s' already exists in the library.", wPipelienEffectName.c_str());
                    throw PipelineLibraryException("A PSO with the specified name already exists in the library.");
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
        else if (useCache && (pPipelineEffect->pipelineLibrary->GetPSOCachingMechanism() == PSOCachingMechanism::CachedBlobs))
        {
            assert(pPipelineEffect->diskCache.IsMapped());
            size_t size = pPipelineEffect->diskCache.GetSize();
            if (size == 0)
            {
                RXN_LOGGER::Trace(L"File size is 0... The disk cache needs to be refreshed...");
                ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pPipelineEffect->pipelineState)));

                ComPointer<ID3DBlob> blob;
                pPipelineEffect->pipelineState->GetCachedBlob(&blob);
                pPipelineEffect->diskCache.Update(blob.Get());
            }
            else
            {
                RXN_LOGGER::Trace(L"Reading the blob data from disk instead of re-compiling shaders!");
                baseDesc.CachedPSO.pCachedBlob = pPipelineEffect->diskCache.GetData();
                baseDesc.CachedPSO.CachedBlobSizeInBytes = pPipelineEffect->diskCache.GetSize();

                HRESULT hr = RenderContext::GetGraphicsDevice()->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pPipelineEffect->pipelineState));


                if (FAILED(hr))
                {
                    RXN_LOGGER::Error(L"Compilation failed.... The cache is probably stale. (old drivers etc.)");
                    baseDesc.CachedPSO = {};
                    ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pPipelineEffect->pipelineState)));

                    ComPointer<ID3DBlob> blob;
                    pPipelineEffect->pipelineState->GetCachedBlob(&blob);
                    pPipelineEffect->diskCache.Update(blob.Get());
                }
            }
        }
        else
        {
            ThrowIfFailed(RenderContext::GetGraphicsDevice()->CreateGraphicsPipelineState(&baseDesc, IID_PPV_ARGS(&pPipelineEffect->pipelineState)));
        }

        SetName(pPipelineEffect->pipelineState.Get(), wPipelienEffectName.c_str());
        SetPipelineCompileFlagSafe(pPipelineEffect, true);
        SetPipelineInFlightFlagSafe(pPipelineEffect, false);
    }

    bool PipelineLibrary::CompileCacheCheck(PipelineEffectTemplate *pPipelineEffect)
    {
        
        auto lock = std::scoped_lock(pPipelineEffect->pipelineLibrary->m_FlagsMutex);
        return pPipelineEffect->pipelineLibrary->m_UseDiskLibraries;
        
    }

    bool PipelineLibrary::GetPipelineCompileFlagSafe(PipelineEffectTemplate *pPipelineEffect)
    {
        auto lock = std::scoped_lock(pPipelineEffect->pipelineLibrary->m_FlagsMutex);
        return pPipelineEffect->compileFlag;
    }

    bool PipelineLibrary::GetPipelineInFlightFlagSafe(PipelineEffectTemplate *pPipelineEffect)
    {
        auto lock = std::scoped_lock(pPipelineEffect->pipelineLibrary->m_FlagsMutex);
        return pPipelineEffect->flightFlag;
    }

    void PipelineLibrary::SetPipelineInFlightFlagSafe(PipelineEffectTemplate *pPipelineEffect, bool flag)
    {
        auto lock = std::scoped_lock(pPipelineEffect->pipelineLibrary->m_FlagsMutex);
        pPipelineEffect->flightFlag = flag;
    }

    void PipelineLibrary::SetPipelineCompileFlagSafe(PipelineEffectTemplate *pPipelineEffect, bool flag)
    {
        auto lock = std::scoped_lock(pPipelineEffect->pipelineLibrary->m_FlagsMutex);
        pPipelineEffect->compileFlag = flag;
    }

    void PipelineLibrary::EndFrame()
    {
        m_DrawIndex = 0;
    }

    void PipelineLibrary::ClearPSOCache()
    {
        RXN_LOGGER::Debug(L"Clearing pipeline caches...");
        WaitForThreads();
        for (auto &effect : m_PipelineEffects)
        {
            if (!effect.IsBaseEffect() && effect.pipelineLibrary)
            {
                effect.pipelineState.Release();
                effect.compileFlag = false;
                effect.flightFlag = false;
            }

            effect.diskCache.DestroyFile(true);
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
            using enum Rxn::Graphics::Mapped::PSOCachingMechanism;
            auto lock = std::scoped_lock(m_FlagsMutex);

            uint32 newMechanism = static_cast<uint32>(m_PipelineStateObjectCachingMechanism) + 1;
            newMechanism = newMechanism % static_cast<uint32>(PSOCachingMechanismCount);

            
            if (!m_PipelineLibrariesSupported && (newMechanism == static_cast<uint32>(PipelineLibraries)))
            {
                newMechanism++;
                newMechanism = newMechanism % static_cast<uint32>(PSOCachingMechanismCount);
            }

            m_PipelineStateObjectCachingMechanism = static_cast<PSOCachingMechanism>(newMechanism);
        }

        WaitForThreads();
    }

    void PipelineLibrary::DestroyPipelineEffect(uint32 pipelineIndex)
    {
        WaitForThreads();

        auto &pipelineEffect = m_PipelineEffects.at(pipelineIndex);

        if (pipelineEffect.pipelineLibrary)
        {
            pipelineEffect.pipelineState.Release();
            pipelineEffect.compileFlag = false;
            pipelineEffect.flightFlag = false;
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


