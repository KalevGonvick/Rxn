#pragma once
#include "EngineSettings.h"
#include "Core/StepTimer.h"

namespace Rxn::Engine
{
    class EngineContext
    {
    public:

        EngineContext();
        ~EngineContext();

    public:

        RXN_ENGINE_API static EngineSettings &GetEngineSettings();
        RXN_ENGINE_API static Core::StepTimer &GetTimer();

    private:

        EngineSettings m_EngineSettings;
        Core::StepTimer m_Timer;

    };
}