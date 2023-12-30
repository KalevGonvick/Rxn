#pragma once

namespace Rxn::Engine
{

    class EngineSettings
    {
    public:

        EngineSettings();
        ~EngineSettings();

    public:

        /* - Setters - */
        RXN_ENGINE_API void SetGameName(const wchar_t *name);
        RXN_ENGINE_API void SetGameShortName(const wchar_t *shortName);
        RXN_ENGINE_API void SetGameVersion(const wchar_t *gameVersion);
        RXN_ENGINE_API void SetBootTime(const wchar_t *bootTime);
        RXN_ENGINE_API void SetMainIcon(const wchar_t *iconUrl);
        RXN_ENGINE_API void SetSplashScreenURL(const wchar_t *id);

        RXN_ENGINE_API void SetLogWriteToConsole(const bool &id);
        RXN_ENGINE_API void SetLogWriteToFile(const bool &id);

        RXN_ENGINE_API void SetLogLevel(const Common::LogLevel &level);

        /* - Getters - */
        RXN_ENGINE_API const wchar_t *GetGameName() const;
        RXN_ENGINE_API const wchar_t *GetGameShortName() const;
        RXN_ENGINE_API const wchar_t *GetGameVersion() const;
        RXN_ENGINE_API const wchar_t *GetBootTime() const;
        RXN_ENGINE_API const wchar_t *GetSplashScreenURL() const;
        RXN_ENGINE_API const wchar_t *GetMainIconURL() const;

        RXN_ENGINE_API bool GetLogWriteToConsole() const;
        RXN_ENGINE_API bool GetLogWriteToFile() const;

        RXN_ENGINE_API Common::LogLevel GetLogLevel() const;

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