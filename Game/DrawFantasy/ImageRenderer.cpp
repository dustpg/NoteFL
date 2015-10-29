#include "stdafx.h"
#include "included.h"

// ImageRenderer类构造函数
ImageRenderer::ImageRenderer(ThisApp& m) noexcept :m_app(m) {
    ZeroMemory(m_cache, sizeof(m_cache));
    //
    Sprite::s_pImageRenderer = this;
    m_parameters.DirtyRectsCount = 0;
    m_parameters.pDirtyRects = nullptr;
    m_parameters.pScrollRect = nullptr;
    m_parameters.pScrollOffset = nullptr;
    // 按钮
    //m_captionButtons[Button_Min].Init(Status_Normal);
    m_captionButtons[Button_Max].Init(Status_Disabled);
    //m_captionButtons[Button_Close].Init(Status_Normal);
    //m_pNowScene = std::move(std::unique_ptr<BaseScene>(new(std::nothrow) HelloScene(this)));
    // 初始化标题按钮
    constexpr auto BUTTON_HEIGHT = 20.f;
    constexpr auto MIN_MAX_BUTTON_WIDTH = 26.f;
    constexpr auto CLOSE_BUTTON_WIDTH = 46.f;
    constexpr auto MIN_X = float(ThisApp::WINDOW_WIDTH) - MIN_MAX_BUTTON_WIDTH * 2 -
        CLOSE_BUTTON_WIDTH;
    constexpr auto MAX_X = MIN_X + MIN_MAX_BUTTON_WIDTH;
    constexpr auto CLOSE_X = MIN_X + MIN_MAX_BUTTON_WIDTH * 2;
    // 最小化
    m_captionButtons[Button_Min].des_rect = {
        MIN_X, 0.f, MAX_X, BUTTON_HEIGHT
    };
    m_captionButtons[Button_Max].des_rect = {
        MAX_X, 0.f, CLOSE_X, BUTTON_HEIGHT
    };
    m_captionButtons[Button_Close].des_rect = {
        CLOSE_X, 0.f, float(ThisApp::WINDOW_WIDTH), BUTTON_HEIGHT
    };
    // 设置
    for (int i = 0; i < lengthof(m_captionButtons[Button_Min].src_rects); ++i) {
        auto by = BUTTON_HEIGHT * i;
        m_captionButtons[Button_Min].src_rects[i] = {
            0.f, by, MIN_MAX_BUTTON_WIDTH, BUTTON_HEIGHT + by
        };
    }
    for (int i = 0; i < lengthof(m_captionButtons[Button_Max].src_rects); ++i) {
        auto by = BUTTON_HEIGHT * i;
        m_captionButtons[Button_Max].src_rects[i] = {
            MIN_MAX_BUTTON_WIDTH, by, MIN_MAX_BUTTON_WIDTH * 2, BUTTON_HEIGHT + by
        };
    }
    for (int i = 0; i < lengthof(m_captionButtons[Button_Close].src_rects); ++i) {
        auto by = BUTTON_HEIGHT * i;
        m_captionButtons[Button_Close].src_rects[i] = {
            MIN_MAX_BUTTON_WIDTH * 2, by, MIN_MAX_BUTTON_WIDTH * 2 + CLOSE_BUTTON_WIDTH,
            BUTTON_HEIGHT + by
        };
    }
    // 设置回调 m_pTempBitmap->CopyFromRenderTarget(nullptr, m_pd2dDeviceContext, nullptr);

    // 回调
    m_captionButtons[Button_Min].SetOnClickCall(this, [](void* btn, void* data)->void {
        register auto* thiss = reinterpret_cast<ImageRenderer*>(data);
        thiss->m_pTempBitmap->CopyFromRenderTarget(
            nullptr, thiss->m_pd2dDeviceContext, nullptr
            );
        thiss->MinSize();
        ::ShowWindow(thiss->m_hwnd, SW_MINIMIZE);
    });
    m_captionButtons[Button_Close].SetOnClickCall(this, [](void* btn, void* data)->void {
        register auto* thiss = reinterpret_cast<ImageRenderer*>(data);
        thiss->m_pTempBitmap->CopyFromRenderTarget(
            nullptr, thiss->m_pd2dDeviceContext, nullptr
            );
        ::PostMessageW(thiss->m_hwnd, WM_CLOSE, 0, 0);
    });
}


