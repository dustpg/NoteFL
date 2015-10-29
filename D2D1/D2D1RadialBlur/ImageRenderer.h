// ImageRenderer类 主管图形图像渲染
#pragma once

class ImageRenderer {
public:
    // 构造函数
    ImageRenderer() noexcept;
    // 析构函数
    ~ImageRenderer() noexcept;
    // 渲染帧
    auto OnRender(UINT syn) noexcept ->HRESULT;
    // 设置窗口句柄
    auto SetHwnd(HWND hwnd) noexcept {  m_hwnd = hwnd; return CreateDeviceIndependentResources();}
    // 创建设备无关资源
    auto CreateDeviceIndependentResources() noexcept ->HRESULT;
    // 创建设备有关资源
    auto CreateDeviceResources() noexcept ->HRESULT;
    // 丢弃设备有关资源
    void DiscardDeviceResources() noexcept;
    // 从文件读取位图
    static auto LoadBitmapFromFile(
        ID2D1DeviceContext* IN pRenderTarget, 
        IWICImagingFactory2* IN pIWICFactory, 
        PCWSTR IN uri, 
        UINT OPTIONAL width, 
        UINT OPTIONAL height,
        ID2D1Bitmap1** OUT ppBitmap
        ) noexcept ->HRESULT;
private:
    // 重建fps文本布局
    void recreate_fps_layout() noexcept;
    // 设置径向模糊参数
    void config_blur_properties() noexcept;
private:
    // D2D 工厂
    ID2D1Factory1*                      m_pd2dFactory = nullptr;
    // WIC 工厂
    IWICImagingFactory2*                m_pWICFactory = nullptr;
    // DWrite工厂
    IDWriteFactory1*                    m_pDWriteFactory = nullptr;
    // 正文文本渲染格式
    IDWriteTextFormat*                  m_pTextFormatMain = nullptr;
    // FPS 显示
    IDWriteTextLayout*                  m_pFPSLayout = nullptr;
    // 高精度计时器
    HTimer                              m_oTimerH;
    // 中精度计时器
    MTimer                              m_oTimerM;
    // 帧计数器
    uint32_t                            m_cFrameCount = 0;
    // 帧结算单位
    uint32_t                            m_cRefreshCount = 30;
    // 间隔时间
    float                               m_fDelta = 0.f;
    // 平均 FPS
    float                               m_fFramePerSec = 1.f;
private:
    // D3D 设备
    ID3D11Device*                       m_pd3dDevice = nullptr;
    // D3D 设备上下文
    ID3D11DeviceContext*                m_pd3dDeviceContext = nullptr;
    // D2D 设备
    ID2D1Device*                        m_pd2dDevice = nullptr;
    // D2D 设备上下文
    ID2D1DeviceContext*                 m_pd2dDeviceContext = nullptr;
    // DXGI 工厂
    IDXGIFactory2*                      m_pDxgiFactory = nullptr;
    // DXGI 设备
    IDXGIDevice1*                       m_pDxgiDevice = nullptr;
    // DXGI 适配器
    IDXGIAdapter*                       m_pDxgiAdapter = nullptr;
#ifdef _DEBUG
    // 调试对象
    ID3D11Debug*                        m_pd3dDebug = nullptr;
#endif
    // DXGI 交换链
    IDXGISwapChain2*                    m_pSwapChain = nullptr;
    // D2D 位图 储存当前显示的位图
    ID2D1Bitmap1*                       m_pd2dTargetBimtap = nullptr;
    // 纯色笔刷
    ID2D1SolidColorBrush*               m_pBaiscBrush = nullptr;
    // 测试位图
    ID2D1Bitmap1*                       m_pTestBitmap = nullptr;
    // 径向模糊特效
    ID2D1Effect*                        m_pRadialBlurEffect = nullptr;
    // 径向模糊特效 输出接口
    ID2D1Image*                         m_pRadialBlurOutput = nullptr;
#ifdef USING_DirectComposition
    // Direct Composition Device
    IDCompositionDevice*                m_pDcompDevice = nullptr;
    // Direct Composition Target
    IDCompositionTarget*                m_pDcompTarget = nullptr;
    // Direct Composition Visual
    IDCompositionVisual*                m_pDcompVisual = nullptr;
#endif
    // 窗口句柄
    HWND                                m_hwnd = nullptr;
    //  所创设备特性等级
    D3D_FEATURE_LEVEL                   m_featureLevel;
    // 手动交换链
    DXGI_PRESENT_PARAMETERS             m_parameters;
};