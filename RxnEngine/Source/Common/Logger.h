#pragma once
#include <string>
#include <memory>
#include <future>
#include <fstream>
#include <ShlObj.h>
#include <string>
#include <tlhelp32.h>
#include <cstdio>


namespace Rxn::Common
{
    enum class LogLevel
    {
        RXN_ERROR = 0,
        RXN_WARN = 1,
        RXN_INFO = 2,
        RXN_DEBUG = 3,
        RXN_TRACE = 4
    };


    class RXN_ENGINE_API Logger
    {

    public:

        Logger();
        ~Logger();

    public:

        static void SetLogLevel(const LogLevel &level);
        static void SetWriteToFile(const bool &writeToFile);
        static void SetWriteToConsole(const bool &writeToConsole);

        static void Info(const wchar_t *fmt...);
        static void Warn(const wchar_t *fmt...);
        static void Error(const wchar_t *fmt...);
        static void Debug(const wchar_t *fmt...);
        static void Trace(const wchar_t *fmt...);

        static void PrintLnSeperator();
        static void PrintLnHeader(const wchar_t *fmt);

        static bool IsMTailRunning();
        static bool StartMTail();

        static bool IsInfoEnabled();
        static bool IsWarnEnabled();
        static bool IsErrorEnabled();
        static bool IsDebugEnabled();
        static bool IsTraceEnabled();

        static std::wstring GetLogFileName();
        static std::wstring GetLogDirectory();

    private:

        class LoggerImpl;
        std::unique_ptr<LoggerImpl> m_pLoggerImpl;

    };


} // Common