// 创建设备无关资源
auto ImageRenderer::CreateDeviceIndependentResources() noexcept ->HRESULT {
    // 创建D2D工厂
    D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_NONE };
#ifdef _DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
    HRESULT hr = ::D2D1CreateFactory(
        D2D1_FACTORY_TYPE_MULTI_THREADED,
        IID_ID2D1Factory1,
        &options,
        reinterpret_cast<void**>(&m_pd2dFactory)
        );
    // 多线程
    if(SUCCEEDED(hr)) {
        hr = m_pd2dFactory->QueryInterface(
            IID_ID2D1Multithread,
            reinterpret_cast<void**>(&m_pMultithread)
            );
    }
    // 创建 WIC 工厂.
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_WICImagingFactory2,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory,
            reinterpret_cast<void **>(&m_pWICFactory)
            );
    }
    // 创建 DirectWrite 工厂.
    if (SUCCEEDED(hr)) {
        hr = ::DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }
    // 创建正文文本格式.
    if (SUCCEEDED(hr)) {
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Microsoft YaHei",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            16.f,
            L"en-us",
            &m_pCaptionFormat
            );
        if (m_pCaptionFormat) {
            m_pCaptionFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            m_pCaptionFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        }
    }
    // 创建布局
    if (SUCCEEDED(hr)) {
        const auto str = L"Draw Fantasy";
        hr = m_pDWriteFactory->CreateTextLayout(
            str, ::wcslen(str),
            m_pCaptionFormat,
            float(ThisApp::WINDOW_WIDTH),
            float(CAPTION_HEIGHT),
            &m_pCaptionLayout
            );
    }
    return hr;
}


// 创建设备资源
auto ImageRenderer::CreateDeviceResources() noexcept ->HRESULT {
    HRESULT hr = S_OK;
    // DXGI Surface 后台缓冲
    IDXGISurface*                        pDxgiBackBuffer = nullptr;
    // 创建 D3D11设备与设备上下文 
    if (SUCCEEDED(hr)) {
        // D3D11 创建flag 
        // 一定要有D3D11_CREATE_DEVICE_BGRA_SUPPORT
        // 否则创建D2D设备上下文会失败
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
        // Debug状态 有D3D DebugLayer就可以取消注释
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };
        // 创建设备
        hr = D3D11CreateDevice(
            // 设为空指针选择默认设备
            nullptr,
            // 强行指定硬件渲染
            D3D_DRIVER_TYPE_HARDWARE,
            // 强行指定软件渲染
            //D3D_DRIVER_TYPE_WARP,
            // 没有软件接口
            nullptr,
            // 创建flag
            creationFlags,
            // 欲使用的特性等级列表
            featureLevels,
            // 特性等级列表长度
            lengthof(featureLevels),
            // SDK 版本
            D3D11_SDK_VERSION,
            // 返回的D3D11设备指针
            &m_pd3dDevice,
            // 返回的特性等级
            &m_featureLevel,
            // 返回的D3D11设备上下文指针
            &m_pd3dDeviceContext
            );
    }
#ifdef _DEBUG
    // 创建 ID3D11Debug
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->QueryInterface(IID_PPV_ARGS(&m_pd3dDebug));
    }
