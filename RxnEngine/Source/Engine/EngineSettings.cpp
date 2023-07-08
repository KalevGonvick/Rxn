#include "Rxn.h"

namespace Rxn::Engine
{

    EngineSettings::EngineSettings()
        : m_wcharBootTime(L"undefined")
        , m_wcharGameName(L"undefined")
        , m_wcharGameShortName(L"undefined")
        , m_LogLevel(Common::LogLevel::RXN_TRACE)
        , m_bLogWriteToConsole(true)
        , m_bLogWriteToFile(true)
    {}

    EngineSettings::~EngineSettings() = default;

    void EngineSettings::SetGameName(const wchar_t *name)
    {
        this->m_wcharGameName = name;
    }

    void EngineSettings::SetGameShortName(const wchar_t *shortName)
    {
        this->m_wcharGameShortName = shortName;
    }

    void EngineSettings::SetGameVersion(const wchar_t *gameVersion)
    {
        this->m_wcharGameVersion = gameVersion;
    }

    void EngineSettings::SetBootTime(const wchar_t *bootTime)
    {
        this->m_wcharBootTime = bootTime;
    }

    void EngineSettings::SetMainIcon(const wchar_t *iconUrl)
    {
        this->m_wcharMainIconURL = iconUrl;
    }

    void EngineSettings::SetLogWriteToConsole(const bool &logToConsole)
    {
        this->m_bLogWriteToConsole = logToConsole;
    }

    void EngineSettings::SetLogWriteToFile(const bool &logToFile)
    {
        this->m_bLogWriteToFile = logToFile;
    }

    void EngineSettings::SetSplashScreenURL(const wchar_t *splashUrl)
    {
        this->m_wcharSplashScreenURL = splashUrl;
    }

    void EngineSettings::SetLogLevel(const Common::LogLevel &level)
    {
        this->m_LogLevel = level;
    }


    /* - Getters - */
    const wchar_t *EngineSettings::GetGameName()
    {
        return this->m_wcharGameName.c_str();
    }

    const wchar_t *EngineSettings::GetGameShortName()
    {
        return this->m_wcharGameShortName.c_str();
    }

    const wchar_t *EngineSettings::GetGameVersion()
    {
        return this->m_wcharGameVersion.c_str();
    }

    const wchar_t *EngineSettings::GetBootTime()
    {
        return this->m_wcharBootTime.c_str();
    }

    const Common::LogLevel EngineSettings::GetLogLevel()
    {
        return this->m_LogLevel;
    }

    const bool &EngineSettings::GetLogWriteToConsole()
    {
        return this->m_bLogWriteToConsole;
    }

    const bool &EngineSettings::GetLogWriteToFile()
    {
        return this->m_bLogWriteToFile;
    }

    const wchar_t *EngineSettings::GetSplashScreenURL()
    {
        return this->m_wcharSplashScreenURL.c_str();
    }

    const wchar_t *EngineSettings::GetMainIconURL()
    {
        return this->m_wcharMainIconURL.c_str();
    }

} // Core
