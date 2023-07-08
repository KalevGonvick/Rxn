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
        void SetGameName(const wchar_t *name);
        void SetGameShortName(const wchar_t *shortName);
        void SetGameVersion(const wchar_t *gameVersion);
        void SetBootTime(const wchar_t *bootTime);
        void SetMainIcon(const wchar_t *iconUrl);
        void SetSplashScreenURL(const wchar_t *id);

        void SetLogWriteToConsole(const bool &id);
        void SetLogWriteToFile(const bool &id);

        void SetLogLevel(const Common::LogLevel &level);

        /* - Getters - */
        const wchar_t *GetGameName();
        const wchar_t *GetGameShortName();
        const wchar_t *GetGameVersion();
        const wchar_t *GetBootTime();
        const wchar_t *GetSplashScreenURL();
        const wchar_t *GetMainIconURL();

        const bool &GetLogWriteToConsole();
        const bool &GetLogWriteToFile();

        const Common::LogLevel GetLogLevel();

    private:

        /* - Settings - */
        WString m_wcharGameName;
        WString m_wcharGameVersion;
        WString m_wcharBootTime;
        WString m_wcharGameShortName;

        WString m_wcharSplashScreenURL;
        WString m_wcharMainIconURL;

        bool m_bLogWriteToConsole;
        bool m_bLogWriteToFile;

        Common::LogLevel m_LogLevel;


    };

} // RxnEngine::Core