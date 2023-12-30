#pragma once
#include "EngineSettings.h"

namespace Rxn::Engine
{

    enum class EngineRuntimeMode
    {
        NONE = 0,
        DEBUG = 1,
        PRODUCTION = 2,
        EDITOR = 3,
        HEADLESS = 4
    };

    class RXN_ENGINE_API RxnEngine
    {
    public:

        RxnEngine();
        ~RxnEngine();

    public:

        void InitializeEngineSettings(const EngineSettings &settings);
        void SetMode(EngineRuntimeMode mode);

        EngineRuntimeMode GetMode() const;
        EngineSettings &GetSettings();


    private:

        EngineRuntimeMode m_EngineMode = EngineRuntimeMode::NONE;
        EngineSettings m_EngineSettings;
    };



}
