#include "stdafx.h"
#include "included.h"

#define TITLE L"径向模糊 基本"

// 初始化
auto ThisApp::Initialize(HINSTANCE hInstance, int nCmdShow) noexcept ->HRESULT {
    HRESULT hr = E_FAIL;
    //register window class
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ThisApp::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(void*);
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
        hr = m_oImagaRenderer.SetHwnd(m_hwnd);
    }
    // 显示窗口
    if (SUCCEEDED(hr)) {
        ::ShowWindow(m_hwnd, nCmdShow);
        ::UpdateWindow(m_hwnd);
        // 异常
        try {
            m_threadRender.std::thread::~thread();
            m_threadRender.std::thread::thread([this]() noexcept {
                ::CoInitialize(nullptr);
                while (true) {
                    m_oImagaRenderer.OnRender(1);
                    if (m_bExit) break;
                }
                ::CoUninitialize();
            });
        }
        // 失败
        catch (...) {
            hr = E_FAIL;
        }
    }
    return hr;
}



// 消息循环
void ThisApp::RunMessageLoop() noexcept {
    MSG msg;
    while (::GetMessageW(&msg, nullptr, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
}


// 窗口过程函数
auto CALLBACK ThisApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept -> LRESULT {
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
            case WM_CLOSE:
                result = pOurApp->OnClose();
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


// 关闭时候
auto ThisApp::OnClose() noexcept ->LRESULT {
    // 将收尾操作(如结束全部子线程)放在这里
    m_bExit = TRUE;
    // 等待线程退出
    try { m_threadRender.join(); }
    catch (...) {}
    // 摧毁窗口
    ::DestroyWindow(m_hwnd);
    return LRESULT(1);
}