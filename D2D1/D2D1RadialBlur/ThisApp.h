// ThisApp类 接管程序与操作系统之间的通信

#pragma once

// ThisApp
class ThisApp{
public:
    // 构造函数
    ThisApp() noexcept {};
    // 析构函数
    ~ThisApp() noexcept {};
    // 初始化
    auto Initialize(HINSTANCE hInstance, int nCmdShow) noexcept ->HRESULT;
    // 消息循环
    void RunMessageLoop() noexcept;
    // 窗口过程函数
    static auto CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept ->LRESULT;
public:
    // 关闭时
    auto OnClose() noexcept ->LRESULT;
private:
    // 窗口句柄
    HWND                        m_hwnd = nullptr;
    // 渲染器
    ImageRenderer               m_oImagaRenderer;
    // 退出
    std::atomic<BOOL>           m_bExit = FALSE;
    // 渲染处理线程
    std::thread                 m_threadRender;
};