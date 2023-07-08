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
        RXN_LOGGER::GetLogDirectory();

        RXN_LOGGER::PrintLnHeader(L"Engine Config");
        RXN_LOGGER::Info(L"Application Starting...");
        RXN_LOGGER::Info(L"Log Dir: %s", RXN_LOGGER::GetLogDirectory().c_str());
        RXN_LOGGER::Info(L"Game Name: %s", Engine::EngineContext::GetEngineSettings().GetGameName());
        RXN_LOGGER::Info(L"Boot Time: %s", Common::Time::GetDateTimeString().c_str());
        RXN_LOGGER::Info(L"Mode: %s", Engine::Runtime::GetEngineModeString().c_str());
        RXN_LOGGER::PrintLnSeperator();

        //Core::RxnBinaryHandler::ReadRxnFile();

        RXN_LOGGER::Info(L"Loaded up... %s", Engine::EngineContext::GetEngineSettings().GetGameName());
    }

    LRESULT Simulation::MessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {

        }

        return Window::MessageHandler(hWnd, msg, wParam, lParam);
    }

} // Rxn::Engine