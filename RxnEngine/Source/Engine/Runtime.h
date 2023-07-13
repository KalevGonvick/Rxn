#pragma once
#include "Platform/WIN32/WindowManager.h"

namespace Rxn::Engine
{


    class RXN_ENGINE_API Runtime : public Platform::Win32::IApplication
    {
    public:

        Runtime();
        ~Runtime();

    public:

        virtual void PreInitialize() override;

        void SetMode(EngineRuntimeMode mode);

        const EngineRuntimeMode GetEngineMode();
        const WString GetEngineModeString();

    private:

        const void SetupLogger();
        const void SetupPlatformGUI();
        const void InitGraphics();

    protected:

        std::shared_ptr<RxnEngine> m_Engine;
        std::shared_ptr<Platform::Win32::WindowManager> m_WindowManager;

    };

}