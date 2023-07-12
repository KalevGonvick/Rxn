#include "Rxn.h"

namespace Rxn::Engine
{

    RxnEngine::RxnEngine()
        : m_uEngineMode(EngineRuntimeMode::NONE)
    {
    }

    RxnEngine::~RxnEngine()
    {
    }

    void RxnEngine::SetMode(EngineRuntimeMode mode)
    {
        this->m_uEngineMode = mode;
    }

    EngineRuntimeMode RxnEngine::GetMode()
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

