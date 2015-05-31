#include "stdafx.h"
#include "included.h"

#define TITLE L"中点Bresenham法画直线"



// 初始化
HRESULT ThisApp::Initialize(HINSTANCE hInstance, int nCmdShow){
    HRESULT hr = E_FAIL;
    //register window class
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ThisApp::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"Direct2DTemplate";
    wcex.hIcon = nullptr;
    // 注册窗口
    ::RegisterClassExW(&wcex);
    // 计算窗口大小
    RECT window_rect = { 0, 0, WNDWIDTH, WNDHEIGHT };
    DWORD window_style = WS_OVERLAPPEDWINDOW;
    ::AdjustWindowRect(&window_rect, window_style, FALSE);
    window_rect.right -= window_rect.left;
    window_rect.bottom -= window_rect.top;
    window_rect.left = (::GetSystemMetrics(SM_CXFULLSCREEN) - window_rect.right) / 2;
    window_rect.top = (::GetSystemMetrics(SM_CYFULLSCREEN) - window_rect.bottom) / 2;
    // 创建窗口
    m_hwnd = ::CreateWindowExW(
#ifdef USING_DirectComposition
        WS_EX_NOREDIRECTIONBITMAP, 
#else
        0,
#endif
        wcex.lpszClassName, TITLE, window_style,
        window_rect.left, window_rect.top, window_rect.right, window_rect.bottom, 
        0, 0, hInstance, this
        );
    hr = m_hwnd ? S_OK : E_FAIL;
    // 设置窗口句柄
    if (SUCCEEDED(hr)) {
        hr = m_ImagaRenderer.SetHwnd(m_hwnd);
    }
    // 显示窗口
    if (SUCCEEDED(hr)) {
        ::ShowWindow(m_hwnd, nCmdShow);
        ::UpdateWindow(m_hwnd);
        m_threadRender.std::thread::~thread();
        m_threadRender.std::thread::thread(
            [this]() {
                ::CoInitialize(nullptr);
                while (true) {
                    m_ImagaRenderer.OnRender(1);
                    if (m_bExit) break;
                }
                ::CoUninitialize();
                return S_OK;
            }
            );
    }
    return hr;
}



// 消息循环
void ThisApp::RunMessageLoop(){
    MSG msg;

    while (::GetMessageW(&msg, nullptr, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
}

#include <windowsx.h>

// 窗口过程函数
LRESULT CALLBACK ThisApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    // 创建时 设置指针
    if (message == WM_CREATE) {
        auto pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        auto pOurApp = reinterpret_cast<ThisApp*>(pcs->lpCreateParams);
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pOurApp));
        result = 1;
    }
    else {
        ThisApp *pOurApp = reinterpret_cast<ThisApp *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(hwnd, GWLP_USERDATA))
            );
        // 处理标记
        bool wasHandled = false;
        if (pOurApp){
            switch (message)
            {
            case WM_MOUSEMOVE:
                pOurApp->m_ImagaRenderer.Lock();
                pOurApp->m_ImagaRenderer.pushed = wParam & MK_LBUTTON;
                if (pOurApp->m_ImagaRenderer.pushed) {
                    pOurApp->m_ImagaRenderer.points[1] = { 
                        float(GET_X_LPARAM(lParam)), float(GET_Y_LPARAM(lParam))
                    };
                }
                pOurApp->m_ImagaRenderer.Unlock();
                break;
            case WM_LBUTTONDOWN:
                pOurApp->m_ImagaRenderer.Lock();
                pOurApp->m_ImagaRenderer.points[0] = {
                    float(GET_X_LPARAM(lParam)), float(GET_Y_LPARAM(lParam))
                };
                pOurApp->m_ImagaRenderer.Unlock();
                break;
            case WM_LBUTTONUP:
                pOurApp->m_ImagaRenderer.Lock();
                pOurApp->m_ImagaRenderer.pushed = FALSE;
                pOurApp->m_ImagaRenderer.Unlock();
                break;
            case WM_CLOSE:
                // 将收尾操作(如结束全部子线程)放在这里
                pOurApp->m_bExit = TRUE;
                // join
                pOurApp->m_threadRender.join();
                ::DestroyWindow(hwnd);
                result = 1;
                wasHandled = true;
                break;
            case WM_DESTROY:
                ::PostQuitMessage(0);
                result = 1;
                wasHandled = true;
                break;
            }
        }
        // 默认处理标记
        if (!wasHandled) {
            result = ::DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }
    return result;
}

