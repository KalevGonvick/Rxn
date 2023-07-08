#include "Rxn.h"
#include "Logger.h"
#include "LoggerImpl.h"

namespace Rxn::Common
{


    /* -------------------------------------------------------- */
    /*  LoggerImpl                                              */
    /* -------------------------------------------------------- */
#pragma region LoggerImpl

    Logger::LoggerImpl::LoggerImpl()
        : m_bWriteToConsole(true)
        , m_bWriteToFile(true)
        , m_uLevel(LogLevel::RXN_ERROR)
        , m_wOutputLogDir(L"")
        , m_bLogFileCreated(false)
    {
    }

    Logger::LoggerImpl::~LoggerImpl()
    {
    }

    std::wstring Logger::LoggerImpl::GetLogDirectory()
    {
        if (this->m_bLogFileCreated)
        {
            return this->m_wOutputLogDir;
        }

        std::wstring path = L"";
        wchar_t *appDataLocal;

        SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataLocal);

        path.append(appDataLocal);
        path.append(L"\\");
        path.append(Engine::EngineContext::GetEngineSettings().GetGameName());
        CreateDirectory(path.c_str(), 0);

        path.append(L"\\Log");
        CreateDirectory(path.c_str(), 0);

        this->m_wOutputLogDir = path;
        this->m_bLogFileCreated = true;

