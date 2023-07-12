#include "Rxn.h"
#include "Engine/Runtime.h"

#include "Engine/SplashScreen.h"
#include "Platform/WIN32/SimulationWindow.h"


namespace Rxn::Engine
{
    Runtime::Runtime()
    {
    }

    Runtime::~Runtime() = default;

    void Runtime::PreInitialize()
    {
        SetupLogger();
        SetupPlatformGUI();
    }

    void Runtime::SetMode(EngineRuntimeMode mode)
    {
        return m_Engine.SetMode(mode);
    }

    const EngineRuntimeMode Runtime::GetEngineMode()
    {
        return m_Engine.GetMode();
    }

    const WString Runtime::GetEngineModeString()
    {
        switch (m_Engine.GetMode())
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

    const void Runtime::SetupPlatformGUI()
    {
        RXN_LOGGER::Info(L"Creating window classes");

        auto splash = std::make_shared<SplashWindow>(Constants::Win32::SPLASHSCREENWINDOWKEY, Constants::Win32::SPLASHSCREENWINDOWKEY);
        splash->m_WindowStyle = Platform::Win32::WindowStyle::POPUP;
        splash->m_Size = SIZE(500, 600);
        m_WindowManager.AddWindow(splash);

        auto childsim = std::make_shared<Platform::Win32::SimulationWindow>(L"TestChildWindow", L"TestChildWindow");
        childsim->m_Size = SIZE(200, 100);

        auto mainsim = std::make_shared<Platform::Win32::SimulationWindow>(Constants::Win32::RENDERVIEWWINDOWKEY, Constants::Win32::RENDERVIEWWINDOWKEY);
        mainsim->m_Size = SIZE(1280, 720);
        mainsim->AddChildComponent(childsim);

        m_WindowManager.AddWindow(mainsim);

        for (auto &window : m_WindowManager.GetManagedWindows())
        {
            window.second->RegisterComponentClass();
            window.second->Initialize();
        }
    }


} // Rxn::Engine