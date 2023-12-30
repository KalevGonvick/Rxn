        /*****************************************************************//**
 * \file   Logger.h
 * \brief
 *
 * \author kalev
 * \date   July 2023
 *********************************************************************/
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


    class Logger : NonCopyable
    {
    public:

        Logger();
        ~Logger();

    public:

        /**
         * .
         *
         * \param level
         */
        RXN_ENGINE_API static void SetLogLevel(const LogLevel &level);

        /**
         * .
         *
         * \param writeToFile
         */
        RXN_ENGINE_API static void SetWriteToFile(const bool &writeToFile);

        /**
         * .
         *
         * \param writeToConsole
         */
        RXN_ENGINE_API static void SetWriteToConsole(const bool &writeToConsole);

        /**
         * .
         *
         * \param ...
         */
        RXN_ENGINE_API static void Info(const wchar_t *fmt...);

        /**
         * .
         *
         * \param ...
         */
        RXN_ENGINE_API static void Warn(const wchar_t *fmt...);

        /**
         * .
         *
         * \param ...
         */
        RXN_ENGINE_API static void Error(const wchar_t *fmt...);

        /**
         * .
         *
         * \param ...
         */
        RXN_ENGINE_API static void Debug(const wchar_t *fmt...);

        /**
         * .
         *
         * \param ...
         */
        RXN_ENGINE_API static void Trace(const wchar_t *fmt...);

        /**
         * .
         *
         */
        RXN_ENGINE_API static void PrintLnSeperator();

        /**
         * .
         *
         * \param fmt
         */
        RXN_ENGINE_API static void PrintLnHeader(const wchar_t *fmt);

        /**
         * .
         *
         * \return
         */
        RXN_ENGINE_API static bool IsMTailRunning();

        /**
         * .
         *
         * \return
         */
        RXN_ENGINE_API static bool StartMTail();

        /**
         * .
         *
         * \return
         */
        RXN_ENGINE_API static bool IsInfoEnabled();

        /**
         * .
         *
         * \return
         */
        RXN_ENGINE_API static bool IsWarnEnabled();

        /**
         * .
         *
         * \return
         */
        RXN_ENGINE_API static bool IsErrorEnabled();

        /**
         * .
         *
         * \return
         */
        RXN_ENGINE_API static bool IsDebugEnabled();

        /**
         * .
         *
         * \return
         */
        RXN_ENGINE_API static bool IsTraceEnabled();

        /**
         * .
         *
         * \return
         */
        RXN_ENGINE_API static WString GetLogFileName();

        /**
         * .
         *
         * \return
         */
        RXN_ENGINE_API static WString GetLogDirectory();

    private:

        class LoggerImpl;
        std::unique_ptr<LoggerImpl> m_LoggerImpl = std::make_unique<LoggerImpl>();

    };


} // Common




