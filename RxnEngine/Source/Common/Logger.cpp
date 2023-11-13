#include "Rxn.h"
#include "Logger.h"
#include "LoggerImpl.h"

namespace Rxn::Common
{


    /* -------------------------------------------------------- */
    /*  LoggerImpl                                              */
    /* -------------------------------------------------------- */
#pragma region LoggerImpl

    Logger::LoggerImpl::LoggerImpl() = default;
    Logger::LoggerImpl::~LoggerImpl() = default;

    WString Logger::LoggerImpl::GetLogDirectory()
    {
        if (this->m_LogFileCreated)
        {
            return this->m_OutputLogDir;
        }

        WString path = L"";
        wchar_t *appDataLocal;

        SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataLocal);

        path.append(appDataLocal);
        path.append(L"\\");
        path.append(Engine::EngineContext::GetEngineSettings().GetGameName());
        CreateDirectory(path.c_str(), nullptr);

        path.append(L"\\Log");
        CreateDirectory(path.c_str(), nullptr);

        m_OutputLogDir = path;
        m_LogFileCreated = true;

        return path;
    }

    void Logger::LoggerImpl::SetLogLevel(LogLevel level)
    {
        this->m_Level = level;
    }

    void Logger::LoggerImpl::SetWriteToFile(bool writeToFile)
    {
        this->m_WriteToFile = writeToFile;
    }

    void Logger::LoggerImpl::SetWriteToConsole(bool writeToConsole)
    {
        this->m_WriteToConsole = writeToConsole;
    }

    void Logger::LoggerImpl::PrintLn(const LogLevel &level, const wchar_t *b)
    {
        std::wfstream outfile;
        outfile.open(WString(GetLogDirectory() + L"\\" + GetLogFileName()), std::ios_base::app);
        WString sev;

        switch (level)
        {
        case LogLevel::RXN_DEBUG:
            sev = m_DebugSeverity;
            break;

        case LogLevel::RXN_ERROR:
            sev = m_ErrorSeverity;
            break;

        case LogLevel::RXN_INFO:
            sev = m_InfoSeverity;
            break;

        case LogLevel::RXN_TRACE:
            sev = m_TraceSeverity;
            break;

        case LogLevel::RXN_WARN:
            sev = m_WarningSeverity;
            break;

        default:
            break;
        }

        if (outfile.is_open())
        {
            WString s = L"\n" + Time::GetDateTimeString() + L" " + sev + L" " + b;
            outfile << s;
            outfile.close();
            fwprintf_s(stdout, s.c_str());
        }
        else
        {
            MessageBox(nullptr, L"Unable to open log file...", L"Log Error", MB_OK);
        }
    }

    void Logger::LoggerImpl::PrintLnSeperator()
    {
        const int32_fast knMaxHeaderLineLen = 100;
        WString s = L"";
        s.append(L"\n");
        for (int32_fast x = 0; x < knMaxHeaderLineLen; x++)
        {
            s.append(L"-");
        }

        std::wfstream outfile;
        outfile.open(std::wstring(GetLogDirectory() + L"\\" + GetLogFileName()), std::ios_base::app);

        if (outfile.is_open())
        {
            outfile << s;
            outfile.close();
        }
        else
        {
            MessageBox(nullptr, L"Unable to open log file...", L"Log Error", MB_OK);
        }
    }

    void Logger::LoggerImpl::PrintLnHeader(const wchar_t *fmt)
    {
        const int32_fast knMaxHeaderLineLen = 100;
        const int32_fast knHeaderPadding = 4;

        size_t titleLen = std::char_traits<wchar_t>::length(fmt);

        size_t remainingLength = knMaxHeaderLineLen - knHeaderPadding - titleLen - knHeaderPadding;
        size_t oneSideLength = remainingLength / 2;

        WString s = L"\n";

        for (size_t x = 0; x < oneSideLength; x++)
        {
            s.append(L"-");
        }

        s.append(L"< ");
        s.append(fmt);
        s.append(L" >");

        for (int x = 0; x < oneSideLength; x++)
        {
            s.append(L"-");
        }

        std::wfstream outfile;
        outfile.open(WString(GetLogDirectory() + L"\\" + GetLogFileName()), std::ios_base::app);

        if (outfile.is_open())
        {
            outfile << s;
            outfile.close();
        }
        else
        {
            MessageBox(nullptr, L"Unable to open log file...", L"Log Error", MB_OK);
        }

    }

    bool Logger::LoggerImpl::IsMTailRunning() const
    {
        bool exists = false;
        PROCESSENTRY32 en;
        HANDLE snpsht = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (Process32First(snpsht, &en))
        {
            while (Process32Next(snpsht, &en))
            {
                if (!_wcsicmp(en.szExeFile, L"mTAIL.exe"))
                {
                    exists = true;
                }
            }
        }

        CloseHandle(snpsht);
        return exists;
    }

    bool Logger::LoggerImpl::StartMTail()
    {
        const int kMaxPathLen = 260;
        if (IsMTailRunning())
        {
            Logger::Error(L"--MTail failed to start - Already Running");
            return false;
        }

        Logger::Info(L"--Starting MTail");

        wchar_t path[kMaxPathLen] = { 0 };

        GetCurrentDirectoryW(kMaxPathLen, path);

        WString url = path + WString(L"/mTail.exe");
        WString params = L" \"" + GetLogDirectory() + L"/" + GetLogFileName() + L"\" /start";

        ShellExecute(nullptr, nullptr, url.c_str(), params.c_str(), nullptr, SW_SHOWDEFAULT);

        return true;
    }

    WString Logger::LoggerImpl::GetLogFileName() const
    {
        WString file = Engine::EngineContext::GetEngineSettings().GetGameName();
        file.append(Engine::EngineContext::GetEngineSettings().GetBootTime());
        file.append(L".log");
        return file;
    }

    bool Logger::LoggerImpl::IsInfoEnabled() const
    {
        return IsLevelEnabled(LogLevel::RXN_INFO);
    }

    bool Logger::LoggerImpl::IsWarnEnabled() const
    {
        return IsLevelEnabled(LogLevel::RXN_WARN);
    }

    bool Logger::LoggerImpl::IsErrorEnabled() const
    {
        return IsLevelEnabled(LogLevel::RXN_ERROR);
    }

    bool Logger::LoggerImpl::IsDebugEnabled() const
    {
        return IsLevelEnabled(LogLevel::RXN_DEBUG);
    }

    bool Logger::LoggerImpl::IsTraceEnabled() const
    {
        return IsLevelEnabled(LogLevel::RXN_TRACE);
    }

    bool Logger::LoggerImpl::IsLevelEnabled(LogLevel level) const
    {
        return m_Level >= level;
    }

    void Logger::LoggerImpl::Info(const wchar_t *fmt, char *args)
    {
        if (!this->IsInfoEnabled())
        {
            return;
        }

        wchar_t b[Constants::MAX_LOG_LINE_LENGTH];
        vswprintf_s(b, fmt, args);
        PrintLn(LogLevel::RXN_INFO, b);

    }

    void Logger::LoggerImpl::Debug(const wchar_t *fmt, char *args)
    {
        if (!IsDebugEnabled())
        {
            return;
        }

        wchar_t b[Constants::MAX_LOG_LINE_LENGTH];
        vswprintf_s(b, fmt, args);
        PrintLn(LogLevel::RXN_DEBUG, b);
    }

    void Logger::LoggerImpl::Warn(const wchar_t *fmt, char *args)
    {
        if (!this->IsWarnEnabled())
        {
            return;
        }

        wchar_t b[Constants::MAX_LOG_LINE_LENGTH];
        vswprintf_s(b, fmt, args);
        this->PrintLn(LogLevel::RXN_WARN, b);
    }

    void Logger::LoggerImpl::Error(const wchar_t *fmt, char *args)
    {
        if (!this->IsErrorEnabled())
        {
            return;
        }

        wchar_t b[Constants::MAX_LOG_LINE_LENGTH];
        vswprintf_s(b, fmt, args);
        this->PrintLn(LogLevel::RXN_ERROR, b);
    }

    void Logger::LoggerImpl::Trace(const wchar_t *fmt, char *args)
    {
        if (!this->IsTraceEnabled())
        {
            return;
        }

        wchar_t b[Constants::MAX_LOG_LINE_LENGTH];
        vswprintf_s(b, fmt, args);
        this->PrintLn(LogLevel::RXN_TRACE, b);
    }

