#include "included.h"
#include "ShellScalingAPI.h"

// 应用程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#ifdef _DEBUG
    ::AllocConsole();
    ::_cwprintf(L"Battle Control  Online! \n");
#endif
    if (SUCCEEDED(::CoInitialize(nullptr)))
    {
        {
            Demo::ThisApp app;
            if (SUCCEEDED(app.Initialize(hInstance, nCmdShow)))
            {
                app.RunMessageLoop();
            }
        }
        ::CoUninitialize();
    }
#ifdef _DEBUG
    ::_cwprintf(L"Battle Control Terminated! \n");
    ::FreeConsole();
#endif
    return 0;
}

#pragma comment(lib, "Shcore")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "Winmm")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dwrite" )
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "d2d1" )
#pragma comment(lib, "dcomp")
#pragma comment(lib, "windowscodecs")