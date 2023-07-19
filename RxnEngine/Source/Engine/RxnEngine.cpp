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
        m_uEngineMode = mode;
    }

    EngineRuntimeMode RxnEngine::GetMode()
    {
        return m_uEngineMode;
    }

    std::shared_ptr<EngineSettings> RxnEngine::GetSettings()
    {
        return m_pEngineSettings;
    }

    void RxnEngine::InitializeEngineSettings(std::shared_ptr<EngineSettings> settings)
    {
        m_pEngineSettings = settings;
    }
}

