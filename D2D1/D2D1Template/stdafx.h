#pragma once

// 用途:  包含不再修改头文件,生成预编译文件
#ifndef _DEBUG
#define NDEBUG
#endif

#define USING_DirectComposition
// 使用固定DPI(PPI)
#define FIXED_DPI (96.f)

// 取消强制_s函数
#define _CRT_SECURE_NO_WARNINGS


#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

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
    if (newObject != nullptr)
        ((IUnknown*)newObject)->AddRef();

    return newObject;
}

inline void SafeCloseHandle(HANDLE& handle){
    if (handle){
        ::CloseHandle(handle);
        handle = nullptr;
    }
}

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <cwchar>

#include <dxgi1_3.h>
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


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwrite.lib" )
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d2d1.lib" )
#pragma comment(lib, "windowscodecs.lib" )