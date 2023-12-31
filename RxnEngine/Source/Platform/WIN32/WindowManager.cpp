#include "Rxn.h"
#include "WindowManager.h"


namespace Rxn::Platform::Win32
{

    WindowManager::WindowManager() = default;
    WindowManager::~WindowManager() = default;

    void WindowManager::CreateNewWindow(WindowDesc &windowDesc)
    {

        if (m_ManagedWindows.contains(windowDesc.windowClassName))
        {
            RXN_LOGGER::Error(L"Window class %s already exists.");
            return;
        }

        auto window = std::make_shared<Window>(windowDesc.windowName, windowDesc.windowClassName, windowDesc.windowSize.cx, windowDesc.windowSize.cy);

        window->m_WindowBackgroundColour = windowDesc.windowBgColour;
        window->m_WindowBorderColour = windowDesc.windowBorderColour;
        window->m_WindowActiveBorderHighlightColour = windowDesc.windowActiveBorderHighlightColour;
        window->m_WindowStyle = static_cast<uword>(windowDesc.windowStyle);

        m_ManagedWindows.try_emplace(window->GetClass(), window);
        m_WindowCounter++;
    }


    void WindowManager::AddWindow(std::shared_ptr<Window> window)
    {
        if (m_ManagedWindows.contains(window->GetClass()))
        {
            RXN_LOGGER::Error(L"Window class %s already exists.");
            return;
        }

        RXN_LOGGER::Info(L"Adding Window %s", window->GetClass().c_str());

        m_ManagedWindows.try_emplace(window->GetClass(), window);
        m_WindowCounter++;
    }
}


