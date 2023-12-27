#include "Rxn.h"
#include "IApplication.h"

extern std::unique_ptr<Rxn::Platform::Win32::IApplication> EntryApplication();

FILE *CreateConsole()
{
    AllocConsole();
    FILE *pStreamOut = nullptr;
    _wfreopen_s(&pStreamOut, L"CONOUT$", L"w", stdout);
    return pStreamOut;
}

void DestroyConsole(FILE *pStream)
{
    fclose(pStream);
    FreeConsole();
}

_Use_decl_annotations_
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
    auto entry = EntryApplication();

    entry->SetupEngineConfigurations();
    Rxn::Engine::EngineContext::GetTimer().SetFixedTimeStep(true);
    Rxn::Engine::EngineContext::GetTimer().SetTargetElapsedTicks(3);
    Rxn::Graphics::RenderContext::InitRenderContext();

    entry->InitializeEngineSystems();
    entry->InitializeRuntime();
    
    FILE *fStreamOut = CreateConsole();
    MSG msg = { nullptr };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            entry->UpdateEngine();
        }
    }
    Rxn::Graphics::RenderContext::GetDebugLayerController()->ReportLiveObjects(
        DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
    );
    entry->OnDestroy();
    DestroyConsole(fStreamOut);

    return static_cast<char>(msg.wParam);
}