#endif
    // 创建 IDXGIDevice
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->QueryInterface(IID_PPV_ARGS(&m_pDxgiDevice));
    }
    // 创建D2D设备
    if (SUCCEEDED(hr)) {
        hr = m_pd2dFactory->CreateDevice(m_pDxgiDevice, &m_pd2dDevice);
    }
    // 创建D2D设备上下文
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_pd2dDeviceContext
            );
    }
    // 获取Dxgi适配器 可以获取该适配器信息
    if (SUCCEEDED(hr)) {
        // 顺带使用像素作为单位
        m_pd2dDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
        hr = m_pDxgiDevice->GetAdapter(&m_pDxgiAdapter);
    }
    // 获取Dxgi工厂
    if (SUCCEEDED(hr)) {
        hr = m_pDxgiAdapter->GetParent(IID_PPV_ARGS(&m_pDxgiFactory));
    }
    // 创建交换链
    if (SUCCEEDED(hr)) {
        RECT rect = { 0 };
        ::GetClientRect(m_hwnd, &rect);
        // 交换链信息
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
        swapChainDesc.Width = rect.right - rect.left;
        swapChainDesc.Height = rect.bottom - rect.top;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.Flags = 0;
#ifdef USING_DirectComposition
        // DirectComposition桌面应用程序
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        // 创建DirectComposition交换链
        hr = m_pDxgiFactory->CreateSwapChainForComposition(
            m_pDxgiDevice,
            &swapChainDesc,
            nullptr,
            &m_pSwapChain
            );
#else
        // 一般桌面应用程序
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        // 利用窗口句柄创建交换链
        hr = m_pDxgiFactory->CreateSwapChainForHwnd(
            m_pd3dDevice,
            m_hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &m_pSwapChain
            );
#endif
    }
    // 确保DXGI队列里边不会超过一帧
    if (SUCCEEDED(hr)) {
        hr = m_pDxgiDevice->SetMaximumFrameLatency(1);
    }
    // 利用交换链获取Dxgi表面
    if (SUCCEEDED(hr)) {
        hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pDxgiBackBuffer));
    }
    // 利用Dxgi表面创建位图
    if (SUCCEEDED(hr)) {
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            96.0f,
            96.0f
            );
        hr = m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(
            pDxgiBackBuffer,
            &bitmapProperties,
            &m_pd2dTargetBimtap
            );
    }
    // 设置
    if (SUCCEEDED(hr)) {
        // 设置 Direct2D 渲染目标
        m_pd2dDeviceContext->SetTarget(m_pd2dTargetBimtap);
        // 使用像素作为单位
        m_pd2dDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
    }
#ifdef USING_DirectComposition
    // 创建直接组合(Direct Composition)设备
    if (SUCCEEDED(hr)) {
        hr = DCompositionCreateDevice(
            /*static_cast<ID2D1Device*>(UIRenderer)*/nullptr,
            IID_PPV_ARGS(&m_pDcompDevice)
            );
    }
    // 创建直接组合(Direct Composition)目标
    if (SUCCEEDED(hr)) {
        hr = m_pDcompDevice->CreateTargetForHwnd(
            m_hwnd, true, &m_pDcompTarget
            );
    }
    // 创建直接组合(Direct Composition)视觉
    if (SUCCEEDED(hr)) {
        hr = m_pDcompDevice->CreateVisual(&m_pDcompVisual);
    }
    // 设置当前交换链为视觉内容
    if (SUCCEEDED(hr)) {
        hr = m_pDcompVisual->SetContent(m_pSwapChain);
    }
    // 设置当前视觉为窗口目标
    if (SUCCEEDED(hr)) {
        hr = m_pDcompTarget->SetRoot(m_pDcompVisual);
    }
    // 向系统提交
    if (SUCCEEDED(hr)) {
        hr = m_pDcompDevice->Commit();
    }
#endif
    // 创建笔刷
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Black),
            &m_pCommonBrush
            );
    }
    // 创建高斯模糊特效
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &m_pGBlur);
    }
    // 创建临时位图
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateBitmap(
            m_pd2dDeviceContext->GetPixelSize(),
            nullptr, 0,
            D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
            ),
            &m_pTempBitmap
            );
    }
    // 读取位图
    if (SUCCEEDED(hr)) {
        hr = this->LoadBitmapFromFile(
            L"resource/iconset.png", &m_pIconSet
            );
    }
    // 设置
    if (SUCCEEDED(hr)) {
        m_captionButtons[Button_Min].SetBitmap(m_pIconSet);
        m_captionButtons[Button_Max].SetBitmap(m_pIconSet);
        m_captionButtons[Button_Close].SetBitmap(m_pIconSet);
    }
    // 创建游戏资源
    if (SUCCEEDED(hr)) {
        //hr = m_pGame->CreateDeviceResources(m_pd2dDeviceContext);
    }
    ::SafeRelease(pDxgiBackBuffer);
    return hr;
}