        return path;
    }

    void Logger::LoggerImpl::SetLogLevel(LogLevel level)
    {
        this->m_uLevel = level;
    }

    void Logger::LoggerImpl::SetWriteToFile(bool writeToFile)
    {
        this->m_bWriteToFile = writeToFile;
    }

    void Logger::LoggerImpl::SetWriteToConsole(bool writeToConsole)
    {
        this->m_bWriteToConsole = writeToConsole;
    }

    void Logger::LoggerImpl::PrintLn(const LogLevel &level, const wchar_t *b)
    {
        std::wfstream outfile;
        outfile.open(std::wstring(GetLogDirectory() + L"\\" + GetLogFileName()), std::ios_base::app);
        std::wstring sev;
        switch (level)
        {
        case LogLevel::RXN_DEBUG:
            sev = m_kpszDebugSeverity;
            break;

        case LogLevel::RXN_ERROR:
            sev = m_kpszErrorSeverity;
            break;

        case LogLevel::RXN_INFO:
            sev = m_kpszInfoSeverity;
            break;

        case LogLevel::RXN_TRACE:
            sev = m_kpszTraceSeverity;
            break;

        case LogLevel::RXN_WARN:
            sev = m_kpszWarningSeverity;
            break;

        default:
            break;
        }

        if (outfile.is_open())
        {
            std::wstring s = L"\n" + Time::GetDateTimeString() + L" " + sev + L" " + b;
            outfile << s;
            outfile.close();
            OutputDebugString(s.c_str());
        }
        else
        {
            MessageBox(0, L"Unable to open log file...", L"Log Error", MB_OK);
        }
    }

    void Logger::LoggerImpl::PrintLnSeperator()
    {
        const int knMaxHeaderLineLen = 100;
        std::wstring s = L"";
        s.append(L"\n");
        for (int x = 0; x < knMaxHeaderLineLen; x++)
        {
            s.append(L"-");
        }
        s.append(L"\n");

        std::wfstream outfile;
        outfile.open(std::wstring(GetLogDirectory() + L"\\" + GetLogFileName()), std::ios_base::app);

        if (outfile.is_open())
        {
            outfile << s;
            outfile.close();
        }
        else
        {
            MessageBox(0, L"Unable to open log file...", L"Log Error", MB_OK);
        }
    }

    void Logger::LoggerImpl::PrintLnHeader(const wchar_t *fmt)
    {
        const int knMaxHeaderLineLen = 100;
        const int knHeaderPadding = 4;

        int titleLen = std::char_traits<wchar_t>::length(fmt);

        int remainingLength = knMaxHeaderLineLen - knHeaderPadding - titleLen - knHeaderPadding;
        int oneSideLength = remainingLength / 2;

        std::wstring s = L"\n";

        for (int x = 0; x < oneSideLength; x++)
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
        outfile.open(std::wstring(GetLogDirectory() + L"\\" + GetLogFileName()), std::ios_base::app);

        if (outfile.is_open())
        {
            outfile << s;
            outfile.close();
        }
        else
        {
            MessageBox(0, L"Unable to open log file...", L"Log Error", MB_OK);
        }

    }

    bool Logger::LoggerImpl::IsMTailRunning()
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

        std::wstring url = path + std::wstring(L"/mTail.exe");
        std::wstring params = L" \"" + GetLogDirectory() + L"/" + GetLogFileName() + L"\" /start";

        ShellExecute(0, 0, url.c_str(), params.c_str(), 0, SW_SHOWDEFAULT);

        return true;
    }

    std::wstring Logger::LoggerImpl::GetLogFileName()
    {
        std::wstring file = Engine::EngineContext::GetEngineSettings().GetGameName();
        file.append(Engine::EngineContext::GetEngineSettings().GetBootTime());
        file.append(L".log");
        return file;
    }

    bool Logger::LoggerImpl::IsInfoEnabled()
    {
        return IsLevelEnabled(LogLevel::RXN_INFO);
    }

    bool Logger::LoggerImpl::IsWarnEnabled()
    {
        return IsLevelEnabled(LogLevel::RXN_WARN);
    }

    bool Logger::LoggerImpl::IsErrorEnabled()
    {
        return IsLevelEnabled(LogLevel::RXN_ERROR);
    }

    bool Logger::LoggerImpl::IsDebugEnabled()
    {
        return IsLevelEnabled(LogLevel::RXN_DEBUG);
    }

    bool Logger::LoggerImpl::IsTraceEnabled()
    {
        return IsLevelEnabled(LogLevel::RXN_TRACE);
    }

    bool Logger::LoggerImpl::IsLevelEnabled(LogLevel level)
    {
        return this->m_uLevel >= level;
    }

    void Logger::LoggerImpl::Info(const wchar_t *fmt, char *args)
    {
        if (!this->IsInfoEnabled())
        {
            return;
        }

        wchar_t b[Constants::MAXLOGLINE];
        vswprintf_s(b, fmt, args);
        //std::async(&LoggerImpl::PrintLn, this, LogLevel::RXN_INFO, b);
        this->PrintLn(LogLevel::RXN_INFO, b);

    }

    void Logger::LoggerImpl::Debug(const wchar_t *fmt, char *args)
    {
        if (!this->IsDebugEnabled())
        {
            return;
        }

        wchar_t b[Constants::MAXLOGLINE];
        vswprintf_s(b, fmt, args);
        //std::async(&LoggerImpl::PrintLn, this, LogLevel::RXN_DEBUG, b);
        this->PrintLn(LogLevel::RXN_DEBUG, b);
    }

    void Logger::LoggerImpl::Warn(const wchar_t *fmt, char *args)
    {
        if (!this->IsWarnEnabled())
        {
            return;
        }

        wchar_t b[Constants::MAXLOGLINE];
        vswprintf_s(b, fmt, args);
        //std::async(&LoggerImpl::PrintLn, this, LogLevel::RXN_WARN, b);
        this->PrintLn(LogLevel::RXN_WARN, b);
    }

    void Logger::LoggerImpl::Error(const wchar_t *fmt, char *args)
    {
        if (!this->IsErrorEnabled())
        {
            return;
        }

        wchar_t b[Constants::MAXLOGLINE];
        vswprintf_s(b, fmt, args);
        //std::async(&LoggerImpl::PrintLn, this, LogLevel::RXN_ERROR, b);
        this->PrintLn(LogLevel::RXN_ERROR, b);
    }

    void Logger::LoggerImpl::Trace(const wchar_t *fmt, char *args)
    {
        if (!this->IsTraceEnabled())
        {
            return;
        }

        wchar_t b[Constants::MAXLOGLINE];
        vswprintf_s(b, fmt, args);
        //std::async(&LoggerImpl::PrintLn, this, LogLevel::RXN_TRACE, b);
        this->PrintLn(LogLevel::RXN_TRACE, b);
    }

