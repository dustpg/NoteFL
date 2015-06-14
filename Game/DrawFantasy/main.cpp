#include "stdafx.h"
#include "included.h"


// 应用程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
#ifdef _DEBUG
    ::AllocConsole();
    ::_cwprintf(L"Battle Control  Online! \n");
#endif
    if (SUCCEEDED(::CoInitialize(nullptr)) && SUCCEEDED(AudioEngine.Initialize())) {
        {
            ThisApp app;
            if (SUCCEEDED(app.Initialize(hInstance, nCmdShow)))
            {
                app.RunMessageLoop();
            }
        }
        AudioEngine.UnInitialize();
        ::CoUninitialize();
    }
#ifdef _DEBUG
    ::_cwprintf(L"Battle Control Terminated! \n");
    ::FreeConsole();
#endif
    return 0;
}

void tt() {
    {
        constexpr double a = double(0x79) / double(0xFF);
        constexpr double b = double(0xB3) / double(0xFF);
        constexpr double c = double(0x36) / double(0xFF);
    }
    {
        constexpr double a = double(167) / double(0xFF);
        constexpr double b = double(197) / double(0xFF);
        constexpr double c = double(132) / double(0xFF);
    }
    {
        constexpr double alpha = 0.4902;
        constexpr double R = (0.6549 - (0.4745 * alpha)) / (1.0 - alpha);
        constexpr double G = (0.7525 - (0.7020 * alpha)) / (1.0 - alpha);
        constexpr double B = (0.5176 - (0.2118 * alpha)) / (1.0 - alpha);
    }
    {
        constexpr double alpha = 0.6863;
        constexpr double R = (0.9294 - (0.9490 * alpha)) / (1.0 - alpha);
        constexpr double G = (0.8706 - (0.8745 * alpha)) / (1.0 - alpha);
        constexpr double B = (0.4115 - (0.2862 * alpha)) / (1.0 - alpha);
    }
    for (uint32_t i = 0; i < 0x10000ui32; ++i) {
        double alpha1 = double((i & 0xFF00ui32) >> 8) / 255.0;
        double alpha2 = double(i & 0xFFui32) / 255.0;
        //
        double R1 = (0.6549 - (0.4745 * alpha1)) / (1.0 - alpha1);
        double G1 = (0.7525 - (0.7020 * alpha1)) / (1.0 - alpha1);
        double B1 = (0.5176 - (0.2118 * alpha1)) / (1.0 - alpha1);
        //
        double R2 = (0.9294 - (0.9490 * alpha2)) / (1.0 - alpha2);
        double G2 = (0.8706 - (0.8745 * alpha2)) / (1.0 - alpha2);
        double B2 = (0.4115 - (0.2862 * alpha2)) / (1.0 - alpha2);
        //
        if (std::abs(R1 - R2) < 0.03 && std::abs(G1 - G2) < 0.03) {
            if (std::abs(B1 - B2) < 0.03) {
                int a = 9;
            }
        }
    }
}
//
ImageRenderer* Sprite::s_pImageRenderer = nullptr;
D2D1_COLOR_F ImageRenderer::s_colorCaption = D2D1::ColorF(0xEBEBEB);

// 输入单例
GameKeyboardMouseInput KMInput;

#pragma comment(lib, "dxguid")
#pragma comment(lib, "dinput8")
#pragma comment(lib, "dwmapi")
#ifdef _DEBUG
#pragma comment(lib, "../../Debug/libvorbis")
#pragma comment(lib, "../../Debug/libogg")
#pragma comment(lib, "../../Debug/WrapAL")
#else
#pragma comment(lib, "../../Release/libvorbis")
#pragma comment(lib, "../../Release/libogg")
#pragma comment(lib, "../../Release/WrapAL")
#endif

// --


// -----------------------------



// π
#define EZ_PI 3.1415296F
// 二分之一π
#define EZ_PI_2 1.5707963F

// 反弹渐出
float inline __fastcall BounceEaseOut(float p) noexcept {
    if (p < 4.f / 11.f) {
        return (121.f * p * p) / 16.f;
    }
    else if (p < 8.f / 11.f) {
        return (363.f / 40.f * p * p) - (99.f / 10.f * p) + 17.f / 5.f;
    }
    else if (p < 9.f / 10.f) {
        return (4356.f / 361.f * p * p) - (35442.f / 1805.f * p) + 16061.f / 1805.f;
    }
    else {
        return (54.f / 5.f * p * p) - (513.f / 25.f * p) + 268.f / 25.f;
    }
}