// ImageRenderer析构函数
ImageRenderer::~ImageRenderer() noexcept{
    // 清空
    m_list.clear();
    this->DiscardDeviceResources();
    // 释放设备无关资源
    ::SafeRelease(m_pd2dFactory);
    ::SafeRelease(m_pWICFactory);
    ::SafeRelease(m_pDWriteFactory);
    ::SafeRelease(m_pCaptionFormat);
    ::SafeRelease(m_pCaptionLayout);
    ::SafeRelease(m_pMultithread);
    // 调试
#ifdef _DEBUG
    if (m_pd3dDebug) {
        m_pd3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    }
    ::SafeRelease(m_pd3dDebug);
#endif
}

// 窗口缩放
auto ImageRenderer::sindow_scale(float ts) noexcept -> float {
    assert(ts > 0.f);
    // 获取大小
    uint32_t width = uint32_t(float(ThisApp::WINDOW_WIDTH) * ts) & (~3ui32);
    uint32_t height = uint32_t(float(ThisApp::WINDOW_HEIGHT) * ts) & (~3ui32);
    // 释放
    m_pd2dDeviceContext->SetTarget(nullptr);
    ::SafeRelease(m_pd2dTargetBimtap);
    ::SafeRelease(m_pTempBitmap);
    auto hr = S_OK;
    ::SetWindowPos(m_hwnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE);
    IDXGISurface*                        pDxgiBackBuffer = nullptr;
    // 缩放
    if (SUCCEEDED(hr)) {
        hr = m_pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
    }
    // 利用交换链获取Dxgi表面
    if (SUCCEEDED(hr)) {
        hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pDxgiBackBuffer));
    }
    // 创建临时位图
    if (SUCCEEDED(hr)) {
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            96.0f,
            96.0f
            );
        hr = m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(
            pDxgiBackBuffer,
            &bitmapProperties,
            &m_pd2dTargetBimtap
            );
    }
    ::SafeRelease(pDxgiBackBuffer);
    // 创建临时位图
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateBitmap(
            m_pd2dDeviceContext->GetPixelSize(),
            nullptr, 0,
            D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
                ),
            &m_pTempBitmap
            );
    }
    return 0.f;
}

// 关闭动画 - 阻塞动画
void ImageRenderer::OpenClose(bool open) noexcept {
    m_bMinSize = !open;
    assert(m_pTempBitmap && "NO!NO!NO!");
    /*// 复制位图
    if (SUCCEEDED(hr)) {
        hr = m_pTempBitmap->CopyFromRenderTarget(nullptr, m_pd2dDeviceContext, nullptr);
        m_timer.MovStartEnd();
    }
    // 清除
    if (SUCCEEDED(hr)) {
        m_pd2dDeviceContext->BeginDraw();
        m_pd2dDeviceContext->Clear(D2D1::ColorF(0x00, 0.f));
        m_pd2dDeviceContext->EndDraw();
        m_pSwapChain->Present(0, 0);
    }*/
    // 进入
    m_pMultithread->Enter();
    // 动画配置
    constexpr float ANIMATION_TIME_CLOSE = 0.2f;
    constexpr uint32_t WINDOW_BILIBILI = 14;
    float time = ANIMATION_TIME_CLOSE;
    // 大循环
    while (time > 0.f) {
        // 计算不透明度
        float opacity = time / ANIMATION_TIME_CLOSE;
        opacity = opacity * opacity;
        if (open) opacity = 1.f - opacity;
        // 计算目标
        D2D1_RECT_F des_rect;
        des_rect.left = float(ThisApp::WINDOW_WIDTH / WINDOW_BILIBILI) * (1.f - opacity);
        des_rect.top = float(ThisApp::WINDOW_HEIGHT / WINDOW_BILIBILI) * (1.f - opacity);
        des_rect.right = float(ThisApp::WINDOW_WIDTH) - des_rect.left;
        des_rect.bottom = float(ThisApp::WINDOW_HEIGHT) - des_rect.top;
        // 渲染
        m_pd2dDeviceContext->BeginDraw();
        m_pd2dDeviceContext->Clear(D2D1::ColorF(0x00, 0.f));
        m_pd2dDeviceContext->DrawBitmap(m_pTempBitmap, &des_rect, opacity);
        m_pd2dDeviceContext->EndDraw();
        m_pSwapChain->Present(1, 0);
        // 更新时间
        time -= m_timer.Delta_s<float>();
        m_timer.MovStartEnd();
    }
    // 离开
    m_pMultithread->Leave();
}

