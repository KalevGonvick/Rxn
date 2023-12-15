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


    class RXN_ENGINE_API Logger
    {
        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;

    public:

        Logger();
        ~Logger();

    public:

        /**
         * .
         *
         * \param level
         */
        static void SetLogLevel(const LogLevel &level);

        /**
         * .
         *
         * \param writeToFile
         */
        static void SetWriteToFile(const bool &writeToFile);

        /**
         * .
         *
         * \param writeToConsole
         */
        static void SetWriteToConsole(const bool &writeToConsole);

        /**
         * .
         *
         * \param ...
         */
        static void Info(const wchar_t *fmt...);

        /**
         * .
         *
         * \param ...
         */
        static void Warn(const wchar_t *fmt...);

        /**
         * .
         *
         * \param ...
         */
        static void Error(const wchar_t *fmt...);

        /**
         * .
         *
         * \param ...
         */
        static void Debug(const wchar_t *fmt...);

        /**
         * .
         *
         * \param ...
         */
        static void Trace(const wchar_t *fmt...);

        /**
         * .
         *
         */
        static void PrintLnSeperator();

        /**
         * .
         *
         * \param fmt
         */
        static void PrintLnHeader(const wchar_t *fmt);

        /**
         * .
         *
         * \return
         */
        static bool IsMTailRunning();

        /**
         * .
         *
         * \return
         */
        static bool StartMTail();

        /**
         * .
         *
         * \return
         */
        static bool IsInfoEnabled();

        /**
         * .
         *
         * \return
         */
        static bool IsWarnEnabled();

        /**
         * .
         *
         * \return
         */
        static bool IsErrorEnabled();

        /**
         * .
         *
         * \return
         */
        static bool IsDebugEnabled();

        /**
         * .
         *
         * \return
         */
        static bool IsTraceEnabled();

        /**
         * .
         *
         * \return
         */
        static WString GetLogFileName();

        /**
         * .
         *
         * \return
         */
        static WString GetLogDirectory();

    private:

        class LoggerImpl;
        std::unique_ptr<LoggerImpl> m_LoggerImpl = std::make_unique<LoggerImpl>();

    };


} // Common




