#pragma once

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
        const wchar_t *GetGameName() const;
        const wchar_t *GetGameShortName() const;
        const wchar_t *GetGameVersion() const;
        const wchar_t *GetBootTime() const;
        const wchar_t *GetSplashScreenURL() const;
        const wchar_t *GetMainIconURL() const;

        bool GetLogWriteToConsole() const;
        bool GetLogWriteToFile() const;

        Common::LogLevel GetLogLevel() const;

    private:

        /* - Settings - */
        WString m_wcharGameName = L"undefined";
        WString m_wcharGameVersion;
        WString m_wcharBootTime = L"undefined";
        WString m_wcharGameShortName = L"undefined";

        WString m_wcharSplashScreenURL;
        WString m_wcharMainIconURL;

        bool m_bLogWriteToConsole = true;
        bool m_bLogWriteToFile = true;

        Common::LogLevel m_LogLevel = Common::LogLevel::RXN_TRACE;


    };

} // RxnEngine::Core