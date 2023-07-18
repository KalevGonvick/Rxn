#pragma once
#include <memory>
#include "BlankProject.h"
#include "Engine/Runtime.h"
#include "Engine/SplashScreen.h"
#include "Platform/WIN32/WinEntry.h"
#include "Platform/WIN32/Window.h"
#include "Graphics/SimulationWindow.h"
#include "Common/CommandLine.h"

class BlankProject : public Rxn::Engine::Runtime
{
public:
    BlankProject() {};

    ~BlankProject() {};

public:
    void ConfigureEngine();
    void Initialize();
    void OnDestroy();
    void Update();
};

ENTRYAPP(BlankProject)


void BlankProject::ConfigureEngine()
{
    RXN_CONTEXT::GetEngineSettings().SetGameName(L"Blank Project");
    RXN_CONTEXT::GetEngineSettings().SetGameShortName(L"BP");
    RXN_CONTEXT::GetEngineSettings().SetMainIcon(L"");
    RXN_CONTEXT::GetEngineSettings().SetLogLevel(Rxn::Common::LogLevel::RXN_TRACE);
    RXN_CONTEXT::GetEngineSettings().SetLogWriteToConsole(true);
    RXN_CONTEXT::GetEngineSettings().SetLogWriteToFile(true);
    RXN_CONTEXT::GetEngineSettings().SetSplashScreenURL(L"Content\\Images\\splashscreen.bmp");
    RXN_CONTEXT::GetEngineSettings().SetBootTime(Rxn::Common::Time::GetTime(true).c_str());

    RXN_LOGGER::SetLogLevel(RXN_CONTEXT::GetEngineSettings().GetLogLevel());
    Rxn::Common::Cmd::ReadCommandLineArguments();

}

void BlankProject::Initialize()
{
    RXN_LOGGER::Info(L"Creating window classes");

    auto splash = std::make_shared<Rxn::Engine::SplashWindow>(Rxn::Constants::Win32::SPLASH_SCREEN_WINDOW_KEY, Rxn::Constants::Win32::SPLASH_SCREEN_WINDOW_KEY);
    splash->SetupWindow();
    m_WindowManager->AddWindow(splash);

    auto mainsim = std::make_shared<Rxn::Graphics::SimulationWindow>(Rxn::Constants::Win32::RENDER_VIEW_WINDOW_KEY, Rxn::Constants::Win32::RENDER_VIEW_WINDOW_KEY, 1280, 720);
    mainsim->SetupWindow();
    m_WindowManager->AddWindow(mainsim);
}



void BlankProject::Update()
{
    auto win = std::dynamic_pointer_cast<Rxn::Graphics::SimulationWindow>(m_WindowManager->m_ManagedWindows.at(Rxn::Constants::Win32::RENDER_VIEW_WINDOW_KEY));
    win->DX12_Update();
    win->DX12_Render();

}

void BlankProject::OnDestroy()
{
}
