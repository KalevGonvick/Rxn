#pragma once
#include <memory>

namespace Rxn::Engine
{
    class RXN_ENGINE_API EngineSettings;
    class RXN_ENGINE_API RxnEngine;

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
        void InitializeEngineSettings(std::shared_ptr<EngineSettings> settings);
        void SetMode(EngineRuntimeMode mode);

        EngineRuntimeMode GetMode();
        std::shared_ptr<EngineSettings> GetSettings();



    private:
        EngineRuntimeMode m_uEngineMode;

        std::shared_ptr<EngineSettings> m_pEngineSettings;
    };



}
