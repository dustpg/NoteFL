#include "stdafx.h"
#include "included.h"


// ImageRenderer类构造函数
ImageRenderer::ImageRenderer() {
    m_parameters.DirtyRectsCount = 0;
    m_parameters.pDirtyRects = nullptr;
    m_parameters.pScrollRect = nullptr;
    m_parameters.pScrollOffset = nullptr;
    ::InitializeCriticalSection(&m_cs);
}


// 创建设备无关资源
HRESULT ImageRenderer::CreateDeviceIndependentResources() {
    // 创建D2D工厂
    D2D1_FACTORY_OPTIONS options = { D2D1_DEBUG_LEVEL_NONE };
#ifdef _DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
    HRESULT hr = ::D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        IID_ID2D1Factory1,
        &options,
        reinterpret_cast<void**>(&m_pd2dFactory)
        );
    // 创建 WIC 工厂.
    if (SUCCEEDED(hr)) {
        hr = ::CoCreateInstance(
            CLSID_WICImagingFactory2,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_pWICFactory)
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
            L"Arial",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            22.f,
            L"", //locale
            &m_pTextFormatMain
            );
        if (m_pTextFormatMain) {
            m_pTextFormatMain->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            m_pTextFormatMain->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        }
    }
    return hr;
}


// 创建设备资源
HRESULT ImageRenderer::CreateDeviceResources() {
    RECT rect = { 0 }; ::GetClientRect(m_hwnd, &rect);
    rect.right -= rect.left;
    rect.bottom -= rect.top;
    m_cWidth = rect.right;
    //
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
        hr = ::D3D11CreateDevice(
            // 设为空指针选择默认设备
            nullptr,
            // 强行指定硬件渲染
            //D3D_DRIVER_TYPE_HARDWARE,
            // 强行指定WARP渲染
            D3D_DRIVER_TYPE_WARP,
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
        //hr = m_pd3dDevice->QueryInterface(IID_PPV_ARGS(&m_pd3dDebug));
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
        // 交换链信息
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
        swapChainDesc.Width = rect.right;
        swapChainDesc.Height = rect.bottom ;
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
    // 创建笔刷
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);
    }
    // 创建位图
    if (SUCCEEDED(hr)) {
        hr = m_pd2dDeviceContext->CreateBitmap(
            D2D1::SizeU(rect.right, rect.bottom),
            D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
            &m_pNowBitmap
            );
    }
    // 创建缓冲区
    if (!m_pBuffer && (m_pBuffer = reinterpret_cast<decltype(m_pBuffer)>(
        ::malloc(sizeof(*m_pBuffer) * rect.right * rect.bottom)
        ))) {
        // 清空
        if (m_pBuffer) {
            ZeroMemory(m_pBuffer, sizeof(*m_pBuffer) * rect.right * rect.bottom);
        }
        else{
            hr = E_OUTOFMEMORY;
        }
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
    ::SafeRelease(pDxgiBackBuffer);
    return hr;
}

// ImageRenderer析构函数
ImageRenderer::~ImageRenderer(){
    this->DiscardDeviceResources();

    ::SafeRelease(m_pd2dFactory);
    ::SafeRelease(m_pWICFactory);
    ::SafeRelease(m_pDWriteFactory);
    ::SafeRelease(m_pTextFormatMain);
    ::SafeRelease(m_pNowPath);
    // 调试
#ifdef _DEBUG
    if (m_pd3dDebug) {
        m_pd3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    }
    ::SafeRelease(m_pd3dDebug);
#endif
    if (m_pBuffer) {
        ::free(m_pBuffer);
        m_pBuffer = nullptr;
    }
    ::DeleteCriticalSection(&m_cs);

}

// 推送点
void ImageRenderer::PushPoint(int x, int y) {
    this->Lock();
    if (now_point) {
        now_point->x = static_cast<float>(x);
        now_point->y = static_cast<float>(y);
        ++now_point;
        ++m_cPointCount;
        if (now_point >= points + MAX_POINT_SIZE) {
            now_point = nullptr;
        }
    }
    this->Unlock();
}

