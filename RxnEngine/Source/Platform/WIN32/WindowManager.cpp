#include "Rxn.h"
#include "WindowManager.h"


namespace Rxn::Platform::Win32
{


    WindowManager::WindowManager()
        : m_ManagedWindows()
        , m_WindowCounter(0)
    {
    }

    WindowManager::~WindowManager() = default;


    void WindowManager::CreateNewWindow(WindowDesc &windowDesc)
    {

        if (m_ManagedWindows.find(windowDesc.windowClassName) != m_ManagedWindows.end())
        {
            RXN_LOGGER::Error(L"Window class %s already exists.");
            return;
        }

        auto window = std::make_shared<Window>(windowDesc.windowName, windowDesc.windowClassName);

        window->m_WindowBackgroundColour = windowDesc.windowBgColour;
        window->m_WindowBorderColour = windowDesc.windowBorderColour;
        window->m_WindowActiveBorderHighlightColour = windowDesc.windowActiveBorderHighlightColour;
        window->m_WindowStyle = windowDesc.windowStyle;

        m_ManagedWindows.emplace(window->GetClass(), window);
        m_WindowCounter++;
    }


    void WindowManager::AddWindow(std::shared_ptr<Window> window)
    {
        if (m_ManagedWindows.find(window->GetClass()) != m_ManagedWindows.end())
        {
            RXN_LOGGER::Error(L"Window class %s already exists.");
            return;
        }

        RXN_LOGGER::Info(L"Adding Window %s", window->GetClass().c_str());

        m_ManagedWindows.emplace(window->GetClass(), window);
        m_WindowCounter++;
    }
}