#pragma endregion // LoggerImpl
    /* -------------------------------------------------------- */


    /* -------------------------------------------------------- */
    /*  Logger                                                  */
    /* -------------------------------------------------------- */
#pragma region Logger

    static Logger &Instance()
    {
        static Logger inst;
        return inst;
    }

    Logger::Logger()
        : m_pLoggerImpl(new LoggerImpl())
    {
    }

    Logger::~Logger() = default;

    std::wstring Logger::GetLogDirectory()
    {
        return Instance().m_pLoggerImpl->GetLogDirectory();
    }


    void Logger::SetLogLevel(const LogLevel &level)
    {
        Instance().m_pLoggerImpl->SetLogLevel(level);
    }

    void Logger::SetWriteToFile(const bool &writeToFile)
    {
        Instance().m_pLoggerImpl->SetWriteToFile(writeToFile);
    }

    void Logger::SetWriteToConsole(const bool &writeToConsole)
    {
        Instance().m_pLoggerImpl->SetWriteToConsole(writeToConsole);
    }

    void Logger::Info(const wchar_t *fmt...)
    {
        va_list args;
        va_start(args, fmt);
        Instance().m_pLoggerImpl->Info(fmt, args);
        va_end(args);
    }

    void Logger::Warn(const wchar_t *fmt...)
    {
        va_list args;
        va_start(args, fmt);
        Instance().m_pLoggerImpl->Warn(fmt, args);
        va_end(args);
    }

    void Logger::Error(const wchar_t *fmt...)
    {
        va_list args;
        va_start(args, fmt);
        Instance().m_pLoggerImpl->Error(fmt, args);
        va_end(args);
    }

    void Logger::Debug(const wchar_t *fmt...)
    {
        va_list args;
        va_start(args, fmt);
        Instance().m_pLoggerImpl->Debug(fmt, args);
        va_end(args);
    }

    void Logger::Trace(const wchar_t *fmt...)
    {

        va_list args;
        va_start(args, fmt);
        Instance().m_pLoggerImpl->Trace(fmt, args);
        va_end(args);
    }

    void Logger::PrintLnSeperator()
    {
        Instance().m_pLoggerImpl->PrintLnSeperator();
    }

    void Logger::PrintLnHeader(const wchar_t *fmt)
    {
        Instance().m_pLoggerImpl->PrintLnHeader(fmt);
    }

    bool Logger::IsMTailRunning()
    {
        return Instance().m_pLoggerImpl->IsMTailRunning();
    }

    bool Logger::StartMTail()
    {
        return Instance().m_pLoggerImpl->StartMTail();
    }

    std::wstring Logger::GetLogFileName()
    {
        return Instance().m_pLoggerImpl->GetLogFileName();
    }

    bool Logger::IsInfoEnabled()
    {
        return Instance().m_pLoggerImpl->IsLevelEnabled(LogLevel::RXN_INFO);
    }

    bool Logger::IsWarnEnabled()
    {
        return Instance().m_pLoggerImpl->IsLevelEnabled(LogLevel::RXN_WARN);
    }

    bool Logger::IsErrorEnabled()
    {
        return Instance().m_pLoggerImpl->IsLevelEnabled(LogLevel::RXN_ERROR);
    }

    bool Logger::IsDebugEnabled()
    {
        return Instance().m_pLoggerImpl->IsLevelEnabled(LogLevel::RXN_DEBUG);
    }

    bool Logger::IsTraceEnabled()
    {
        return Instance().m_pLoggerImpl->IsLevelEnabled(LogLevel::RXN_TRACE);
    }

    LogLevel Logger::LoggerImpl::GetLogLevel()
    {
        return this->m_uLevel;
    }

#pragma endregion // Logger
    /* -------------------------------------------------------- */


} // RxnEngine::Common