// 丢弃设备相关资源
void ImageRenderer::DiscardDeviceResources(){
    ::SafeRelease(m_pBlackBrush);
    ::SafeRelease(m_pNowBitmap);

    ::SafeRelease(m_pd3dDevice);
    ::SafeRelease(m_pd3dDeviceContext);
    ::SafeRelease(m_pd2dDevice);
    ::SafeRelease(m_pd2dDeviceContext);
    ::SafeRelease(m_pDxgiFactory);
    ::SafeRelease(m_pDxgiDevice);
    ::SafeRelease(m_pDxgiAdapter);
    ::SafeRelease(m_pSwapChain);
    ::SafeRelease(m_pd2dTargetBimtap);

    // DirectComposition
#ifdef USING_DirectComposition
    ::SafeRelease(m_pDcompDevice);
    ::SafeRelease(m_pDcompTarget);
    ::SafeRelease(m_pDcompVisual);
#endif
}

// 渲染图形图像
HRESULT ImageRenderer::OnRender(UINT syn){
    HRESULT hr = S_OK;
    // 没有就创建
    if (!m_pd2dDeviceContext) {
        hr = this->CreateDeviceResources();
    }
    this->Lock();
    auto tnow_point = this->now_point;
    auto tpoints = this->points;
    this->Unlock();
    // 成功就渲染
    if (SUCCEEDED(hr)) {
        // 开始渲染
        m_pd2dDeviceContext->BeginDraw();
        // 重置转换
        m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
        // 清屏
        m_pd2dDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::White));
        // 正式刻画.........
        if (tnow_point != tpoints) {
            auto i = tpoints;
            for (; i < tnow_point; ++i) {
                m_pd2dDeviceContext->DrawLine(i[0], i[1], m_pBlackBrush);
            }
            m_pd2dDeviceContext->DrawLine(*i, *tpoints, m_pBlackBrush);
        }
        if (!tnow_point) {
            // 检查是否存在路径几何
            if (!m_pNowPath) {
                MakePath();
                MakeBitmap();
            }
            D2D1_RECT_F rect = { 0.f };
            reinterpret_cast<D2D1_SIZE_F&>(rect.right) = m_pd2dDeviceContext->GetSize();
            auto rect2 = rect;
            rect.right *= 0.5f;
            rect2.left = rect.right;
            // 左边显示Direct2D
            m_pd2dDeviceContext->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            if (m_pNowPath)   m_pd2dDeviceContext->FillGeometry(m_pNowPath, m_pBlackBrush);
            m_pd2dDeviceContext->PopAxisAlignedClip();
            // 右边显示边缘填充
            m_pd2dDeviceContext->PushAxisAlignedClip(rect2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            m_pd2dDeviceContext->DrawBitmap(m_pNowBitmap);
            m_pd2dDeviceContext->PopAxisAlignedClip();
            // 中间分界线
            m_pBlackBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
            m_pd2dDeviceContext->DrawLine(
                D2D1::Point2F(rect2.left),
                D2D1::Point2F(rect2.left, rect2.bottom),
                m_pBlackBrush,
                2.f
                );
            m_pBlackBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        }
        // 结束渲染
        m_pd2dDeviceContext->EndDraw();
        // 呈现目标
        hr = m_pSwapChain->Present(syn, 0);
    }
    // 设备丢失?
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        this->DiscardDeviceResources();
        hr = S_FALSE;
    }
    return hr;
}


#define to_int(a) static_cast<int>(a)
#define to_float(a) static_cast<float>(a)
#include <algorithm>

