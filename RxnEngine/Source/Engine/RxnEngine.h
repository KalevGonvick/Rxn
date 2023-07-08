#pragma once
#include <memory>

namespace Rxn::Engine
{
    class RXN_ENGINE_API EngineSettings;
    class RXN_ENGINE_API RxnEngine;
    namespace Runtime
    {
        enum class EngineRuntimeMode
        {
            NONE        = 0,
            DEBUG       = 1,
            PRODUCTION  = 2,
            EDITOR      = 3,
            HEADLESS    = 4
        };

        extern RxnEngine g_RxnEngine;
        
        void RXN_ENGINE_API SetMode(EngineRuntimeMode mode);
        
        EngineRuntimeMode RXN_ENGINE_API GetEngineMode();
        
        std::wstring RXN_ENGINE_API GetEngineModeString();

    }

    class RXN_ENGINE_API RxnEngine
    {
    public:

        RxnEngine();
        ~RxnEngine();

    public:
        void InitializeEngineSettings(std::shared_ptr<EngineSettings> settings);
        void SetMode(Runtime::EngineRuntimeMode mode);
        
        Runtime::EngineRuntimeMode GetMode();
        std::shared_ptr<EngineSettings> GetSettings();

        

    private:
        Runtime::EngineRuntimeMode m_uEngineMode;

        std::shared_ptr<EngineSettings> m_pEngineSettings;
    };

    

}
