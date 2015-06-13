﻿// ThisApp类 接管程序与操作系统之间的通信

#pragma once

// 本程序
class ThisApp {
public:
    // 窗口宽度
    static constexpr uint32_t WINDOW_WIDTH = 800;
    // 窗口高度
    static constexpr uint32_t WINDOW_HEIGHT = 600;
public:
    // 构造函数
    ThisApp() noexcept : m_imageRenderer(*this) {};
    // 析构函数
    ~ThisApp() noexcept {};
    // 初始化
    auto Initialize(HINSTANCE hInstance, int nCmdShow)noexcept ->HRESULT;
    // 消息处理
    auto MessageHandle(UINT message, WPARAM wParam, LPARAM lParam, LRESULT&) ->bool;
    // 消息循环
    void RunMessageLoop() noexcept;
public:
    // 读取图片文件
    auto LoadBitmapFromFile(PCWSTR uri, ID2D1Bitmap1 ** ppb) noexcept {
        return m_imageRenderer.LoadBitmapFromFile(uri, ppb);
    }
    // 载入新的场景
    template<class T> auto LoadScene() { m_spNowScene.reset(new(std::nothrow) T(*this)); }
    // 载入新的场景
    template<class T> auto LoadScene(void* data) { m_spNowScene.reset(new(std::nothrow) T(data, *this)); }
public:
    // 显示标题栏
    auto ShowCaption()noexcept { return m_imageRenderer.ShowCaption(); }
    // 获取当前场景
    auto Scene() noexcept { return m_pOldScene; }
    // 加锁
    auto Lock() { m_mutex.lock(); }
    // 解锁
    auto Unlock() { m_mutex.unlock(); }
    // 是否退出
    auto IsExit() noexcept -> BOOL  { return m_bExit; }
    // 获取窗口句柄
    auto GetHwnd() noexcept { return m_hwnd; }
    // 渲染窗口
    static auto Render(ThisApp* pThis)noexcept->HRESULT;
    // 获取主题颜色
    static auto GetThemeColor(D2D1_COLOR_F&) noexcept->HRESULT;
private:
    // 窗口过程函数
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
    // 窗口句柄
    HWND                        m_hwnd = nullptr;
    // 当前场景
    std::unique_ptr<BaseScene>  m_spNowScene = nullptr;
    // 场景
    const BaseScene*            m_pOldScene = nullptr;
    // 渲染器
    ImageRenderer               m_imageRenderer;
    // 渲染线程
    std::thread                 m_threadRender;
    // 互斥锁
    GameMutex                   m_mutex;
    // 退出
    std::atomic<BOOL>           m_bExit = FALSE;
};