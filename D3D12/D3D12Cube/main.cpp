#include "stdafx.h"
#include "included.h"
#include <ShellScalingApi.h>


// 应用程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
#ifdef _DEBUG
    ::AllocConsole();
    ::_cwprintf(L"Battle Control  Online! \n");
#endif
    // SetProcessDpiAwareness 只能用在Win8.1以上
    auto hr = ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    assert(SUCCEEDED(hr));
    if (SUCCEEDED(::CoInitialize(nullptr))) {
        {
            ThisApp app;
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

#ifdef _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif



#pragma comment(lib, "dcomp")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3d12")
#pragma comment(lib, "Shcore")
#pragma comment(lib, "winmm")
#pragma comment(lib, "d3dcompiler")
