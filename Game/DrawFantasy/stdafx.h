#pragma once

// 用途:  包含不再修改头文件,生成预编译文件


/*/ DEBUG时 取消C4005警告 微软自己蛋疼也要别人蛋疼
#ifdef _DEBUG
#pragma warning( disable: 4005 )
#endif*/
#define USING_DirectComposition
// 使用固定DPI(PPI)
#define FIXED_DPI (96.f)

// 取消强制_s函数
#define _CRT_SECURE_NO_WARNINGS

#ifndef WINVER              // Allow use of features specific to Windows 8 or later.
#define WINVER 0x0800       
#endif						// Change this to the appropriate value to target other versions of Windows.

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows 8 or later.
#define _WIN32_WINNT 0x0800
#endif						// Change this to the appropriate value to target other versions of Windows.


#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <windowsx.h>

// WrapAL
#include "../WrapAL/wrapal.h"

#ifdef _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


template<class Interface>
inline void SafeRelease(Interface *&pInterfaceToRelease){
    if (pInterfaceToRelease != nullptr){
        pInterfaceToRelease->Release();
        pInterfaceToRelease = nullptr;
    }
}


template <typename Interface>
inline Interface* SafeAcquire(Interface* newObject)
{
    if (newObject != nullptr) {
        newObject->AddRef();
    }
    return newObject;
}

inline void SafeCloseHandle(HANDLE& handle){
    if (handle){
        ::CloseHandle(handle);
        handle = nullptr;
    }
}

#include <algorithm>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <cwchar>
#include <mutex>
#include <new>

#include <dxgi1_2.h>
#include <D3D11.h>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <dwrite_2.h>
#include <wincodec.h>

// DirectComposition 
#ifdef USING_DirectComposition
#   include <dcomp.h>
#   pragma comment(lib, "dcomp")
#endif



#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dwrite" )
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "d2d1" )
#pragma comment(lib, "windowscodecs" )


#include "mruby.h"
#include "mruby/compile.h"