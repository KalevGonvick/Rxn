#pragma once
#include <memory>
#include "BlankProject.h"
#include "Engine/Runtime.h"
#include "Platform/WIN32/WinEntry.h"

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
}

void BlankProject::OnDestroy()
{
}

void BlankProject::Update()
{
}
