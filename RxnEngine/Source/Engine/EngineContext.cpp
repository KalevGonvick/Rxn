#include "Rxn.h"
#include "EngineContext.h"

namespace Rxn::Engine
{
    inline EngineContext g_EngineContext;

    EngineContext::EngineContext() = default;
    EngineContext::~EngineContext() = default;

    EngineSettings &EngineContext::GetEngineSettings()
    {
        return g_EngineContext.m_EngineSettings;
    }

    Core::StepTimer &EngineContext::GetTimer()
    {
        return g_EngineContext.m_Timer;
    }


}