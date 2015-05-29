// ThisApp类 接管程序与操作系统之间的通信

#pragma once

class ThisApp{
public:
    // 构造函数
    ThisApp(){};
    // 析构函数
    ~ThisApp(){};
    // 初始化
    HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
    // 消息循环
    void RunMessageLoop();
private:
    // 窗口过程函数
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
    // 窗口句柄
    HWND                        m_hwnd = nullptr;
    // 渲染器
    ImageRenderer               m_ImagaRenderer;
    // 退出
    std::atomic<BOOL>           m_bExit = FALSE;
    // 渲染处理线程
    std::thread                 m_threadRender;
};