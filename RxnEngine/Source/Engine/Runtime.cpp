#include "Rxn.h"
#include "Engine/Runtime.h"

#include "Engine/SplashScreen.h"
#include "Graphics/SimulationWindow.h"


namespace Rxn::Engine
{
    Runtime::Runtime()
    {
    }

    Runtime::~Runtime() = default;

    void Runtime::PreInitialize()
    {
        this->m_Engine = std::make_shared<RxnEngine>();
        this->m_WindowManager = std::make_shared<Platform::Win32::WindowManager>();
        SetupLogger();
        SetupPlatformGUI();
        InitGraphics();
    }

    void Runtime::SetMode(EngineRuntimeMode mode)
    {
        return m_Engine->SetMode(mode);
    }

    const EngineRuntimeMode Runtime::GetEngineMode()
    {
        return m_Engine->GetMode();
    }

    const WString Runtime::GetEngineModeString()
    {
        switch (m_Engine->GetMode())
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

        auto splash = std::make_shared<SplashWindow>(Constants::Win32::SPLASH_SCREEN_WINDOW_KEY, Constants::Win32::SPLASH_SCREEN_WINDOW_KEY);
        splash->SetupWindow();
        m_WindowManager->AddWindow(splash);

        auto mainsim = std::make_shared<Graphics::SimulationWindow>(Constants::Win32::RENDER_VIEW_WINDOW_KEY, Constants::Win32::RENDER_VIEW_WINDOW_KEY, 1280, 720);
        mainsim->SetupWindow();
        m_WindowManager->AddWindow(mainsim);

        //for (auto &window : m_WindowManager->m_ManagedWindows)
        //{
        //    window.second->RegisterComponentClass();
        //    window.second->Initialize();
        //}


    }

    const void Runtime::InitGraphics()
    {
        //std::shared_ptr<Platform::Win32::Window> registeredSim = m_WindowManager->m_ManagedWindows.at(Constants::Win32::RENDER_VIEW_WINDOW_KEY);

        //m_RenderManager = std::make_shared<Graphics::RenderManager>(registeredSim->m_Size.cx, registeredSim->m_Size.cy, registeredSim->m_pHWnd);
        //m_RenderManager->OnInit();

    }


} // Rxn::Engine