// 丢弃设备相关资源
void ImageRenderer::DiscardDeviceResources() noexcept {
    for (auto i = 0u; i < m_cCacheSize; ++i) {
        m_cache[i].path[0] = 0;
        ::SafeRelease(m_cache[i].bitmap);
    }
    m_cCacheSize = 0;
    // 资源
    ::SafeRelease(m_pGBlur);
    ::SafeRelease(m_pTempBitmap);
    ::SafeRelease(m_pd3dDevice);
    ::SafeRelease(m_pd3dDeviceContext);
    ::SafeRelease(m_pd2dDevice);
    ::SafeRelease(m_pd2dDeviceContext);
    ::SafeRelease(m_pDxgiFactory);
    ::SafeRelease(m_pDxgiDevice);
    ::SafeRelease(m_pCommonBrush);
    ::SafeRelease(m_pDxgiAdapter);
    ::SafeRelease(m_pSwapChain);
    ::SafeRelease(m_pd2dTargetBimtap);
    ::SafeRelease(m_pIconSet);

    // DirectComposition
#ifdef USING_DirectComposition
    SafeRelease(m_pDcompDevice);
    SafeRelease(m_pDcompTarget);
    SafeRelease(m_pDcompVisual);
#endif
}

// 最小化窗口
void ImageRenderer::MinSize() noexcept {
    m_bMinSize = true;
    // 进入
    m_pMultithread->Enter();
    // 动画配置
    constexpr float ANIMATION_TIME_CLOSE = 0.2f;
    constexpr uint32_t WINDOW_BILIBILI = 20;
    float time = ANIMATION_TIME_CLOSE;
    // 大循环
    while (time > 0.f) {
        // 计算不透明度
        float opacity = time / ANIMATION_TIME_CLOSE;
        opacity = opacity * opacity;
        // 计算目标
        D2D1_RECT_F des_rect;
        des_rect.left = 0.f;
        des_rect.top = float(ThisApp::WINDOW_HEIGHT / WINDOW_BILIBILI) * (1.f - opacity);
        des_rect.right = float(ThisApp::WINDOW_WIDTH) ;
        des_rect.bottom = float(ThisApp::WINDOW_HEIGHT) + des_rect.top;
        // 渲染
        m_pd2dDeviceContext->BeginDraw();
        m_pd2dDeviceContext->Clear(D2D1::ColorF(0x00, 0.f));
        m_pd2dDeviceContext->DrawBitmap(m_pTempBitmap, &des_rect, opacity);
        m_pd2dDeviceContext->EndDraw();
        m_pSwapChain->Present(1, 0);
        // 更新时间
        time -= m_timer.Delta_s<float>();
        m_timer.MovStartEnd();
    }
    // 离开
    m_pMultithread->Leave();
}

