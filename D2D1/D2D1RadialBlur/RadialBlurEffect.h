#pragma once

// 径向模糊特效 GUID
extern const GUID CLSID_DustPG_RadialBlurEffect;

// 径向模糊特效
class RadialBlurEffect final : public ID2D1EffectImpl, public ID2D1DrawTransform {
public:
    // 属性列表
    enum : UINT32 {
        // D2D1_POINT_2F
        Properties_CenterPoint = 0,
        // float
        Properties_Magnitude,
        // float
        Properties_Samples
    };
public:
    // ID2D1EffectImpl
    IFACEMETHODIMP Initialize(ID2D1EffectContext* pContextInternal, ID2D1TransformGraph* pTransformGraph) noexcept override;
    IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE changeType) noexcept override;
    IFACEMETHODIMP SetGraph(ID2D1TransformGraph* pGraph) noexcept override { assert("unsupported!"); return E_NOTIMPL; }
    // IUnknown
    IFACEMETHODIMP_(ULONG) AddRef() noexcept override { return ++m_cRef; }
    IFACEMETHODIMP_(ULONG) Release() noexcept override;
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppOutput) noexcept override;
    // ID2D1Transform
    IFACEMETHODIMP MapInputRectsToOutputRect(const D2D1_RECT_L* pInputRects,
        const D2D1_RECT_L* pInputOpaqueSubRects,
        UINT32 inputRectCount,
        D2D1_RECT_L* pOutputRect,
        D2D1_RECT_L* pOutputOpaqueSubRect) noexcept override; 
    IFACEMETHODIMP MapOutputRectToInputRects(const D2D1_RECT_L* pOutputRect,
        D2D1_RECT_L* pInputRects,
        UINT32 inputRectCount) const noexcept override;
    IFACEMETHODIMP MapInvalidRect(UINT32 inputIndex,
        D2D1_RECT_L invalidInputRect,
        D2D1_RECT_L* pInvalidOutputRect) const noexcept override;
    // ID2D1TransformNode
    IFACEMETHODIMP_(UINT32) GetInputCount() const noexcept override { return 1; }
    // ID2D1DrawTransform
    IFACEMETHODIMP SetDrawInfo(ID2D1DrawInfo *pDrawInfo) noexcept override;
public:
    // 构造函数
    RadialBlurEffect() noexcept {}
    // 析构函数
    ~RadialBlurEffect() noexcept { ::SafeRelease(m_pDrawInfo); }
    // 注册特效
    static auto Register(ID2D1Factory1* factory) noexcept ->HRESULT;
public:
    // 设置中心点
    auto SetCenter(const D2D1_POINT_2F& pt) noexcept { m_cbuffer.center = pt; }
    // 获取中心点
    auto GetCenter() const noexcept { return m_cbuffer.center; }
    // 设置程度
    auto SetMagnitude(float m) noexcept { m_cbuffer.magnitude = m; }
    // 获取程度
    auto GetMagnitude() const noexcept { return m_cbuffer.magnitude; }
    // 设置采样数量
    auto SetSamples(float s) noexcept { m_cbuffer.samples = s; }
    // 获取采样数量
    auto GetSamples() const noexcept { return m_cbuffer.samples; }
private:
    // 刻画信息
    ID2D1DrawInfo*              m_pDrawInfo = nullptr;
    // 引用计数器
    ULONG                       m_cRef = 1;
    // 保留
    ULONG                       m_nUnused = 0;
    // 输入矩形
    D2D1_RECT_L                 m_inputRect = D2D1::RectL();
    // 常量缓存
    struct {
        // 中心点
        D2D1_POINT_2F           center = D2D1_POINT_2F();
        // 程度
        float                   magnitude = 0.1f;
        // 采样点数量
        float                   samples = 16.f;
        // 常量缓存
    }                           m_cbuffer;
};