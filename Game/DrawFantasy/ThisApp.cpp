#include "stdafx.h"
#include "included.h"



// 渲染窗口 -- 渲染线程
auto ThisApp::Render(ThisApp* pThis) noexcept ->HRESULT {
    ::CoInitialize(nullptr);
    // 初始化游戏场景
    pThis->m_spNowScene.reset(new(std::nothrow)HelloScene(*pThis));
    if (!pThis->m_spNowScene) return E_OUTOFMEMORY;
    // 刷新一帧
    pThis->m_imageRenderer.OnRender(0);
    // 加锁
    pThis->Lock();
    // 渲染
    while (pThis->m_spNowScene){
        auto scene = std::move(pThis->m_spNowScene);
        pThis->m_pOldScene = scene.get();
        // 运行
        try {
            scene->Run();
        }
        catch (...) {
            break;
        }
    }
    // 解锁
    pThis->Unlock();
    // 强行退出
    if (!pThis->m_bExit) {
        ::PostMessageW(pThis->GetHwnd(), WM_CLOSE, 0, 0);
    }
    ::CoUninitialize();
    return S_OK;
}

// 初始化
auto ThisApp::Initialize(HINSTANCE hInstance, int nCmdShow) noexcept->HRESULT {
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
    wcex.lpszClassName = L"DrawFantasyClass";
    wcex.hIcon = nullptr;
    // 注册窗口
    ::RegisterClassExW(&wcex);
    // 计算窗口大小
    RECT window_rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    DWORD window_style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP ;
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
        wcex.lpszClassName, L"Draw Fantasy", window_style,
        window_rect.left, window_rect.top, window_rect.right, window_rect.bottom, 
        0, 0, hInstance, this
        );
    hr = m_hwnd ? S_OK : E_FAIL;
    // 显示窗口
    if (SUCCEEDED(hr)) {
        hr = KMInput.Init(hInstance, m_hwnd);
        ::ShowWindow(m_hwnd, nCmdShow);
        ::UpdateWindow(m_hwnd);
    }
    // 创建渲染线程
    if (SUCCEEDED(hr)) {
        try {
            m_threadRender.std::thread::~thread();
            m_threadRender.std::thread::thread(Render, this);
        }
        catch (...) {
            hr = E_FAIL;
        }
    }
    return hr;
}



// 消息处理
auto ThisApp::MessageHandle(UINT message, WPARAM wParam, LPARAM lParam, LRESULT & result) -> bool {
    bool wasHandled = false;
    POINT pt = { KMInput.x(), KMInput.y() };
    switch (message)
    {
    case WM_NCMOUSEMOVE:
    case WM_MOUSEMOVE:
        m_mutex.lock();
        m_imageRenderer.OnMouseMove(pt.x, pt.y);
        m_mutex.unlock();
        wasHandled = true;
        break;
    case WM_NCHITTEST:
        m_mutex.lock();
        result = m_imageRenderer.OnNCHitTest(pt.x, pt.y);
        m_mutex.unlock();
        wasHandled = true;
        break;
    case WM_NCLBUTTONUP:
    case WM_LBUTTONUP:
        m_mutex.lock();
        m_imageRenderer.OnLButtonUp(pt.x, pt.y);
        m_mutex.unlock();
        wasHandled = true;
        break;
    case WM_NCLBUTTONDOWN:
    case WM_LBUTTONDOWN:
        m_mutex.lock();
        m_imageRenderer.OnLButtonDown(pt.x, pt.y);
        m_mutex.unlock();
        //wasHandled = true;
        break;
    case WM_SETFOCUS:
        m_mutex.lock();
        m_imageRenderer.RefreshTimer();
        m_imageRenderer.OnSetFocus();
        //m_game.OnMouseMove(-1, -1);
        m_mutex.unlock();
        wasHandled = true;
        break;
    case WM_KILLFOCUS:
        m_mutex.lock();
        m_imageRenderer.OnKillFocus();
        m_mutex.unlock();
        wasHandled = true;
        break;
    case WM_SIZE:
        // 还原
        if (wParam == SIZE_RESTORED) {
            m_imageRenderer.Open();
        }
        wasHandled = true;
        break;
    case WM_DWMCOLORIZATIONCOLORCHANGED:
        // 更新主题颜色
        ThisApp::GetThemeColor(ImageRenderer::s_colorCaption);
        break;
    case WM_CLOSE:
        m_bExit = TRUE;
        m_mutex.lock();
        if (m_spNowScene && !m_spNowScene->IsExitDirectly()) {
            m_bExit = FALSE;
        }
        else {
            m_pOldScene = nullptr;
        }
        m_mutex.unlock();
        // 退出
        if (m_bExit) {
            m_threadRender.join();
            m_imageRenderer.Close();
            ::DestroyWindow(m_hwnd);
            result = 1;
        }
        wasHandled = true;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        result = 1;
        wasHandled = true;
        break;
    }
    return wasHandled;
}





