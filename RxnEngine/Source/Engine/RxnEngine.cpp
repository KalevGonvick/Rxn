#include "Rxn.h"

namespace Rxn::Engine
{

    RxnEngine::RxnEngine() = default;
    RxnEngine::~RxnEngine() = default;

    void RxnEngine::SetMode(EngineRuntimeMode mode)
    {
        m_EngineMode = mode;
    }

    EngineRuntimeMode RxnEngine::GetMode() const
    {
        return m_EngineMode;
    }

    EngineSettings &RxnEngine::GetSettings()
    {
        return m_EngineSettings;
    }

    void RxnEngine::InitializeEngineSettings(const EngineSettings &settings)
    {
        m_EngineSettings = settings;
    }
}

