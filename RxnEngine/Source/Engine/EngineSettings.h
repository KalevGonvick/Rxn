#pragma once

#define ENGINE_CONFIG Core::GameSettings

namespace Rxn::Engine
{

	class RXN_ENGINE_API EngineSettings
	{
	public:

		EngineSettings();
		~EngineSettings();

	public:

		/* - Setters - */
		void SetGameName(UINT id);
		void SetGameShortName(UINT id);
		void SetGameVersion(UINT id);
		void SetBootTime(const wchar_t * time);
		void SetMainIcon(UINT id);
		void SetLogLevel(UINT id);
		void SetLogWriteToConsole(UINT id);
		void SetLogWriteToFile(UINT id);
		void SetSplashScreenURL(UINT id);

		/* - Getters - */
		wchar_t * GetGameName();
		wchar_t * GetGameShortName();
		wchar_t * GetGameVersion();
		wchar_t * GetBootTime();
		wchar_t * GetLogLevel();
		wchar_t * GetLogWriteToConsole();
		wchar_t * GetLogWriteToFile();
		wchar_t * GetSplashScreenURL();

	private:

		/* - Settings - */
		wchar_t m_wGameName[Constants::kusMaxNameString];
		wchar_t m_wGameVersion[Constants::kusMaxNameString];
		wchar_t m_wBootTime[Constants::kusMaxNameString];
		wchar_t m_wGameShortName[Constants::kusMaxNameString];
		wchar_t m_wLogLevel[Constants::kusMaxNameString];
		wchar_t m_wLogWriteToConsole[Constants::kusMaxNameString];
		wchar_t m_wLogWriteToFile[Constants::kusMaxNameString];
		wchar_t m_wSplashScreenURL[Constants::kusMaxNameString];
	};

} // RxnEngine::Core