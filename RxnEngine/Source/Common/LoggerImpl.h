#pragma once
#include "Logger.h"
#include <string>


namespace Rxn::Common
{

    class RXN_ENGINE_API Logger::LoggerImpl
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

        bool IsMTailRunning() const;
        bool StartMTail();

        bool IsInfoEnabled() const;
        bool IsWarnEnabled() const;
        bool IsErrorEnabled() const;
        bool IsDebugEnabled() const;
        bool IsTraceEnabled() const;
        bool IsLevelEnabled(LogLevel level) const;

        void Info(const wchar_t *fmt, char *args);
        void Debug(const wchar_t *fmt, char *args);
        void Warn(const wchar_t *fmt, char *args);
        void Error(const wchar_t *fmt, char *args);
        void Trace(const wchar_t *fmt, char *args);

        WString GetLogFileName() const;
        WString GetLogDirectory();
        LogLevel GetLogLevel() const;

    private:

        bool m_WriteToFile = true;
        bool m_WriteToConsole = true;
        bool m_LogFileCreated = false;

        LogLevel m_Level = LogLevel::RXN_ERROR;

        WString m_OutputLogDir = L"";

        const WString m_DebugSeverity = L"[DEBUG]";
        const WString m_ErrorSeverity = L"[ERROR]";
        const WString m_WarningSeverity = L"[WARN]";
        const WString m_InfoSeverity = L"[INFO]";
        const WString m_TraceSeverity = L"[TRACE]";

    };


} // Common