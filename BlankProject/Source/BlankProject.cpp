#pragma once
#include <memory>
#include "BlankProject.h"
#include "Engine/Runtime.h"
#include "Graphics/SplashScreen.h"
#include "Platform/WIN32/WinEntry.h"
#include "Platform/WIN32/Window.h"
#include "Graphics/RenderWindow.h"
#include "Common/CommandLine.h"

class BlankProject : public Rxn::Engine::Runtime
{
public:

    BlankProject() = default;
    ~BlankProject() = default;

public:

    void SetupEngineConfigurations() override;
    void InitializeRuntime() override;
    void UpdateEngine() override;
    void OnDestroy() override;
};

ENTRYAPP(BlankProject)


void BlankProject::SetupEngineConfigurations()
{
    RXN_CONTEXT::GetEngineSettings().SetGameName(L"Blank Project");
    RXN_CONTEXT::GetEngineSettings().SetGameShortName(L"BP");
    RXN_CONTEXT::GetEngineSettings().SetMainIcon(L"");
    RXN_CONTEXT::GetEngineSettings().SetLogLevel(Rxn::Common::LogLevel::RXN_TRACE);
    RXN_CONTEXT::GetEngineSettings().SetLogWriteToConsole(true);
    RXN_CONTEXT::GetEngineSettings().SetLogWriteToFile(true);
    RXN_CONTEXT::GetEngineSettings().SetSplashScreenURL(L"Content\\Images\\splashscreen.bmp");
    RXN_CONTEXT::GetEngineSettings().SetBootTime(RXN_TIME_UTIL::GetTime(true).c_str());

    RXN_LOGGER::SetLogLevel(RXN_CONTEXT::GetEngineSettings().GetLogLevel());
    Rxn::Common::Cmd::ReadCommandLineArguments();

}

void BlankProject::InitializeRuntime()
{
    RXN_LOGGER::Info(L"Creating window classes");

    auto splash = std::make_shared<Rxn::Engine::SplashWindow>(Rxn::Constants::Win32::SPLASH_SCREEN_WINDOW_KEY, Rxn::Constants::Win32::SPLASH_SCREEN_WINDOW_KEY, 500, 600);
    splash->SetupWindow();
    m_WindowManager->AddWindow(splash);

    auto mainsim = std::make_shared<Rxn::Graphics::RenderWindow>(Rxn::Constants::Win32::RENDER_VIEW_WINDOW_KEY, Rxn::Constants::Win32::RENDER_VIEW_WINDOW_KEY, 1920, 1080);
    mainsim->SetupWindow();
    m_WindowManager->AddWindow(mainsim);

    //auto mainsim2 = std::make_shared<Rxn::Graphics::SimulationWindow>(Rxn::Constants::Win32::RENDER_VIEW_WINDOW2_KEY, Rxn::Constants::Win32::RENDER_VIEW_WINDOW2_KEY, 1920, 1080);
    //mainsim2->SetupWindow();
    //m_WindowManager->AddWindow(mainsim2);
}



void BlankProject::UpdateEngine()
{
    auto win = std::dynamic_pointer_cast<Rxn::Graphics::RenderWindow>(m_WindowManager->m_ManagedWindows.at(Rxn::Constants::Win32::RENDER_VIEW_WINDOW_KEY));
    win->UpdateSimulation();

    // Scene setup
    win->PreRenderPass();

    // present the frame
    win->RenderPass();

    //auto win2 = std::dynamic_pointer_cast<Rxn::Graphics::SimulationWindow>(m_WindowManager->m_ManagedWindows.at(Rxn::Constants::Win32::RENDER_VIEW_WINDOW2_KEY));
    //win2->UpdateSimulation();

    //// Scene setup
    //win2->PreRenderPass();

    //// present the frame
    //win2->RenderPass();

}

void BlankProject::OnDestroy()
{

}
