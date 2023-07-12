#include "Rxn.h"

#include "IApplication.h"
#include "Common/Logger.h"
#include "Common/CommandLine.h"


extern std::unique_ptr<Rxn::Platform::Win32::IApplication> EntryApplication();

_Use_decl_annotations_
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
    auto entry = EntryApplication();

    entry->ConfigureEngine();
    entry->PreInitialize();
    entry->Initialize();

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

    return 0;
}