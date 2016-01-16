#pragma once

#define  _WIN32_WINNT   0x0A01

#include <Windows.h>

#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <cwchar>
#include <cstddef>
#include <atomic>
#include <thread>
#include <new>

#include <dxgi1_4.h>
#include <D3D11.h>
#include <d2d1_3.h>
#include <d2d1_3helper.h>
#include <dwrite_2.h>
#include <wincodec.h>

// DirectComposition 
#ifdef USING_DirectComposition
#   include <dcomp.h>
#endif

// demo
namespace Demo {
    template<class Interface>
    inline void SafeRelease(Interface *&pInterfaceToRelease){
        if (pInterfaceToRelease != nullptr){
            pInterfaceToRelease->Release();
            pInterfaceToRelease = nullptr;
        }
    }
    template <typename Interface>
    inline Interface* SafeAcquire(Interface* newObject)
    {
        if (newObject != nullptr)
            ((IUnknown*)newObject)->AddRef();

        return newObject;
    }
    inline void SafeCloseHandle(HANDLE& handle){
        if (handle){
            ::CloseHandle(handle);
            handle = nullptr;
        }
    }
    // the timer - high
    class HTimer {
    public:
        // QueryPerformanceCounter
        static inline auto QueryPerformanceCounter(LARGE_INTEGER* ll) noexcept {
            auto old = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
            auto r = ::QueryPerformanceCounter(ll);
            ::SetThreadAffinityMask(::GetCurrentThread(), old);
            return r;
        }
        // refresh the frequency
        auto inline RefreshFrequency() noexcept { ::QueryPerformanceFrequency(&m_cpuFrequency); }
        // start timer
        auto inline Start() noexcept { HTimer::QueryPerformanceCounter(&m_cpuCounterStart); }
        // move end var to start var
        auto inline MovStartEnd() noexcept { m_cpuCounterStart = m_cpuCounterEnd; }
        // delta time in sec.
        template<typename T> auto inline Delta_s() noexcept {
            HTimer::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
        // delta time in ms.
        template<typename T> auto inline Delta_ms() noexcept {
            HTimer::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e3) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
        // delta time in micro sec.
        template<typename T> auto inline Delta_mcs() noexcept {
            HTimer::QueryPerformanceCounter(&m_cpuCounterEnd);
            return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e6) / static_cast<T>(m_cpuFrequency.QuadPart);
        }
    private:
        // CPU freq
        LARGE_INTEGER            m_cpuFrequency;
        // CPU start counter
        LARGE_INTEGER            m_cpuCounterStart;
        // CPU end counter
        LARGE_INTEGER            m_cpuCounterEnd;
    public:
        // ctor
        HTimer() noexcept { m_cpuCounterStart.QuadPart = 0; m_cpuCounterEnd.QuadPart = 0; RefreshFrequency(); }
    };

#include <Mmsystem.h>
    // the timer : medium
    class MTimer {
    public:
        // refresh the frequency
        auto inline RefreshFrequency() noexcept { }
        // start timer
        auto inline Start() noexcept { m_dwStart = ::timeGetTime(); }
        // move end var to start var
        auto inline MovStartEnd() noexcept { m_dwStart = m_dwNow; }
        // delta time in sec.
        template<typename T> auto inline Delta_s() noexcept {
            m_dwNow = ::timeGetTime();
            return static_cast<T>(m_dwNow - m_dwStart) * static_cast<T>(0.001);
        }
        // delta time in ms.
        template<typename T> auto inline Delta_ms() noexcept {
            m_dwNow = ::timeGetTime();
            return static_cast<T>(m_dwNow - m_dwStart);
        }
        // delta time in micro sec.
        template<typename T> auto inline Delta_mcs() noexcept {
            m_dwNow = ::timeGetTime();
            return static_cast<T>(m_dwNow - m_dwStart) * static_cast<T>(1000);
        }
    private:
        // start time
        DWORD                   m_dwStart = 0;
        // now time
        DWORD                   m_dwNow = 0;
    public:
        // ctor
        MTimer() noexcept { this->Start(); }
    };
    // renderer
    class ImageRenderer {
    public:
        // 构造函数
        ImageRenderer() noexcept;
        // 析构函数
        ~ImageRenderer() noexcept;
        // 渲染帧
        auto OnRender(UINT syn) noexcept->HRESULT;
        // 设置渲染模式
        void SetSpriteMode() noexcept { m_bSpriteMode = !m_bSpriteMode; }
        // 设置窗口句柄
        auto SetHwnd(HWND hwnd) noexcept { m_hwnd = hwnd; return CreateDeviceIndependentResources(); }
        // 创建设备无关资源
        auto CreateDeviceIndependentResources() noexcept->HRESULT;
        // 创建设备有关资源
        auto CreateDeviceResources() noexcept->HRESULT;
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
            ) noexcept->HRESULT;
    private:
        // 重建fps文本布局
        void recreate_fps_layout() noexcept;
        // 渲染地图
        void render_map() noexcept;
    private:
        // D2D 工厂
        ID2D1Factory4*                      m_pd2dFactory = nullptr;
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
        // 渲染模式
        std::atomic<bool>                   m_bSpriteMode = false;
        // 保留
        bool                                m_unused[7];
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
        ID2D1Device3*                       m_pd2dDevice = nullptr;
        // D2D 设备上下文
        ID2D1DeviceContext3*                m_pd2dDeviceContext = nullptr;
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
        // D2D 精灵集
        ID2D1SpriteBatch*                   m_pSpriteBatch = nullptr;
        // D2D 位图 地图资源集
        ID2D1Bitmap1*                       m_pMapAsset = nullptr;
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
        // 鼠标点击时
        auto OnClick() noexcept { m_oImagaRenderer.SetSpriteMode(); return LRESULT(1); }
    private:
        // 窗口句柄
        HWND                        m_hwnd = nullptr;
        // 渲染器
        ImageRenderer               m_oImagaRenderer;
        // 渲染处理线程
        std::thread                 m_threadRender;
        // 退出
        std::atomic<BOOL>           m_bExit = FALSE;
    };
}