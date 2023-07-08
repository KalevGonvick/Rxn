#include "Rxn.h"
#include "Simulation.h"

#include "Engine/SplashScreen.h"


namespace Rxn::Engine
{
    Simulation::Simulation()
        : Platform::Win32::Window(L"MainApplication", 0)
    {
        this->Platform::Win32::Window::SetSize(1280, 720);
        this->Platform::Win32::Window::RegisterComponentClass();
        this->Platform::Win32::Window::Initialize();
    }

    Simulation::~Simulation()
    {
    }

    void Simulation::PreInitialize()
    {
        Engine::SplashScreen::Open();
        Common::Logger::GetLogDirectory();

        Common::Logger::PrintLnHeader(L"Engine Config");
        Common::Logger::Info(L"Application Starting...");
        Common::Logger::Info(L"Log Dir: %s", Common::Logger::GetLogDirectory().c_str());
        Common::Logger::Info(L"Game Name: %s", Engine::EngineContext::GetEngineSettings().GetGameName());
        Common::Logger::Info(L"Boot Time: %s", Common::Time::GetDateTimeString().c_str());
        Common::Logger::Info(L"Mode: %s", Engine::Runtime::GetEngineModeString().c_str());
        Common::Logger::PrintLnSeperator();

        //Core::RxnBinaryHandler::ReadRxnFile();

        Common::Logger::Info(L"Loaded up... %s", Engine::EngineContext::GetEngineSettings().GetGameName());
    }

    LRESULT Simulation::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {

        }

        return Window::MessageHandler(hWnd, msg, wParam, lParam);
    }

} // Rxn::Engine