// ThisApp类 接管程序与操作系统之间的通信

#pragma once


// 窗口高度
static constexpr wchar_t* WINDOW_TITLE = L"Title";
// 本程序
class ThisApp {
public:
    // 窗口宽度
    static constexpr uint32_t WINDOW_WIDTH = 640;
    // 窗口高度
    static constexpr uint32_t WINDOW_HEIGHT = 480;
    // 构造函数
    ThisApp()noexcept;
    // 析构函数
    ~ThisApp() noexcept;
    // 初始化
    auto Initialize(HINSTANCE hInstance, int nCmdShow) noexcept ->HRESULT;
    // 消息循环
    void RunMessageLoop() noexcept;
private:
    // 窗口过程函数
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
private:
    // 窗口句柄
    HWND                        m_hwnd = nullptr;
    // 渲染器
    SceneRenderer               m_sceneRenderer;
    // 退出
    std::atomic<BOOL>           m_bExit = FALSE;
};