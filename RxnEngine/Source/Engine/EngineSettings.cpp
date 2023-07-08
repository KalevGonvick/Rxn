#include "Rxn.h"

namespace Rxn::Engine
{
	
	EngineSettings::EngineSettings()
		: m_wBootTime(L"undefined")
		, m_wGameName(L"undefined")
		, m_wLogLevel(L"undefined")
		, m_wGameShortName(L"undefined")
		, m_wLogWriteToConsole(L"undefined")
		, m_wLogWriteToFile(L"undefined")
	{}

	EngineSettings::~EngineSettings()
	{}

	/* - Setters - */
	void EngineSettings::SetGameName(UINT id)
	{
		LoadString(HInstance(), id, this->m_wGameName, Constants::kusMaxNameString);
	}

	void EngineSettings::SetGameShortName(UINT id)
	{
		LoadString(HInstance(), id, this->m_wGameShortName, Constants::kusMaxNameString);
	}

	void EngineSettings::SetGameVersion(UINT id)
	{
		LoadString(HInstance(), id, this->m_wGameVersion, Constants::kusMaxNameString);
	}

	void EngineSettings::SetBootTime(const wchar_t * time)
	{
		wcscpy_s(this->m_wBootTime, time);
	}

	void EngineSettings::SetMainIcon(UINT id)
	{
		LoadIcon(HInstance(), MAKEINTRESOURCE(id));
	}

	void EngineSettings::SetLogLevel(UINT id)
	{
		LoadString(HInstance(), id, this->m_wLogLevel, Constants::kusMaxNameString);
	}

	void EngineSettings::SetLogWriteToConsole(UINT id)
	{
		LoadString(HInstance(), id, this->m_wLogWriteToConsole, Constants::kusMaxNameString);
	}

	void EngineSettings::SetLogWriteToFile(UINT id)
	{
		LoadString(HInstance(), id, this->m_wLogWriteToFile, Constants::kusMaxNameString);
	}

	void EngineSettings::SetSplashScreenURL(UINT id)
	{
		LoadString(HInstance(), id, this->m_wSplashScreenURL, Constants::kusMaxNameString);
	}

	/* - Getters - */
	wchar_t* EngineSettings::GetGameName()
	{
		return this->m_wGameName;
	}

	wchar_t* EngineSettings::GetGameShortName()
	{
		return this->m_wGameShortName;
	}

	wchar_t* EngineSettings::GetGameVersion()
	{
		return this->m_wGameVersion;
	}

	wchar_t* EngineSettings::GetBootTime()
	{
		return this->m_wBootTime;
	}

	wchar_t* EngineSettings::GetLogLevel()
	{
		return this->m_wLogLevel;
	}

	wchar_t* EngineSettings::GetLogWriteToConsole()
	{
		return this->m_wLogWriteToConsole;
	}

	wchar_t* EngineSettings::GetLogWriteToFile()
	{
		return this->m_wLogWriteToFile;
	}

	wchar_t* EngineSettings::GetSplashScreenURL()
	{
		return this->m_wSplashScreenURL;
	}

} // Core
