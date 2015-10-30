#include "stdafx.h"
#include "included.h"

// {386E2670-F9F8-43C9-B25D-FF7D50F63B67}
const GUID CLSID_DustPG_RadialBlurEffect = {
    0x386e2670, 0xf9f8, 0x43c9, { 0xb2, 0x5d, 0xff, 0x7d, 0x50, 0xf6, 0x3b, 0x67 }
};

// {63DCE15F-2EDF-4C3A-B81B-538142E4613F}
static const GUID GUID_DustPG_RadialBlurShader = { 
    0x63dce15f, 0x2edf, 0x4c3a, { 0xb8, 0x1b, 0x53, 0x81, 0x42, 0xe4, 0x61, 0x3f } 
};

// DIY Version
#define MY_D2D1_VALUE_TYPE_BINDING(CLASS, TYPE, NAME)\
    {\
        L#NAME, [](IUnknown* obj, const BYTE* data, UINT32 len) noexcept {\
            assert(obj && data && len == sizeof(TYPE));\
            auto impl = static_cast<ID2D1EffectImpl*>(obj);\
            auto ths = static_cast<CLASS*>(impl);\
            ths->Set##NAME(*reinterpret_cast<const TYPE*>(data));\
            return S_OK;\
        },  [](const IUnknown* obj, BYTE* OPTIONAL data, UINT32 len, UINT32* OPTIONAL outeln) noexcept {\
            assert(obj);\
            if (data) {\
                auto impl = static_cast<const ID2D1EffectImpl*>(obj);\
                auto ths = static_cast<const RadialBlurEffect*>(impl);\
                *reinterpret_cast<TYPE*>(data) = ths->Get##NAME();\
            }\
            if (outeln) {\
                *outeln = sizeof(TYPE);\
            }\
            return S_OK;\
        }\
    }

// 注册径向模糊特效
auto RadialBlurEffect::Register(ID2D1Factory1* factory) noexcept ->HRESULT {
    assert(factory && "bad argment");
    const WCHAR* pszXml = LR"xml(<?xml version = "1.0" ?>
<Effect>
    <Property name = "DisplayName" type = "string" value = "RadialBlurEffect" />
    <Property name = "Author" type = "string" value = "dustpg" />
    <Property name = "Category" type = "string" value = "Transform" />
    <Property name = "Description" type = "string" value = "径向模糊" />
    <Inputs>
        <Input name = "Source" />
    </Inputs>
    <Property name='Center' type='vector2'>
        <Property name='DisplayName' type='string' value='radial blur center'/>
    </Property>
    <Property name='Magnitude' type='float'>
        <Property name='DisplayName' type='string' value='radial blur cagnitude'/>
    </Property>
    <Property name='Samples' type='float'>
        <Property name='DisplayName' type='string' value='radial blur samples count'/>
    </Property>
</Effect>
)xml";
    // 创建
    auto create_this = [](IUnknown** effect) noexcept ->HRESULT {
        assert(effect && "bad argment");
        ID2D1EffectImpl* obj = new(std::nothrow) RadialBlurEffect();
        *effect = obj;
        return obj ? S_OK : E_OUTOFMEMORY;
    };
    // 属性绑定
    D2D1_PROPERTY_BINDING bindings[] = {
        MY_D2D1_VALUE_TYPE_BINDING(RadialBlurEffect, D2D1_POINT_2F, Center),
        MY_D2D1_VALUE_TYPE_BINDING(RadialBlurEffect, float, Magnitude),
        MY_D2D1_VALUE_TYPE_BINDING(RadialBlurEffect, float, Samples),
    };
    // 注册
    return factory->RegisterEffectFromString(
        CLSID_DustPG_RadialBlurEffect,
        pszXml,
        bindings, lengthof(bindings),
        create_this
        );
}

// 初始化对象 Create 创建后 会调用此方法
IFACEMETHODIMP RadialBlurEffect::Initialize(
    ID2D1EffectContext* pEffectContext,
    ID2D1TransformGraph* pTransformGraph) noexcept {
    // 参数检查
    assert(pEffectContext && pTransformGraph && "bad arguments");
    if (!pEffectContext || !pTransformGraph) return E_INVALIDARG;
    HRESULT hr = S_FALSE;
    // 载入shader文件
    if (SUCCEEDED(hr)) {
        // 检查是否已经可以
        BYTE buf[4] = { 0 };
        auto tmp = pEffectContext->LoadPixelShader(GUID_DustPG_RadialBlurShader, buf, 0);
        // 失败时载入
        if (FAILED(tmp)) {
            auto file = std::fopen("ShaderObject\\RadialBlurEffect.cso", "rb");
            if (file) {
                // 读取文件信息
                std::fseek(file, 0L, SEEK_END);
                size_t length = std::ftell(file);
                std::fseek(file, 0L, SEEK_SET);
                BYTE* pBuffer = new(std::nothrow) BYTE[length];
                // 申请成功
                if (pBuffer) {
                    fread(pBuffer, 1, length, file);
                    hr = pEffectContext->LoadPixelShader(GUID_DustPG_RadialBlurShader, pBuffer, length);
                    delete[] pBuffer;
                }
                else {
                    hr = E_OUTOFMEMORY;
                }
                std::fclose(file);
            }
        }
    }
    // 连接
    if (SUCCEEDED(hr)) {
        hr = pTransformGraph->SetSingleTransformNode(this);
    }
    return hr;
}


