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

        static void Tick(Core::StepTimer::LPUPDATEFUNC func);

        static uint64 GetElapsedTicks();
        static float64 GetElapsedSeconds();
        static uint64 GetTotalTicks();
        static float64 GetTotalSeconds();
        static uint32 GetFrameCount();
        static uint32 GetFramesPerSecond();
        static void SetFixedTimeStep(bool isFixedTimestep);

    private:

        EngineSettings m_EngineSettings;
        Core::StepTimer m_Timer;

    };
}