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
	Rxn::Engine::EngineContext::GetEngineSettings().SetGameName(IDS_NAME);
	Rxn::Engine::EngineContext::GetEngineSettings().SetGameShortName(IDS_SHORTNAME);
	Rxn::Engine::EngineContext::GetEngineSettings().SetMainIcon(IDI_MAINICON);
	Rxn::Engine::EngineContext::GetEngineSettings().SetLogLevel(IDS_LOGLEVEL);
	Rxn::Engine::EngineContext::GetEngineSettings().SetLogWriteToConsole(IDS_LOGWRITETOCONSOLE);
	Rxn::Engine::EngineContext::GetEngineSettings().SetLogWriteToFile(IDS_LOGWRITETOFILE);
	Rxn::Engine::EngineContext::GetEngineSettings().SetSplashScreenURL(IDS_SPLASHSCREENURL);
	Rxn::Engine::EngineContext::GetEngineSettings().SetBootTime(Rxn::Common::Time::GetTime(true).c_str());
}