// 准备渲染
IFACEMETHODIMP RadialBlurEffect::PrepareForRender(D2D1_CHANGE_TYPE changeType) noexcept {
    if (changeType == D2D1_CHANGE_TYPE_NONE) return S_OK;
    // 设置像素着色器
    m_pDrawInfo->SetPixelShader(GUID_DustPG_RadialBlurShader);
    // 修改了属性
    if (changeType == D2D1_CHANGE_TYPE_PROPERTIES) {
        // 更新常量缓存
        return m_pDrawInfo->SetPixelShaderConstantBuffer(
            reinterpret_cast<BYTE*>(&m_cbuffer), sizeof(m_cbuffer)
            );
    }
    return S_OK;
}

// 实现 IUnknown::Release
IFACEMETHODIMP_(ULONG) RadialBlurEffect::Release() noexcept {
    if ((--m_cRef) == 0){
        delete this;
        return 0;
    }
    else {
        return m_cRef;
    }
}

// 实现 IUnknown::QueryInterface
IFACEMETHODIMP RadialBlurEffect::QueryInterface(REFIID riid, _Outptr_ void** ppOutput) noexcept {
    *ppOutput = nullptr;
    HRESULT hr = S_OK;
    // 获取 ID2D1EffectImpl
    if (riid == __uuidof(ID2D1EffectImpl)) {
        *ppOutput = static_cast<ID2D1EffectImpl*>(this);
    }
    // 获取 ID2D1DrawTransform
    else if (riid == __uuidof(ID2D1DrawTransform)) {
        *ppOutput = static_cast<ID2D1DrawTransform*>(this);
    }
    // 获取 ID2D1Transform
    else if (riid == __uuidof(ID2D1Transform)) {
        *ppOutput = static_cast<ID2D1Transform*>(this);
    }
    // 获取 ID2D1TransformNode
    else if (riid == __uuidof(ID2D1TransformNode)) {
        *ppOutput = static_cast<ID2D1TransformNode*>(this);
    }
    // 获取 IUnknown
    else if (riid == __uuidof(IUnknown)){
        *ppOutput = this;
    }
    // 没有接口
    else {
        hr = E_NOINTERFACE;
    }
    if (*ppOutput != nullptr) {
        AddRef();
    }
    return hr;
}


// 设置刻画信息
IFACEMETHODIMP RadialBlurEffect::SetDrawInfo(_In_ ID2D1DrawInfo *drawInfo) noexcept {
    ::SafeRelease(m_pDrawInfo);
    m_pDrawInfo = ::SafeAcquire(drawInfo);
    return S_OK;
}

// 映射无效矩形区
IFACEMETHODIMP RadialBlurEffect::MapInvalidRect(
    UINT32 inputIndex,
    D2D1_RECT_L invalidInputRect,
    _Out_ D2D1_RECT_L* pInvalidOutputRect
    ) const noexcept {
    *pInvalidOutputRect = m_inputRect;
    return S_OK;
}

// 映射输出矩形到输入矩形数组
IFACEMETHODIMP RadialBlurEffect::MapOutputRectToInputRects(
    _In_ const D2D1_RECT_L* pOutputRect,
    _Out_writes_(inputRectCount) D2D1_RECT_L* pInputRects,
    UINT32 inputRectCount
    ) const noexcept {
    // 虽说是数组 这里就一个
    if (inputRectCount != 1) return E_INVALIDARG;
    // 映射
    pInputRects[0] = m_inputRect;
    return S_OK;
}

// 映射输入矩形数组到输出输出矩形
IFACEMETHODIMP RadialBlurEffect::MapInputRectsToOutputRect(
    _In_reads_(inputRectCount) const D2D1_RECT_L* pInputRects,
    _In_reads_(inputRectCount) const D2D1_RECT_L* pInputOpaqueSubRects,
    UINT32 inputRectCount,
    _Out_ D2D1_RECT_L* pOutputRect,
    _Out_ D2D1_RECT_L* pOutputOpaqueSubRect
    ) noexcept {
    // 虽说是数组 这里就一个
    if (inputRectCount != 1) return E_INVALIDARG;
    *pOutputRect = pInputRects[0];
    m_inputRect = pInputRects[0];
    *pOutputOpaqueSubRect = *pOutputRect;
    return S_OK;
}