#pragma endregion // LoggerImpl
    /* -------------------------------------------------------- */


    /* -------------------------------------------------------- */
    /*  Logger                                                  */
    /* -------------------------------------------------------- */
#pragma region Logger

    inline const Logger Instance;
    Logger::Logger() = default;
    Logger::~Logger() = default;

    WString Logger::GetLogDirectory()
    {
        return Instance.m_LoggerImpl->GetLogDirectory();
    }

    void Logger::SetLogLevel(const LogLevel &level)
    {
        Instance.m_LoggerImpl->SetLogLevel(level);
    }

    void Logger::SetWriteToFile(const bool &writeToFile)
    {
        Instance.m_LoggerImpl->SetWriteToFile(writeToFile);
    }

    void Logger::SetWriteToConsole(const bool &writeToConsole)
    {
        Instance.m_LoggerImpl->SetWriteToConsole(writeToConsole);
    }

    void Logger::Info(const wchar_t *fmt...)
    {
        va_list args;
        va_start(args, fmt);
        Instance.m_LoggerImpl->Info(fmt, args);
        va_end(args);
    }

    void Logger::Warn(const wchar_t *fmt...)
    {
        va_list args;
        va_start(args, fmt);
        Instance.m_LoggerImpl->Warn(fmt, args);
        va_end(args);
    }

    void Logger::Error(const wchar_t *fmt...)
    {
        va_list args;
        va_start(args, fmt);
        Instance.m_LoggerImpl->Error(fmt, args);
        va_end(args);
    }

    void Logger::Debug(const wchar_t *fmt...)
    {
        va_list args;
        va_start(args, fmt);
        Instance.m_LoggerImpl->Debug(fmt, args);
        va_end(args);
    }

    void Logger::Trace(const wchar_t *fmt...)
    {

        va_list args;
        va_start(args, fmt);
        Instance.m_LoggerImpl->Trace(fmt, args);
        va_end(args);
    }

    void Logger::PrintLnSeperator()
    {
        Instance.m_LoggerImpl->PrintLnSeperator();
    }

    void Logger::PrintLnHeader(const wchar_t *fmt)
    {
        Instance.m_LoggerImpl->PrintLnHeader(fmt);
    }

    bool Logger::IsMTailRunning()
    {
        return Instance.m_LoggerImpl->IsMTailRunning();
    }

    bool Logger::StartMTail()
    {
        return Instance.m_LoggerImpl->StartMTail();
    }

    WString Logger::GetLogFileName()
    {
        return Instance.m_LoggerImpl->GetLogFileName();
    }

    bool Logger::IsInfoEnabled()
    {
        return Instance.m_LoggerImpl->IsLevelEnabled(LogLevel::RXN_INFO);
    }

    bool Logger::IsWarnEnabled()
    {
        return Instance.m_LoggerImpl->IsLevelEnabled(LogLevel::RXN_WARN);
    }

    bool Logger::IsErrorEnabled()
    {
        return Instance.m_LoggerImpl->IsLevelEnabled(LogLevel::RXN_ERROR);
    }

    bool Logger::IsDebugEnabled()
    {
        return Instance.m_LoggerImpl->IsLevelEnabled(LogLevel::RXN_DEBUG);
    }

    bool Logger::IsTraceEnabled()
    {
        return Instance.m_LoggerImpl->IsLevelEnabled(LogLevel::RXN_TRACE);
    }

    LogLevel Logger::LoggerImpl::GetLogLevel() const
    {
        return m_Level;
    }

#pragma endregion // Logger
    /* -------------------------------------------------------- */


} // RxnEngine::Common



