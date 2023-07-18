#include "Rxn.h"
#include "EngineContext.h"

namespace Rxn::Engine
{
    static EngineContext &GetContext()
    {
        static EngineContext inst;
        return inst;
    }

    EngineContext::EngineContext()
        : m_EngineSettings()
    {
    }

    EngineContext::~EngineContext() = default;

    EngineSettings &EngineContext::GetEngineSettings()
    {
        return GetContext().m_EngineSettings;
    }

    void EngineContext::Tick(Core::StepTimer::LPUPDATEFUNC func)
    {
        GetContext().m_Timer.Tick(func);
    }

    uint64 EngineContext::GetElapsedTicks()
    {
        return GetContext().m_Timer.GetElapsedSeconds();
    }

    float64 EngineContext::GetElapsedSeconds()
    {
        return GetContext().m_Timer.GetElapsedSeconds();
    }

    uint64 EngineContext::GetTotalTicks()
    {
        return GetContext().m_Timer.GetTotalTicks();
    }

    float64 EngineContext::GetTotalSeconds()
    {
        return GetContext().m_Timer.GetTotalSeconds();
    }

    uint32 EngineContext::GetFrameCount()
    {
        return GetContext().m_Timer.GetFrameCount();
    }

    uint32 EngineContext::GetFramesPerSecond()
    {
        return GetContext().m_Timer.GetFramesPerSecond();
    }

    void EngineContext::SetFixedTimeStep(bool isFixedTimestep)
    {
        GetContext().m_Timer.SetFixedTimeStep(isFixedTimestep);
    }


}