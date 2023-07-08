#include "Rxn.h"
#include "CommandLine.h"
#include <algorithm>
#include <stdexcept>

namespace Rxn::Common::Cmd
{
    inline void HandleEngineMode(const wchar_t* arg)
    {
        if (wcscmp(arg, L"debug") == 0)
        {
            Engine::Runtime::SetMode(Engine::Runtime::EngineRuntimeMode::DEBUG);
        }

        if (wcscmp(arg, L"editor") == 0)
        {
            Engine::Runtime::SetMode(Engine::Runtime::EngineRuntimeMode::EDITOR);
        }

        if (wcscmp(arg, L"headless") == 0)
        {
            Engine::Runtime::SetMode(Engine::Runtime::EngineRuntimeMode::HEADLESS);
        }

        if (wcscmp(arg, L"production") == 0)
        {
            Engine::Runtime::SetMode(Engine::Runtime::EngineRuntimeMode::PRODUCTION);
        }
    }

    inline void HandleLoggerMode(const wchar_t * arg)
    {
        if (wcscmp(arg, L"mtail") == 0)
        {
            if (!Common::Logger::StartMTail())
            {
                Common::Logger::Error(L"Failed to start MTail... exiting...");
                throw std::runtime_error("Failed to start MTail... exiting...");
            }
        }

        // ... and other logging flags
    }

    void ReadCommandLineArguments()
    {
        int argc = 0;
        wchar_t ** argv = CommandLineToArgvW(GetCommandLine(), &argc);
        const char DASH = '-';

        for (int x = 1; x < argc; x++)
        {
            std::wstring key = argv[x];
            
            if (key[0] == DASH && key[1] == DASH)
            {
                key.erase(0, 2);
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                ReadArgument(key.c_str());
            }
            else if (key[0] == DASH)
            {
                key.erase(0, 1);
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                // TODO handle parameter argument instead of flag
            }
        }
    }

    void ReadArgument(const wchar_t* argKey, const wchar_t* argValue, boolean argumentIsFlag)
    {
        if (argumentIsFlag)
        {
            HandleLoggerMode(argKey);
            HandleEngineMode(argKey);
        }
        else 
        {
            // TODO handle parameter args
        }
        
    }

    
}