// 消息循环
void ThisApp::RunMessageLoop() noexcept {
    MSG msg;
#if 1
    while (::GetMessageW(&msg, nullptr, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }
#else
    while (true) {
        if (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        else  {
            m_imageRenderer.OnRender(0);
        }
    }
#endif
}

// 窗口过程函数
LRESULT CALLBACK ThisApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    // 创建时 设置指针
    if (message == WM_CREATE) {
        auto pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        auto pOurApp = reinterpret_cast<ThisApp*>(pcs->lpCreateParams);
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pOurApp));
        // 初始化设备
        result = SUCCEEDED(pOurApp->m_imageRenderer.SetHwnd(hwnd)) ? 1 : 0;
        // 刷新一帧
        if (result) pOurApp->m_imageRenderer.OnRender(0);
        // 获取主题颜色
        ThisApp::GetThemeColor(ImageRenderer::s_colorCaption);
    }
    else {
        ThisApp *pOurApp = reinterpret_cast<ThisApp *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(hwnd, GWLP_USERDATA))
            );
        // 处理标记
        bool wasHandled = false;
        if (pOurApp){
            wasHandled = pOurApp->MessageHandle(message, wParam, lParam, result);
        }
        // 默认处理标记
        if (!wasHandled) {
            result = ::DefWindowProcW(hwnd, message, wParam, lParam);
        }
    }
    return result;
}

#include <dwmapi.h>

// 获取主题颜色
auto ThisApp::GetThemeColor(D2D1_COLOR_F& colorf) noexcept -> HRESULT {
    union
    {
        DWORD color;
        uint8_t argb[4];
    };
    color = 0;
#if 1
    auto hr = S_OK;
    DWORD buffer_size = sizeof(DWORD);
    ::RegGetValueA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\DWM",
        "ColorizationColor",
        RRF_RT_DWORD,
        nullptr,
        &color,
        &buffer_size
        );
    DWORD balance = 0;
    buffer_size = sizeof(DWORD);
    ::RegGetValueA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\DWM",
        "ColorizationColorBalance",
        RRF_RT_DWORD,
        nullptr,
        &balance,
        &buffer_size
        );
    {
        /*/ TODO: 修正
        float alpha = float(argb[3]) / 255.f;
        float r = float(argb[2]) / 255.f;
        float g = float(argb[1]) / 255.f;
        float b = float(argb[0]) / 255.f;
        float rr = r * r / (r + g + b) + 1.f;
        float gg = g * g / (r + g + b) + 1.f;
        float bb = b * b / (r + g + b) + 1.f;
        colorf.a = 1.f;
        colorf.r = r * alpha + rr * (1.f - alpha);
        colorf.g = g * alpha + gg * (1.f - alpha);
        colorf.b = b * alpha + bb * (1.f - alpha);
        // 阈值保证
        if (colorf.r > 1.f) colorf.r = 1.f;
        if (colorf.g > 1.f) colorf.g = 1.f;
        if (colorf.b > 1.f) colorf.b = 1.f;
        if (colorf.r < 0.f) colorf.r = 0.f;
        if (colorf.g < 0.f) colorf.g = 0.f;
        if (colorf.b < 0.f) colorf.b = 0.f;*/

        constexpr auto cc = 0xaff2df49ui32;
        constexpr float a = 0.78f; //float((cc >> 24) & 0xFF) / 255.f;
        constexpr float r = float((cc >> 16) & 0xFF) / 255.f;
        constexpr float g = float((cc >>  8) & 0xFF) / 255.f;
        constexpr float b = float((cc >>  0) & 0xFF) / 255.f;
        //---
        constexpr float R = (float(0xA9) / 255.f - r * a) / (1.f - a);
        constexpr float G = (float(0x95) / 255.f - g * a) / (1.f - a);
        constexpr float B = (float(0x0A) / 255.f - b * a) / (1.f - a);
        //---
        constexpr float rr = r * r / (r + g + b);
        constexpr float gg = g * g / (r + g + b);
        constexpr float bb = b * b / (r + g + b);

        auto blend_channel = [](float ch1, float ch2, float prec) {
            register auto data = ch1 + (ch2 - ch1) * prec;
            return data > 1.f ? 1.f : (data < 0.f ? 0.f : data);
        };

        colorf.a = 1.f;
        auto prec = 1.f - float(balance) / 100.f;
        constexpr float basegrey = float(217) / 255.f;
        colorf.r = blend_channel(float(argb[2]) / 255.f, basegrey, prec);
        colorf.g = blend_channel(float(argb[1]) / 255.f, basegrey, prec);
        colorf.b = blend_channel(float(argb[0]) / 255.f, basegrey, prec);
    }
