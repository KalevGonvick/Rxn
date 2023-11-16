#pragma once
#include "EngineSettings.h"
#include "Core/StepTimer.h"

namespace Rxn::Engine
{
    class RXN_ENGINE_API EngineContext
    {
    public:

        EngineContext();
        ~EngineContext();

    public:

        static EngineSettings &GetEngineSettings();
        static Core::StepTimer &GetTimer();

    private:

        EngineSettings m_EngineSettings;
        Core::StepTimer m_Timer;

    };
}