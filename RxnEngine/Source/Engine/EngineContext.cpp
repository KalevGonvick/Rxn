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

    void EngineContext::Tick()
    {
        g_EngineContext.m_Timer.Tick(nullptr);
    }

    float64 EngineContext::GetElapsedTicks()
    {
        return g_EngineContext.m_Timer.GetElapsedSeconds();
    }

    float64 EngineContext::GetElapsedSeconds()
    {
        return g_EngineContext.m_Timer.GetElapsedSeconds();
    }

    uint64 EngineContext::GetTotalTicks()
    {
        return g_EngineContext.m_Timer.GetTotalTicks();
    }

    float64 EngineContext::GetTotalSeconds()
    {
        return g_EngineContext.m_Timer.GetTotalSeconds();
    }

    uint64 EngineContext::GetFrameCount()
    {
        return g_EngineContext.m_Timer.GetFrameCount();
    }

    uint64 EngineContext::GetFramesPerSecond()
    {
        return g_EngineContext.m_Timer.GetFramesPerSecond();
    }

    void EngineContext::SetFixedTimeStep(bool isFixedTimestep)
    {
        g_EngineContext.m_Timer.SetFixedTimeStep(isFixedTimestep);
    }

    void EngineContext::SetTargetElapsedTicks(uint64 tickrate)
    {
        g_EngineContext.m_Timer.SetTargetElapsedTicks(tickrate);
    }


}