#else
    BOOL aphla_blend = false;
    auto hr = ::DwmGetColorizationColor(&color, &aphla_blend);
    /*if (aphla_blend) {
#if 0
        {
            constexpr auto cc = 0xc7cdbd3eui32;
            constexpr float a = float((cc >> 24) & 0xFF) / 255.f;
            constexpr float r = float((cc >> 16) & 0xFF) / 255.f;
            constexpr float g = float((cc >>  8) & 0xFF) / 255.f;
            constexpr float b = float((cc >>  0) & 0xFF) / 255.f;
            //---
            constexpr float R = (0.9294f - r * a) / (1.f - a);
            constexpr float G = (0.8706f - g * a) / (1.f - a);
            constexpr float B = (0.4118f - b * a) / (1.f - a);
            //---
            constexpr float rr = r * r / (r + g + b);
            constexpr float gg = g * g / (r + g + b);
            constexpr float bb = b * b / (r + g + b);
        }
        {
            constexpr auto cc = 0xc7'b7'48'37ui32;
            constexpr float a = float((cc >> 24) & 0xFF) / 255.f;
            constexpr float r = float((cc >> 16) & 0xFF) / 255.f;
            constexpr float g = float((cc >> 8) & 0xFF) / 255.f;
            constexpr float b = float((cc >> 0) & 0xFF) / 255.f;
            //---
            constexpr float R = (0.8745f - r * a) / (1.f - a);
            constexpr float G = (0.4549f - g * a) / (1.f - a);
            constexpr float B = (0.3961f - b * a) / (1.f - a);
            //---
            constexpr float rr = r * r / (r + g + b);
            constexpr float gg = g * g / (r + g + b);
            constexpr float bb = b * b / (r + g + b);
        }
        {
            constexpr auto cc = 0xc7'49'61'a6ui32;
            constexpr float a = float((cc >> 24) & 0xFF) / 255.f;
            constexpr float r = float((cc >> 16) & 0xFF) / 255.f;
            constexpr float g = float((cc >> 8) & 0xFF) / 255.f;
            constexpr float b = float((cc >> 0) & 0xFF) / 255.f;
            //---
            constexpr float R = (0.4627f - r * a) / (1.f - a);
            constexpr float G = (0.5529f - g * a) / (1.f - a);
            constexpr float B = (0.8078f - b * a) / (1.f - a);
            //---
            constexpr float rr = r * r / (r + g + b);
            constexpr float gg = g * g / (r + g + b);
            constexpr float bb = b * b / (r + g + b);
        }
#endif
        // TODO: 修正
        float alpha = float(argb[3]) / 255.f;
        float r = float(argb[2]) / 255.f;
        float g = float(argb[1]) / 255.f;
        float b = float(argb[0]) / 255.f;
        float rr = r * r / (r + g + b) + 1.f;
        float gg = g * g / (r + g + b) + 1.f;
        float bb = b * b / (r + g + b) + 1.f;
        colorf.a = 1.f;
        colorf.r = r * alpha + rr * (1.f - alpha);
        colorf.g = g * alpha + gg * (1.f - alpha);
        colorf.b = b * alpha + bb * (1.f - alpha);
        // 阈值保证
        if (colorf.r > 1.f) colorf.r = 1.f;
        if (colorf.g > 1.f) colorf.g = 1.f;
        if (colorf.b > 1.f) colorf.b = 1.f;
        if (colorf.r < 0.f) colorf.r = 0.f;
        if (colorf.g < 0.f) colorf.g = 0.f;
        if (colorf.b < 0.f) colorf.b = 0.f;
    }
    else {
        colorf = D2D1::ColorF(color, float(color>>24)/255.f);
    }*/
#endif
    /*{
        auto dll = ::GetModuleHandleW(L"dwmapi");
        struct COLORIZATIONPARAMS  {
            COLORREF        clrColor;           // ColorizationColor
            COLORREF        clrAftGlow;         // ColorizationAfterglow
            UINT            nIntensity;         // ColorizationColorBalance -> 0-100
            UINT            clrAftGlowBal;      // ColorizationAfterglowBalance
            UINT            clrBlurBal;         // ColorizationBlurBalance
            UINT            clrGlassReflInt;    // ColorizationGlassReflectionIntensity
            BOOL            fOpaque;
        } params;
        HRESULT(WINAPI *dwmGetColorizationParameters) (COLORIZATIONPARAMS *colorparam) = nullptr;
        if (dll && (dwmGetColorizationParameters = (decltype(dwmGetColorizationParameters))
            ::GetProcAddress(dll, reinterpret_cast<const char*>(127)))) {
            auto hr = dwmGetColorizationParameters(&params);
            int a = 9;
        }
    }*/
    return hr;
}


