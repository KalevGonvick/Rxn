#pragma once
#include "EngineSettings.h"

namespace Rxn::Engine
{
    class RXN_ENGINE_API EngineContext
    {
    public:

        EngineContext();
        ~EngineContext();

    public:

        static EngineSettings& GetEngineSettings();

    private:
        
        EngineSettings m_EngineSettings;

    };
}