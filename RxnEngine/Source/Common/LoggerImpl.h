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

        WString GetLogFileName();
        WString GetLogDirectory();
        LogLevel GetLogLevel();

    private:

        bool m_WriteToFile;
        bool m_WriteToConsole;
        bool m_LogFileCreated;

        LogLevel m_Level;

        WString m_OutputLogDir;

        const WString m_DebugSeverity = L"[DEBUG]";
        const WString m_ErrorSeverity = L"[ERROR]";
        const WString m_WarningSeverity = L"[WARN]";
        const WString m_InfoSeverity = L"[INFO]";
        const WString m_TraceSeverity = L"[TRACE]";

    };


} // Common