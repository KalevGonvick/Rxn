#pragma once
#include "Logger.h"
#include <string>


namespace Rxn::Common
{

    class Logger::LoggerImpl
    {
    public:

        LoggerImpl();
        ~LoggerImpl();

    public:

        void SetLogLevel(LogLevel level);
        void SetWriteToFile(bool writeToFile);
        void SetWriteToConsole(bool writeToConsole);

        void PrintLnSeperator();
        void PrintLnHeader(const wchar_t *fmt);
        void PrintLn(const LogLevel &level, const wchar_t *fmt);

        bool IsMTailRunning();
        bool StartMTail();

        bool IsInfoEnabled();
        bool IsWarnEnabled();
        bool IsErrorEnabled();
        bool IsDebugEnabled();
        bool IsTraceEnabled();
        bool IsLevelEnabled(LogLevel level);

        void Info(const wchar_t *fmt, char *args);
        void Debug(const wchar_t *fmt, char *args);
        void Warn(const wchar_t *fmt, char *args);
        void Error(const wchar_t *fmt, char *args);
        void Trace(const wchar_t *fmt, char *args);

        std::wstring GetLogFileName();
        std::wstring GetLogDirectory();
        LogLevel GetLogLevel();

    private:

        bool m_bWriteToFile;
        bool m_bWriteToConsole;
        bool m_bLogFileCreated;

        LogLevel m_uLevel;

        std::wstring m_wOutputLogDir;

        const std::wstring m_kpszDebugSeverity = L"[DEBUG]";
        const std::wstring m_kpszErrorSeverity = L"[ERROR]";
        const std::wstring m_kpszWarningSeverity = L"[WARN]";
        const std::wstring m_kpszInfoSeverity = L"[INFO]";
        const std::wstring m_kpszTraceSeverity = L"[TRACE]";

    };


} // Common