// CUIAnimation 缓动函数
float __fastcall EasingFunction(AnimationType type, float p) noexcept {
    assert((p >= 0.f && p <= 1.f) && "bad argument");
    switch (type)
    {
    default:
        assert(!"type unknown");
        __fallthrough;
    case AnimationType::Type_LinearInterpolation:
        // 线性插值     f(x) = x
        return p;
    case AnimationType::Type_QuadraticEaseIn:
        // 平次渐入     f(x) = x^2
        return p * p;
    case AnimationType::Type_QuadraticEaseOut:
        // 平次渐出     f(x) =  -x^2 + 2x
        return -(p * (p - 2.f));
    case AnimationType::Type_QuadraticEaseInOut:
        // 平次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^2)
        // [0.5, 1.f]   f(x) = -(1/2)((2x-1)*(2x-3)-1) ; 
        return p < 0.5f ? (p * p * 2.f) : ((-2.f * p * p) + (4.f * p) - 1.f);
    case AnimationType::Type_CubicEaseIn:
        // 立次渐入     f(x) = x^3;
        return p * p * p;
    case AnimationType::Type_CubicEaseOut:
        // 立次渐出     f(x) = (x - 1)^3 + 1
    {
        register float f = p - 1.f;
        return f * f * f + 1.f;
    }
    case AnimationType::Type_CubicEaseInOut:
        // 立次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^3) 
        // [0.5, 1.f]   f(x) = (1/2)((2x-2)^3 + 2) 
        if (p < 0.5f) {
            return p * p * p * 2.f;
        }
        else {
            register float f = (2.f * p) - 2.f;
            return 0.5f * f * f * f + 1.f;
        }
    case AnimationType::Type_QuarticEaseIn:
        // 四次渐入     f(x) = x^4
    {
        register float f = p * p;
        return f * f;
    }
    case AnimationType::Type_QuarticEaseOut:
        // 四次渐出     f(x) = 1 - (x - 1)^4
    {
        register float f = (p - 1.f); f *= f;
        return 1.f - f * f;
    }
    case AnimationType::Type_QuarticEaseInOut:
        // 四次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^4)
        // [0.5, 1.f]   f(x) = -(1/2)((2x-2)^4 - 2)
        if (p < 0.5f) {
            register float f = p * p;
            return 8.f * f * f;
        }
        else {
            register float f = (p - 1.f); f *= f;
            return 1.f - 8.f * f * f;
        }
    case AnimationType::Type_QuinticEaseIn:
        // 五次渐入     f(x) = x^5
    {
        register float f = p * p;
        return f * f * p;
    }
    case AnimationType::Type_QuinticEaseOut:
        // 五次渐出     f(x) = (x - 1)^5 + 1
    {
        register float f = (p - 1.f);
        return f * f * f * f * f + 1.f;
    }
    case AnimationType::Type_QuinticEaseInOut:
        // 五次出入
        // [0, 0.5)     f(x) = (1/2)((2x)^5) 
        // [0.5, 1.f]   f(x) = (1/2)((2x-2)^5 + 2)
        if (p < 0.5) {
            register float f = p * p;
            return 16.f * f * f * p;
        }
        else {
            register float f = ((2.f * p) - 2.f);
            return  f * f * f * f * f * 0.5f + 1.f;
        }
    case AnimationType::Type_SineEaseIn:
        // 正弦渐入     
        return ::sin((p - 1.f) * EZ_PI_2) + 1.f;
    case AnimationType::Type_SineEaseOut:
        // 正弦渐出     
        return ::sin(p * EZ_PI_2);
    case AnimationType::Type_SineEaseInOut:
        // 正弦出入     
        return 0.5f * (1.f - ::cos(p * EZ_PI));
    case AnimationType::Type_CircularEaseIn:
        // 四象圆弧
        return 1.f - ::sqrt(1.f - (p * p));
    case AnimationType::Type_CircularEaseOut:
        // 二象圆弧
        return ::sqrt((2.f - p) * p);
    case AnimationType::Type_CircularEaseInOut:
        // 圆弧出入
        if (p < 0.5f) {
            return 0.5f * (1.f - ::sqrt(1.f - 4.f * (p * p)));
        }
        else {
            return 0.5f * (::sqrt(-((2.f * p) - 3.f) * ((2.f * p) - 1.f)) + 1.f);
        }
    case AnimationType::Type_ExponentialEaseIn:
        // 指数渐入     f(x) = 2^(10(x - 1))
        return (p == 0.f) ? (p) : (::pow(2.f, 10.f * (p - 1.f)));
    case AnimationType::Type_ExponentialEaseOut:
        // 指数渐出     f(x) =  -2^(-10x) + 1
        return (p == 1.f) ? (p) : (1.f - ::powf(2.f, -10.f * p));
    case AnimationType::Type_ExponentialEaseInOut:
        // 指数出入
        // [0,0.5)      f(x) = (1/2)2^(10(2x - 1)) 
        // [0.5,1.f]    f(x) = -(1/2)*2^(-10(2x - 1))) + 1 
        if (p == 0.0f || p == 1.0f) return p;
        if (p < 0.5f) {
            return 0.5f * ::powf(2.f, (20.f * p) - 10.f);
        }
        else {
            return -0.5f * ::powf(2.f, (-20.f * p) + 1.f) + 1.f;
        }
    case AnimationType::Type_ElasticEaseIn:
        // 弹性渐入
        return ::sin(13.f * EZ_PI_2 * p) * ::pow(2.f, 10.f * (p - 1.f));
    case AnimationType::Type_ElasticEaseOut:
        // 弹性渐出
        return ::sin(-13.f * EZ_PI_2 * (p + 1.f)) * ::powf(2.f, -10.f * p) + 1.f;
    case AnimationType::Type_ElasticEaseInOut:
        // 弹性出入
        if (p < 0.5f) {
            return 0.5f * ::sin(13.f * EZ_PI_2 * (2.f * p)) * ::pow(2.f, 10.f * ((2.f * p) - 1.f));
        }
        else {
            return 0.5f * (::sin(-13.f * EZ_PI_2 * ((2.f * p - 1.f) + 1.f)) * ::pow(2.f, -10.f * (2.f * p - 1.f)) + 2.f);
        }
    case AnimationType::Type_BackEaseIn:
        // 回退渐入
        return  p * p * p - p * ::sin(p * EZ_PI);
    case AnimationType::Type_BackEaseOut:
        // 回退渐出
    {
        register float f = (1.f - p);
        return 1.f - (f * f * f - f * ::sin(f * EZ_PI));
    }
    case AnimationType::Type_BackEaseInOut:
        // 回退出入
        if (p < 0.5f) {
            register float f = 2.f * p;
            return 0.5f * (f * f * f - f * ::sin(f * EZ_PI));
        }
        else {
            register float f = (1.f - (2 * p - 1.f));
            return 0.5f * (1.f - (f * f * f - f * ::sin(f * EZ_PI))) + 0.5f;
        }
    case AnimationType::Type_BounceEaseIn:
        // 反弹渐入
        return 1.f - ::BounceEaseOut(1.f - p);
    case AnimationType::Type_BounceEaseOut:
        // 反弹渐出
        return ::BounceEaseOut(p);
    case AnimationType::Type_BounceEaseInOut:
        // 反弹出入
        if (p < 0.5f) {
            return 0.5f * (1.f - ::BounceEaseOut(1.f - (p*2.f)));
        }
        else {
            return 0.5f * ::BounceEaseOut(p * 2.f - 1.f) + 0.5f;
        }
    }
}