// 边缘填充
void ImageRenderer::EdgeFilling() {
    points[m_cPointCount + 1] = points[0];
    int x0, y0, x1, y1;
    // 查找最小值
    // 这不是必须的, 优化的话在PushPoint那里可以优化(记录/更新这些点)
    int right = static_cast<int>(std::max_element(points, points + m_cPointCount + 1,
        [](D2D1_POINT_2F& p1, D2D1_POINT_2F& p2)->bool {return p1.x < p2.x; }
    )->x);
    //
    int dx, dy, eps1, k, p, q, flag, temp;
    float x, y, xIncre, yIncre;
    for (size_t i = 0; i <= m_cPointCount; i++) {
        x0 = to_int(points[i].x);
        y0 = to_int(points[i].y);
        x1 = to_int(points[i + 1].x);
        y1 = to_int(points[i + 1].y);
        //保证(x0,y0)在(x1,y1)的下面  
        if (y0 > y1) {
            temp = y0;
            y0 = y1;
            y1 = temp;
            temp = x0;
            x0 = x1;
            x1 = temp;
        }
        dx = x1 - x0;
        dy = y1 - y0;
        x = to_float(x0);
        y = to_float(y0);
        //注意这里和DDA算法不同，yIncre=1  
        //if(abs(dx)>abs(dy))  
        //eps1=abs(dx);  
        //else  
        eps1 = abs(dy);
        xIncre = (float)dx / (float)eps1;
        yIncre = (float)dy / (float)eps1;
        flag = 1;
        for (k = 0; k <= eps1; k++) {
            p = (int)(x + 0.5);
            q = (int)(y + 0.5);
            //每条直线的上端点不处理，避免交点处处理两次，没达到填充的效果  
            if (k != 0) {
                //putpixel(p, q);
                DDA(p, q, right, q);
            }
            x += xIncre;
            y += yIncre;
        }
    }

    //
}


// ImageRenderer
void ImageRenderer::DDA(int x0, int y0, int x1, int y1) {
    auto putpixel = [this](int x, int y, UINT32 color) {
        reinterpret_cast<UINT32&>(m_pBuffer[m_cWidth * y + x]) = color;
    };
    auto getpixel = [this](int x, int y) {
        return reinterpret_cast<UINT32&>(m_pBuffer[m_cWidth * y + x]);
    };
    //
    int dx, dy, eps1, k;
    float x, y, xIncre, yIncre;
    int p, q;
    dx = x1 - x0;
    dy = y1 - y0;
    x = to_float(x0);
    y = to_float(y0);
    if (abs(dx) > abs(dy))
        eps1 = abs(dx);
    else
        eps1 = abs(dy);
    xIncre = (float)dx / (float)eps1;
    yIncre = (float)dy / (float)eps1;
    for (k = 0; k <= eps1; k++)
    {
        p = (int)(x + 0.5);
        q = (int)(y + 0.5);
        if (getpixel(p, q) == 0x00)
            putpixel(p, q, 0xFF000000);
        else
            putpixel(p, q, 0x00);
        x += xIncre;
        y += yIncre;
    }
}

//  创建路径几何
void ImageRenderer::MakePath() {
    SafeRelease(m_pNowPath);
    m_pd2dFactory->CreatePathGeometry(&m_pNowPath);
    if (m_pNowPath) {
        ID2D1GeometrySink* sink = nullptr;
        HRESULT hr = m_pNowPath->Open(&sink);
        if (SUCCEEDED(hr)) {
            sink->BeginFigure(points[0], D2D1_FIGURE_BEGIN_FILLED);
            sink->AddLines(points + 1, m_cPointCount);
            sink->EndFigure(D2D1_FIGURE_END_CLOSED);
            sink->Close();
        }
        SafeRelease(sink);
    }
}

// 创建位图
void ImageRenderer::MakeBitmap() {
    this->EdgeFilling();
    // 载入
    RECT rc; ::GetClientRect(m_hwnd, &rc);
    m_pNowBitmap->CopyFromMemory(nullptr, m_pBuffer, sizeof(*m_pBuffer)*m_cWidth);
    // 清理
    ZeroMemory(m_pBuffer, (rc.right - rc.left)*(rc.bottom - rc.top) * sizeof(*m_pBuffer));
}



// 从文件读取位图
HRESULT ImageRenderer::LoadBitmapFromFile(
    ID2D1DeviceContext *pRenderTarget,
    IWICImagingFactory2 *pIWICFactory,
    PCWSTR uri,
    UINT width,
    UINT height,
    ID2D1Bitmap1 **ppBitmap
    ){
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
            if (SUCCEEDED(hr)) {
                if (width == 0) {
                    FLOAT scalar = static_cast<FLOAT>(height) / static_cast<FLOAT>(originalHeight);
                    width = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
                }
                else if (height == 0) {
                    FLOAT scalar = static_cast<FLOAT>(width) / static_cast<FLOAT>(originalWidth);
                    height = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
                }

                hr = pIWICFactory->CreateBitmapScaler(&pScaler);
                if (SUCCEEDED(hr)) {
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
    if (SUCCEEDED(hr))  {
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