// 渲染图形图像
auto ImageRenderer::OnRender(UINT syn) noexcept ->float {
    HRESULT hr = S_OK;
    // 没有就创建
    if (!m_pd2dDeviceContext) {
        hr = this->CreateDeviceResources();
        m_timer.Start();
        m_timer.RefreshFrequency();
    }
    // 更新计时器
    auto delta = m_timer.Delta_s<float>() * this->time_scalar;
    // 成功就渲染
    if (SUCCEEDED(hr)) {
        // 设置
        m_pMultithread->Enter();
        {
            // 缩放窗口?
            if (m_fWS != m_fOldWS) {
                this->sindow_scale(m_fOldWS = m_fWS);
            }
            m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Scale(m_fOldWS, m_fOldWS));
        }
        ID2D1CommandList* command = nullptr;
        // 更新模糊
        if (m_fBlur != m_fBlurEnd) {
            auto va = delta / time_scalar * 5.f;
            if (m_fBlur < m_fBlurEnd) {
                m_fBlur += va;
                m_fBlur = std::min(m_fBlur, m_fBlurEnd);
            }
            else {
                m_fBlur -= va * 4.f;
                m_fBlur = std::max(m_fBlur, 0.f);
            }
        }
        // 需要模糊?
        if (m_fBlur) {
            m_pd2dDeviceContext->CreateCommandList(&command);
            m_pd2dDeviceContext->SetTarget(command);
        }
        else {
            m_pd2dDeviceContext->SetTarget(m_pd2dTargetBimtap);
        }
        // 开始渲染
        m_pd2dDeviceContext->BeginDraw();
        // 清屏
        m_pd2dDeviceContext->Clear(D2D1::ColorF(0x00000000, 0.0f));
        m_timer.MovStartEnd();
        // 最小化
        if (!m_bMinSize) {
            // 渲染游戏
            //m_mutex.lock();
            //m_mutex.unlock();
            // 刷新精灵
            for (auto& sprite : m_list) {
                sprite.Updata(delta);
            }
            // 排序
            if (m_bSptNeedSort) {
                m_list.sort(
                    [](const Sprite& sprite1, const Sprite& sprite2) noexcept { return sprite1.z < sprite2.z; }
                );
                m_bSptNeedSort = false;
            }
            D2D1_MATRIX_3X2_F matrix;
            m_pd2dDeviceContext->GetTransform(&matrix);
            // 渲染精灵
            if(command){
                auto last_z = m_list.front().z;
                bool doit = true;
                auto draw_effect = [this](ID2D1CommandList* command) noexcept {
                    // 结束渲染
                    m_pd2dDeviceContext->EndDraw();
                    m_pd2dDeviceContext->SetTarget(m_pd2dTargetBimtap);
                    m_pd2dDeviceContext->BeginDraw();
                    // 渲染
                    command->Close();
                    m_pGBlur->SetInput(0, command);
                    m_pGBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, m_fBlur);
                    D2D1_MATRIX_3X2_F matrix;
                    m_pd2dDeviceContext->GetTransform(&matrix);
                    m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
                    m_pd2dDeviceContext->DrawImage(m_pGBlur);
                    m_pd2dDeviceContext->SetTransform(&matrix);

                };
                // 遍历
                for (auto& sprite : m_list) {
                    if (doit && last_z < m_zBlur && sprite.z >= m_zBlur) {
                        doit = false;
                        draw_effect(command);
                    }
                    last_z = sprite.z;
                    sprite.Render(m_pd2dDeviceContext, matrix);
                }
                // 干他!
                if (doit) {
                    draw_effect(command);
                }
            }
            else {
                for (auto& sprite : m_list) {
                    sprite.Render(m_pd2dDeviceContext, matrix);
                }
            }
            // 回退
            m_pd2dDeviceContext->SetTransform(&matrix);
            // 渲染标题栏
            this->RenderCaption(delta);
            // 渲染边框
            D2D1_RECT_F rect = {
                0.f,
                0.f,
                float(ThisApp::WINDOW_WIDTH),
                float(ThisApp::WINDOW_HEIGHT)
            };
            m_pd2dDeviceContext->PushAxisAlignedClip(&rect, D2D1_ANTIALIAS_MODE_ALIASED);
            m_pCommonBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
            m_pd2dDeviceContext->DrawRectangle(&rect, m_pCommonBrush);
            m_pd2dDeviceContext->PopAxisAlignedClip();
        }
        // 结束渲染
        m_pd2dDeviceContext->EndDraw();
        // 释放
        ::SafeRelease(command);
        // 离开
        m_pMultithread->Leave();
        // 呈现目标
        hr = m_pSwapChain->Present(syn, 0);
    }
    // 设备丢失?
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        ::MessageBoxW(
            m_hwnd,
            L"DXGI_ERROR_DEVICE_REMOVED or DXGI_ERROR_DEVICE_RESET",
            L"Error",
            MB_ICONERROR
            );
        this->DiscardDeviceResources();
        hr = S_FALSE;
    }
    assert(SUCCEEDED(hr));
    return delta;
}


