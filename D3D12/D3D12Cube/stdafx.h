#pragma once

// 用途:  包含不再修改头文件,生成预编译文件

#define USING_DirectComposition

// 取消强制_s函数
#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>




template<class Interface>
inline void SafeRelease(Interface *&pInterfaceToRelease) {
    if (pInterfaceToRelease != nullptr) {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = nullptr;
    }
}


template <typename Interface>
inline Interface* SafeAcquire(Interface* newObject) {
    if (newObject != nullptr)
        ((IUnknown*)newObject)->AddRef();

    return newObject;
}


#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <atomic>
#include <thread>
#include <cwchar>

#include <mmsystem.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <D3D12.h>
#include <wincodec.h>

// DirectComposition 
#ifdef USING_DirectComposition
#include <dcomp.h>
#endif
