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

    entry->ConfigureEngine();

    Rxn::Graphics::RenderContext::InitRenderContext();

    entry->PreInitialize();
    entry->Initialize();
    FILE *fStreamOut = CreateConsole();
    MSG msg = { 0 };
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            entry->Update();
        }
    }
    entry->OnDestroy();
    DestroyConsole(fStreamOut);
    return static_cast<char>(msg.wParam);
}