#include <VersionHelpers.h>

// 初始化库
class InitializeLibrary {
    typedef enum PROCESS_DPI_AWARENESS {
        PROCESS_DPI_UNAWARE = 0,
        PROCESS_SYSTEM_DPI_AWARE = 1,
        PROCESS_PER_MONITOR_DPI_AWARE = 2
    } PROCESS_DPI_AWARENESS;
    // SetProcessDpiAwareness
    static HRESULT STDAPICALLTYPE SetProcessDpiAwarenessF(PROCESS_DPI_AWARENESS);
public:
    //
    InitializeLibrary() {
        // >= Win8.1 ?
        if (::IsWindows8OrGreater()) {
            m_hDllShcore = ::LoadLibraryW(L"Shcore.dll");
            assert(m_hDllShcore);
            if (m_hDllShcore) {
                auto setProcessDpiAwareness =
                    reinterpret_cast<decltype(&InitializeLibrary::SetProcessDpiAwarenessF)>(
                        ::GetProcAddress(m_hDllShcore, "SetProcessDpiAwareness")
                        );
                assert(setProcessDpiAwareness);
                if (setProcessDpiAwareness) {
                    setProcessDpiAwareness(InitializeLibrary::PROCESS_PER_MONITOR_DPI_AWARE);
                }
            }
        }
    };
    //
    ~InitializeLibrary() {
        if (m_hDllShcore) {
            ::FreeLibrary(m_hDllShcore);
            m_hDllShcore = nullptr;
        }
    }
private:
    // Shcore
    HMODULE     m_hDllShcore = nullptr;
} instance;


#pragma comment(lib, "libmruby")
