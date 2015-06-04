// ImageRenderer类 主管图形图像渲染

#pragma once


// 图像渲染器
class ImageRenderer{
    // 友元声明
    friend class Sprite;
public:
    // 构造函数
    ImageRenderer(ThisApp&)noexcept;
    // 析构函数
    ~ImageRenderer() noexcept;
    // 打开/关闭动画 - 阻塞动画
    void OpenClose(bool) noexcept;
    // 打开
    auto Open() noexcept { this->OpenClose(true); }
    // 关闭
    auto Close() noexcept { this->OpenClose(false); }
    // 最小化窗口
    void MinSize() noexcept;
    // 渲染帧
    auto OnRender(UINT syn)noexcept ->float;
    // 设置窗口句柄
    auto SetHwnd(HWND hwnd)noexcept { m_hwnd = hwnd; return this->CreateDeviceIndependentResources();}
public:
    // 标题栏高度
    static constexpr uint32_t CAPTION_HEIGHT = 32;
    // 边框厚度
    static constexpr uint32_t BORDER_THICKNESS = 4;
    // 图标宽度
    static constexpr uint32_t ICON_WIDTH = 16;
    // 主题颜色
    static D2D1_COLOR_F s_colorCaption;
    // 标题栏按钮
    enum CaptionButton : uint32_t {
        Button_Min,     // 最小化
        Button_Max,     // 最大化
        Button_Close,   // 关闭
        CBUTTON_SIZE,
    };
    // 渲染标题
    void RenderCaption(float) noexcept;
    // 鼠标移动
    auto OnMouseMove(int x, int y) noexcept ->void;
    // 鼠标移动左键弹起
    auto OnLButtonUp(int x, int y) noexcept ->void;
    // 鼠标移动左键按下
    auto OnLButtonDown(int x, int y) noexcept ->void;
    // 非客户区点击测试
    auto OnNCHitTest(int x, int y) noexcept->LRESULT;
    // 更新计时器
    auto RefreshTimer() noexcept { return m_timer.RefreshFrequency(); }
    // 显示标题
    auto ShowCaption() noexcept { m_captionTime = CAPTION_ANIMATION_TIME; m_bCaptionOut = true; }
    // 设置焦点
    auto OnSetFocus() noexcept {  m_colorCaption = s_colorCaption; }
    // 失去焦点
    auto OnKillFocus() noexcept { static auto constexpr color = 0xEBEBEB; m_colorCaption = D2D1::ColorF(color); }
private:
    // 创建设备无关资源
    auto CreateDeviceIndependentResources() noexcept->HRESULT;
    // 创建设备有关资源
    auto CreateDeviceResources() noexcept->HRESULT;
    // 丢弃设备有关资源
    void DiscardDeviceResources() noexcept;
    // 从文件读取位图
    auto LoadBitmapFromFile(ID2D1DeviceContext*, IWICImagingFactory2 *, PCWSTR uri, UINT, UINT, ID2D1Bitmap1 **) noexcept ->HRESULT;
public:
    // 读取图片文件
    auto LoadBitmapFromFile(PCWSTR uri, ID2D1Bitmap1 ** ppb) noexcept {
        return ImageRenderer::LoadBitmapFromFile(m_pd2dDeviceContext, m_pWICFactory, uri, 0, 0, ppb);
    }
private:
    // 本程序
    ThisApp&                        m_app;
    // D2D 工厂
    ID2D1Factory1*                  m_pd2dFactory = nullptr;
    // WIC 工厂
    IWICImagingFactory2*            m_pWICFactory = nullptr;
    // DWrite工厂
    IDWriteFactory1*                m_pDWriteFactory = nullptr;
    // 标题文本格式
    IDWriteTextFormat*              m_pCaptionFormat = nullptr;
    // 标题文本
    IDWriteTextLayout*              m_pCaptionLayout = nullptr;
    // 多线程
    ID2D1Multithread*               m_pMultithread = nullptr;
private:
    // D3D 设备
    ID3D11Device*                   m_pd3dDevice = nullptr;
    // D3D 设备上下文
    ID3D11DeviceContext*            m_pd3dDeviceContext = nullptr;
    // D2D 设备
    ID2D1Device*                    m_pd2dDevice = nullptr;
    // D2D 设备上下文
    ID2D1DeviceContext*             m_pd2dDeviceContext = nullptr;
    // DXGI 工厂
    IDXGIFactory2*                  m_pDxgiFactory = nullptr;
    // DXGI 设备
    IDXGIDevice1*                   m_pDxgiDevice = nullptr;
    // DXGI 适配器
    IDXGIAdapter*                   m_pDxgiAdapter = nullptr;
#ifdef _DEBUG
    // 调试对象
    ID3D11Debug*                    m_pd3dDebug = nullptr;
#endif
    // 类公共笔刷
    ID2D1SolidColorBrush*           m_pCommonBrush = nullptr;
    // DXGI 交换链
    IDXGISwapChain1*                m_pSwapChain = nullptr;
    // D2D 位图 储存当前显示的位图
    ID2D1Bitmap1*                   m_pd2dTargetBimtap = nullptr;
    // D2D 位图 临时位图
    ID2D1Bitmap1*                   m_pTempBitmap = nullptr;
#ifdef USING_DirectComposition
    // Direct Composition Device
    IDCompositionDevice*            m_pDcompDevice = nullptr;
    // Direct Composition Target
    IDCompositionTarget*            m_pDcompTarget = nullptr;
    // Direct Composition Visual
    IDCompositionVisual*            m_pDcompVisual = nullptr;
#endif
    // 窗口句柄
    HWND                            m_hwnd = nullptr;
    // 图标集
    ID2D1Bitmap1*                   m_pIconSet = nullptr;
    // 计时器
    UITimer                         m_timer;
    //  所创设备特性等级
    D3D_FEATURE_LEVEL               m_featureLevel;
    // 手动交换链
    DXGI_PRESENT_PARAMETERS         m_parameters;
private:
    // 标题栏颜色
    D2D1_COLOR_F                    m_colorCaption = D2D1::ColorF(D2D1::ColorF::White);
    // 标题栏值
    float                           m_captionValue = 0.f;
    // 事件
    float                           m_captionTime = 0.f;
    // 需要排序
    bool                            m_bSptNeedSort = false;
    // 标题渐出
    bool                            m_bCaptionOut = false;
    // 最小化
    bool                            m_bMinSize = false;
    //
    bool                            m_bUnused[5];
private:
    // 精灵链表
    SpriteList                      m_list;
    // 标题栏按钮
    EzButton                        m_captionButtons[CBUTTON_SIZE];
public: // 精灵相关
    // 需要排序
    auto NeedSort() noexcept { m_bSptNeedSort = true; }
    // 新的精灵
    auto NewSprite() /*throw(std::bad_alloc&)*/ {
        Sprite sprite;
        m_list.push_back(std::move(sprite));
        return &m_list.back();
    }
    // 释放精灵
    auto ReleaseSprite(const Sprite* sprite) noexcept {
        for (auto itr = m_list.begin(); itr != m_list.end(); ++itr) {
            if (&static_cast<const Sprite&>(*itr) == sprite) {
                try {
                    m_list.erase(itr);
                }
                catch (...) {
                    // TODO: Exceptional Handling
                }
                return;
            }
        }
    }
};