// 从文件读取位图
auto ImageRenderer::LoadBitmapFromFile(
    ID2D1DeviceContext *pRenderTarget,
    IWICImagingFactory2 *pIWICFactory,
    PCWSTR uri,
    UINT width,
    UINT height,
    ID2D1Bitmap1 **ppBitmap
    ) noexcept ->HRESULT {
    IWICBitmapDecoder *pDecoder = nullptr;
    IWICBitmapFrameDecode *pSource = nullptr;
    IWICStream *pStream = nullptr;
    IWICFormatConverter *pConverter = nullptr;
    IWICBitmapScaler *pScaler = nullptr;

    HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
        uri,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &pDecoder
        );

    if (SUCCEEDED(hr)) {
        hr = pDecoder->GetFrame(0, &pSource);
    }
    if (SUCCEEDED(hr)) {
        hr = pIWICFactory->CreateFormatConverter(&pConverter);
    }


    if (SUCCEEDED(hr)) {
        if (width != 0 || height != 0) {
            UINT originalWidth, originalHeight;
            hr = pSource->GetSize(&originalWidth, &originalHeight);
            if (SUCCEEDED(hr))  {
                if (width == 0)
                {
                    FLOAT scalar = static_cast<FLOAT>(height) / static_cast<FLOAT>(originalHeight);
                    width = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
                }
                else if (height == 0)
                {
                    FLOAT scalar = static_cast<FLOAT>(width) / static_cast<FLOAT>(originalWidth);
                    height = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
                }

                hr = pIWICFactory->CreateBitmapScaler(&pScaler);
                if (SUCCEEDED(hr))
                {
                    hr = pScaler->Initialize(
                        pSource,
                        width,
                        height,
                        WICBitmapInterpolationModeCubic
                        );
                }
                if (SUCCEEDED(hr))  {
                    hr = pConverter->Initialize(
                        pScaler,
                        GUID_WICPixelFormat32bppPBGRA,
                        WICBitmapDitherTypeNone,
                        nullptr,
                        0.f,
                        WICBitmapPaletteTypeMedianCut
                        );
                }
            }
        }
        else {
            hr = pConverter->Initialize(
                pSource,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.f,
                WICBitmapPaletteTypeMedianCut
                );
        }
    }
    if (SUCCEEDED(hr)) {
        hr = pRenderTarget->CreateBitmapFromWicBitmap(
            pConverter,
            nullptr,
            ppBitmap
            );
    }

    ::SafeRelease(pDecoder);
    ::SafeRelease(pSource);
    ::SafeRelease(pStream);
    ::SafeRelease(pConverter);
    ::SafeRelease(pScaler);

    return hr;
}

// 读取图片
auto ImageRenderer::LoadBitmapFromFile(PCWSTR uri, ID2D1Bitmap1 ** ppb) noexcept ->HRESULT{
    assert(uri && ppb);
    // 查找缓存信息
    for (auto i = 0u; i < m_cCacheSize; ++i) {
        if (!m_cache[i].bitmap) break;
        if (!std::wcscmp(m_cache[i].path, uri)) {
            *ppb = ::SafeAcquire(m_cache[i].bitmap);
            return S_OK;
        }
    }
    // 再硬盘载入
    auto hr = ImageRenderer::LoadBitmapFromFile(
        m_pd2dDeviceContext, m_pWICFactory, uri, 0, 0, ppb
        );
    // 缓存空间足够
    if (SUCCEEDED(hr) && m_cCacheSize < BITMAP_CACHE_SIZE) {
        std::wcscpy(m_cache[m_cCacheSize].path, uri);
        m_cache[m_cCacheSize].bitmap = ::SafeAcquire(*ppb);
        ++m_cCacheSize;
    }
    return hr;

}

// ------------------

// 鼠标移动时
auto ImageRenderer::OnMouseMove(int x, int y) noexcept ->void {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    for (auto & buton : m_captionButtons) {
        buton.OnMouseMove(fx, fy);
    }
    // 检查标题栏
    D2D1_RECT_F rect = { 0.f, 0.f, float(ThisApp::WINDOW_WIDTH), float(CAPTION_HEIGHT) };
    // 外面移动到里面?
    if (m_bCaptionOut) {
        if (!::InRect(fx, fy, rect)) {
            m_bCaptionOut = false;
            m_captionTime = CAPTION_ANIMATION_TIME;
        }
    }
    // 里面移动到外面?
    else {
        if (::InRect(fx, fy, rect)) {
            m_bCaptionOut = true;
            m_captionTime = CAPTION_ANIMATION_TIME;
        }
    }
}

