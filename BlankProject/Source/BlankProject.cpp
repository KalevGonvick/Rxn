#pragma once
#include <memory>
#include "BlankProject.h"
#include "Engine/Simulation.h"
#include "Platform/WIN32/WinEntry.h"

class BlankProject : public Rxn::Engine::Simulation
{
public:
    BlankProject() {};

    ~BlankProject() {};

public:
    void ConfigureEngine();

    void Initialize() {};

    void Update() {};
};

ENTRYAPP(BlankProject)


void BlankProject::ConfigureEngine()
{
    Rxn::Engine::EngineContext::GetEngineSettings().SetGameName(L"Blank Project");
    Rxn::Engine::EngineContext::GetEngineSettings().SetGameShortName(L"BP");
    Rxn::Engine::EngineContext::GetEngineSettings().SetMainIcon(L"");
    Rxn::Engine::EngineContext::GetEngineSettings().SetLogLevel(Rxn::Common::LogLevel::RXN_TRACE);
    Rxn::Engine::EngineContext::GetEngineSettings().SetLogWriteToConsole(true);
    Rxn::Engine::EngineContext::GetEngineSettings().SetLogWriteToFile(true);
    Rxn::Engine::EngineContext::GetEngineSettings().SetSplashScreenURL(L"Content\\Images\\splashscreen.bmp");
    Rxn::Engine::EngineContext::GetEngineSettings().SetBootTime(Rxn::Common::Time::GetTime(true).c_str());
}