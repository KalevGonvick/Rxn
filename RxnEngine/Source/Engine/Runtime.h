#pragma once
#include "Platform/WIN32/WindowManager.h"
#include "RxnEngine.h"

namespace Rxn::Engine
{


    class RXN_ENGINE_API Runtime : public Platform::Win32::IApplication
    {
    public:

        Runtime();
        ~Runtime();

    public:

        void InitializeEngineSystems() override;

        void SetMode(EngineRuntimeMode mode);

        EngineRuntimeMode GetEngineMode();
        WString GetEngineModeString();

    private:

        const void SetupLogger();
        const void SetupPlatformGUI();

    protected:

        RxnEngine &GetEngine();
        Platform::Win32::WindowManager &GetWindowManager();

    private:

        RxnEngine m_Engine{};
        Platform::Win32::WindowManager m_WindowManager{};

    };

}