// 左键弹起
auto ImageRenderer::OnLButtonUp(int x, int y) noexcept -> void {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    for (auto & buton : m_captionButtons) {
        buton.OnMouseLUp(fx, fy);
    }
}

// 左键弹起
auto ImageRenderer::OnLButtonDown(int x, int y) noexcept -> void {
    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);
    for (auto & buton : m_captionButtons) {
        buton.OnMouseLDown(fx, fy);
    }
}

//  Game: 非客户区点击测试
auto ImageRenderer::OnNCHitTest(int x, int y) noexcept -> LRESULT {
    LRESULT result = HTCLIENT;
    // 标题栏?
    if (y < CAPTION_HEIGHT) {
        float fx = static_cast<float>(x);
        float fy = static_cast<float>(y);

        D2D1_RECT_F rect = {
            m_captionButtons[Button_Min].des_rect.left,
            0.f,
            m_captionButtons[Button_Close].des_rect.right,
            m_captionButtons[Button_Min].des_rect.bottom
        };
        if (!::InRect(fx, fy, rect)) {
            result = HTCAPTION;
        }
    }
    return result;
}


// Game: 渲染标题
void ImageRenderer::RenderCaption(float delta) noexcept {
    // 更新
    m_captionValue = m_captionTime / CAPTION_ANIMATION_TIME;
    if (m_bCaptionOut) m_captionValue = 1.f - m_captionValue;
    if (m_captionTime > 0.f) {
        m_captionTime -= delta;
    }
    // 不需要渲染
    if (m_captionValue <= 0.f) return;
    if (m_captionValue > 1.f) m_captionValue = 1.f;
    // 保存变换矩阵
    D2D1_MATRIX_3X2_F old_matrix;
    m_pd2dDeviceContext->GetTransform(&old_matrix);
    m_pd2dDeviceContext->SetTransform(
        D2D1::Matrix3x2F::Translation(0.f, float(CAPTION_HEIGHT) * (m_captionValue - 1.f)) *
        old_matrix
        );
    // 标题栏位置
    D2D1_RECT_F rect = { 0.f, 0.f, float(ThisApp::WINDOW_WIDTH), float(CAPTION_HEIGHT) };
    // 设置笔刷不透明度
    m_pCommonBrush->SetOpacity(m_captionValue);
    // 渲染标题
    m_pCommonBrush->SetColor(D2D1::ColorF(0x808080));
    m_pd2dDeviceContext->FillRectangle(&rect, m_pCommonBrush);
    m_pCommonBrush->SetColor(&m_colorCaption);
    m_pd2dDeviceContext->FillRectangle(&rect, m_pCommonBrush);
    m_pCommonBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
    m_pd2dDeviceContext->DrawTextLayout(
        D2D1::Point2F(),
        m_pCaptionLayout,
        m_pCommonBrush
        );
    // 标题按钮
    for (auto& b : m_captionButtons) {
        b.alpha_base = m_captionValue;
        b.Render(m_pd2dDeviceContext, delta);
    }
    // 图标
    constexpr float ICON_WIDTHF = float(ICON_WIDTH);
    constexpr float ICON_OFFSET = float((CAPTION_HEIGHT - ICON_WIDTH)/2);
    rect = { ICON_OFFSET, ICON_OFFSET, ICON_OFFSET + ICON_WIDTHF,  ICON_OFFSET + ICON_WIDTHF };
    D2D1_RECT_F src_rect = { 256.f - ICON_OFFSET, 0.f, 256.f, ICON_OFFSET };
    m_pd2dDeviceContext->DrawBitmap(
        m_pIconSet, rect, m_captionValue,
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
        &src_rect
        );
    // 设置回来
    m_pCommonBrush->SetOpacity(1.f);
    m_pd2dDeviceContext->SetTransform(&old_matrix);
}

