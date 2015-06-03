// ImageRenderer类 主管图形图像渲染

#pragma once

#include <stack>
static constexpr uint32_t MAX_POINT_SIZE = 64;

class ImageRenderer{
public:
    // 构造函数
    ImageRenderer();
    // 析构函数
    ~ImageRenderer();
    // 渲染帧
    HRESULT OnRender(UINT syn);
    // 加锁
    auto Lock() { ::EnterCriticalSection(&m_cs); }
    // 解锁
    auto Unlock() { ::LeaveCriticalSection(&m_cs); }
    // 推送点
    void PushPoint(int x, int y);
    // 设置窗口句柄
    HRESULT SetHwnd(HWND hwnd) {  m_hwnd = hwnd; return CreateDeviceIndependentResources();}
public:
    // 生成几何体
    void MakePath();
    // 生成位图
    void MakeBitmap();
    // 刻画线条
    void DrawLine(int, int, int, int);
    // Push to Stack
    void Push2Stack(int x, int y);
private:
    // 设置像素点
    auto putpixel(int x, int y, UINT32 color) { reinterpret_cast<UINT32&>(m_pBuffer[m_cWidth * y + x]) = color; }
    // 设置像素点
    auto getpixel(int x, int y) { return reinterpret_cast<UINT32&>(m_pBuffer[m_cWidth * y + x]); }
    // 创建设备无关资源
    HRESULT CreateDeviceIndependentResources();
    // 创建设备有关资源
    HRESULT CreateDeviceResources();
    // 丢弃设备有关资源
    void DiscardDeviceResources();
    // 从文件读取位图
    HRESULT LoadBitmapFromFile(ID2D1DeviceContext*, IWICImagingFactory2 *, PCWSTR uri, UINT, UINT, ID2D1Bitmap1 **);
private:
    // D2D 工厂
    ID2D1Factory1*                      m_pd2dFactory = nullptr;
    // WIC 工厂
    IWICImagingFactory2*                m_pWICFactory = nullptr;
    // DWrite工厂
    IDWriteFactory1*                    m_pDWriteFactory = nullptr;
    // 正文文本渲染格式
    IDWriteTextFormat*                  m_pTextFormatMain = nullptr;
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
    IDXGISwapChain1*                    m_pSwapChain = nullptr;
    // D2D 位图 储存当前显示的位图
    ID2D1Bitmap1*                       m_pd2dTargetBimtap = nullptr;
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
    // 临界区
    CRITICAL_SECTION                    m_cs;
    // 黑色笔刷
    ID2D1SolidColorBrush*               m_pBlackBrush = nullptr;
    // 路径几何
    ID2D1PathGeometry*                  m_pNowPath = nullptr;
    // 几何位图
    ID2D1Bitmap*                        m_pNowBitmap = nullptr;
    // 缓冲区
    RGBQUAD*                            m_pBuffer = nullptr;
    // 线段数量
    UINT32                              m_cPointCount = 0;
    // 宽度
    UINT32                              m_cWidth = 0;
    // 高度
    INT32                               m_cHeight = 0;
    // 种子栈
    std::stack<POINT>                   m_stackSeed;
public:
    // 当前点
    D2D1_POINT_2F*                      now_point = points;
    // 起始点
    D2D1_POINT_2F                       points[MAX_POINT_SIZE];
private:
    D2D1_POINT_2F                       SAFE_ZONE = D2D1::Point2F();
};