// ------------------ EzButton ------------
// EzButton: 构造函数
EzButton::EzButton() noexcept {
}

// 左键点击时
void EzButton::OnMouseLUp(float x, float y) noexcept {
    if (m_oldStatus == Status_Disabled) return;
    if (!::InRect(x, y, this->des_rect)) return;
    m_tarStatus = Status_Hover;
    this->time = this->duration;
    // 调用回调
    if (this->alpha_base > 0.f)
        m_onClick(this, m_clickData);
}

// 左键按下时
void EzButton::OnMouseLDown(float x, float y) noexcept {
    if (m_oldStatus == Status_Disabled) return;
    if (!::InRect(x, y, this->des_rect)) return;
    m_tarStatus = Status_Pushed;
    this->time = this->duration;
}

// EzButton: 鼠标移动
void EzButton::OnMouseMove(float x, float y) noexcept {
    if (m_oldStatus == Status_Disabled) return;
    auto out = ::InRect(m_x, m_y, this->des_rect);
    auto in = ::InRect(x, y, this->des_rect);
    // 在外边移动里面
    if (!out && in) {
        m_tarStatus = Status_Hover;
        this->time = this->duration;
    }
    // 里面移动外面
    else if(out && !in) {
        m_tarStatus = Status_Normal;
        this->time = this->duration;
    }
    // 更新
    m_x = x;
    m_y = y;
}

// EzButton: 渲染
void EzButton::Render(ID2D1DeviceContext* pRenderTarget, float delta) noexcept {
    if (this->time > 0.f) {
        // 减少
        this->time -= delta;
        // 修改状态
        if (this->time <= 0.f) m_oldStatus = m_tarStatus;
        // 修改值
        m_fValue = 1.f - this->time / this -> duration;
        if (m_fValue < 0.f) m_fValue = 0.f;
        if (m_fValue > 1.f) m_fValue = 1.f;
    }
    if (m_tarStatus == Status_Normal) {
        // 渲染
        if (m_fValue != 0.f) {
            pRenderTarget->DrawBitmap(
                m_pBitmap, &this->des_rect, alpha_base,
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
                this->src_rects + m_tarStatus
                );
        }
        pRenderTarget->DrawBitmap(
            m_pBitmap, &this->des_rect, alpha_base * (1.f - m_fValue),
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            this->src_rects + m_oldStatus
            );
    }
    else{
        // 渲染
        if (m_fValue != 1.f) {
            pRenderTarget->DrawBitmap(
                m_pBitmap, &this->des_rect, alpha_base,
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
                this->src_rects + m_oldStatus
                );
        }
        pRenderTarget->DrawBitmap(
            m_pBitmap, &this->des_rect, alpha_base * m_fValue,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            this->src_rects + m_tarStatus
            );
    }
}
// --------- GameButton ----------
// GameButton: 构造函数
GameButton::GameButton(Sprite * s) noexcept : m_pSprite(s) { 
    ZeroMemory(src_rects, sizeof(src_rects)); 
}


// GameButton: 移动构造函数
GameButton::GameButton(GameButton&& btn) noexcept :m_pSprite(btn.m_pSprite){
    ::memcpy(src_rects, btn.src_rects, sizeof(src_rects));
    btn.m_pSprite = nullptr;
}

// GameButton: 刷新
bool GameButton::Update() noexcept {
    assert(m_pSprite && "OH NO!");
    if (!m_pSprite) return false;
    D2D1_POINT_2F pt = {
        static_cast<float>(KMInput.x()),
        static_cast<float>(KMInput.y())
    };
    // 计算
    m_pSprite->TransformPoint(pt);
    // 位置
    D2D1_RECT_F rect = { 
        0.f, 0.f,
        m_pSprite->GetWidth(), m_pSprite->GetHeight()
    };
    // 检查
    uint32_t offset = 0;
    // 按下
    auto down = KMInput.MKeyDown(DIMOFS_BUTTON0);
    auto in = ::InRect(pt.x, pt.y, rect);
    if (in) {
        if (KMInput.MKeyDown(DIMOFS_BUTTON0)) {
            m_bClickIn = true;
        }
        if (KMInput.MPress(DIMOFS_BUTTON0) && m_bClickIn) {
            offset = Status_Pushed;
        }
        else {
            offset = Status_Hover;
        }
    }
    else {
        offset = m_bClickIn ? Status_Pushed : Status_Normal;
    }
    // 触发
    bool click = false;
    if (KMInput.MTrigger(DIMOFS_BUTTON0) && m_bClickIn){
        m_bClickIn = false;
        click = true;
    }
    m_pSprite->src_rect = this->src_rects[offset];
    return click;
}