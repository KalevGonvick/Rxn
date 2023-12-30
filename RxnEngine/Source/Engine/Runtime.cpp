#include "Rxn.h"
#include "Engine/Runtime.h"

#include "Graphics/SplashScreen.h"
#include "Graphics/RenderWindow.h"


namespace Rxn::Engine
{
    Runtime::Runtime() = default;
    Runtime::~Runtime() = default;

    void Runtime::InitializeEngineSystems()
    {


        SetupLogger();
    }

    void Runtime::SetMode(EngineRuntimeMode mode)
    {
        return GetEngine().SetMode(mode);
    }

    EngineRuntimeMode Runtime::GetEngineMode()
    {
        return GetEngine().GetMode();
    }

    WString Runtime::GetEngineModeString()
    {
        switch (GetEngine().GetMode())
        {
        case EngineRuntimeMode::DEBUG:
            return L"Debug";
        case EngineRuntimeMode::EDITOR:
            return L"Editor";
        case EngineRuntimeMode::HEADLESS:
            return L"Server";
        case EngineRuntimeMode::PRODUCTION:
            return L"Release";
        case EngineRuntimeMode::NONE:
            return L"None";
        default:
            return L"Unknown";
        }
    }

    const void Runtime::SetupLogger()
    {
        RXN_LOGGER::SetLogLevel(EngineContext::GetEngineSettings().GetLogLevel());
        RXN_LOGGER::GetLogDirectory();
        RXN_LOGGER::PrintLnHeader(L"Engine Config");
        RXN_LOGGER::Info(L"Application Starting...");
        RXN_LOGGER::Info(L"Log Dir: %s", RXN_LOGGER::GetLogDirectory().c_str());
        RXN_LOGGER::Info(L"Game Name: %s", Engine::EngineContext::GetEngineSettings().GetGameName());
        RXN_LOGGER::Info(L"Boot Time: %s", Common::Time::GetDateTimeString().c_str());
        RXN_LOGGER::Info(L"Mode: %s", Engine::Runtime::GetEngineModeString().c_str());
        RXN_LOGGER::PrintLnSeperator();

        //Core::RxnBinaryHandler::ReadRxnFile();

        RXN_LOGGER::Info(L"Loaded up... %s", Engine::EngineContext::GetEngineSettings().GetGameName());
    }

    RxnEngine &Runtime::GetEngine()
    {
        return m_Engine;
    }

    Platform::Win32::WindowManager &Runtime::GetWindowManager()
    {
        return m_WindowManager;
    }




} // Rxn::Engine