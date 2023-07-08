#include "Rxn.h"

namespace Rxn::Engine
{
    namespace Runtime
    {
        RxnEngine g_RxnEngine;

        void SetMode(EngineRuntimeMode mode)
        {
            return g_RxnEngine.SetMode(mode);
        }

        EngineRuntimeMode GetEngineMode()
        {
            return g_RxnEngine.GetMode();
        }

        std::wstring RXN_ENGINE_API GetEngineModeString()
        {
            switch (g_RxnEngine.GetMode())
            {
            case EngineRuntimeMode::DEBUG:
                return L"Debug";
            case EngineRuntimeMode::EDITOR:
                return L"Editor";
            case EngineRuntimeMode::HEADLESS:
                return L"Server";
            case EngineRuntimeMode::PRODUCTION:
                return L"Release";
            case EngineRuntimeMode::NONE:
                return L"None";
            default:
                return L"Unknown";
            }
        }

    }
    

    RxnEngine::RxnEngine()
        : m_uEngineMode(Runtime::EngineRuntimeMode::NONE)
    {
    }

    RxnEngine::~RxnEngine()
    {
    }
    
    void RxnEngine::SetMode(Runtime::EngineRuntimeMode mode)
    {
        this->m_uEngineMode = mode;
    }
    
    Runtime::EngineRuntimeMode RxnEngine::GetMode()
    {
        return this->m_uEngineMode;
    }

    std::shared_ptr<EngineSettings> RxnEngine::GetSettings()
    {
        return this->m_pEngineSettings;
    }

    void RxnEngine::InitializeEngineSettings(std::shared_ptr<EngineSettings> settings)
    {
        this->m_pEngineSettings = settings;
    }
}

