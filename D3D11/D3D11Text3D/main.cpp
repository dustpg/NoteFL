#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dwrite_1.h>
#include <d2d1_1.h>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <vector>


struct alignas(sizeof(float)*4) GlobalData {
    DirectX::XMMATRIX       view;
    DirectX::XMMATRIX       projection;
    IDXGISwapChain*         swap_chain;
    ID3D11Device*           device;
    ID3D11DeviceContext*    device_context;
    ID3D11RenderTargetView* d3d_rtv;
    ID3D11DepthStencilView* d3d_dsv;
    ID3D11Texture2D*        d3d_dsbuffer;
    ID3D11PixelShader*      d3d_white_ps;
    ID3D11PixelShader*      d3d_black_ps;
    ID3D11Buffer*           d3d_rect_vertex;
    ID3D11InputLayout*      d3d_rect_layout;
    ID3D11InputLayout*      d3d_v2d_layout;
    ID3D11VertexShader*     d3d_rect_vs;
    ID3D11PixelShader*      d3d_rect_ps;
    ID3D11Buffer*           d3d_rect_cbuffer;
    ID3D11Buffer*           d3d_text_vertex;
    ID3D11VertexShader*     d3d_vertex2d_vs;
    ID3D11SamplerState*     d3d_sampler;
    ID3D11Texture2D*        d3d_texture;
    ID3D11BlendState*       d3d_blendstate;
    ID3D11RasterizerState*  d3d_frame;

    ID3D11ShaderResourceView*   d3d_texture_view;

    IDWriteFactory1*        dw_factory;
    IDWriteTextFormat*      dw_format;
    IDWriteTextLayout*      dw_layout;

    ID2D1Factory1*          d2d_factory;
    ID2D1Device*            d2d_device;
    ID2D1DeviceContext*     d2d_context;
    ID2D1Bitmap1*           d2d_target;
    ID2D1SolidColorBrush*   d2d_brush;

    uint32_t                text_vertex_count;
} g_data = {  };

// 顶点-纹理 模型
struct VertexTex {
    DirectX::XMFLOAT3   pos;
    DirectX::XMFLOAT2   tex;
};

// 顶点 模型
struct Vertex2D {
    DirectX::XMFLOAT2   pos;
};

// 常量缓存
struct MatrixBufferType {
    DirectX::XMMATRIX   world;
    DirectX::XMMATRIX   view;
    DirectX::XMMATRIX   projection;
};

// FONT
static const wchar_t FONT_NAME[] = L"Arial";
static const wchar_t TEXT[] = LR"(话说
天下大势，分久必合，合久必分。
周末七国分争，并入于秦。及秦灭之后，
楚、汉分争，又并入于汉。汉朝自高祖斩
白蛇而起义，一统天下，后来光武中兴，
传至献帝，遂分为三国。)";

enum { FONT_SIZE = 24 };
namespace Demo {
    const GUID IID_IDWriteFactory1 = {
        0x30572f99, 0xdac6, 0x41db,{ 0xa1, 0x6e, 0x04, 0x86, 0x30, 0x7e, 0x60, 0x6a }
    };
}
// D3D
static constexpr float SCREEN_NEAR_Z = (0.01f);
static constexpr float SCREEN_FAR_Z = (1000.f);
enum { WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 720 };
enum { OFFSCREEN_WIDTH = 512, OFFSCREEN_HEIGHT = 512 };
enum { MSAA = 1 };
static const wchar_t WINDOW_TITLE[] = L"D3D11 Interop With D2D";
LRESULT CALLBACK ThisWndProc(HWND , UINT , WPARAM , LPARAM ) noexcept;
void DoRender(uint32_t sync, uint32_t counter) noexcept;
void CallOnce() noexcept;
bool InitD3D(HWND) noexcept;
void ClearD3D() noexcept;
HRESULT CreateTexRect() noexcept;
HRESULT CreateVertexFromLayout(IDWriteTextLayout& layout, ID3D11Buffer** ppb, uint32_t& l)noexcept;

template<class Interface>
inline void SafeRelease(Interface *&pInterfaceToRelease) {
    if (pInterfaceToRelease != nullptr) {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = nullptr;
    }
}

extern "C" int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE, char*, int nCmdShow) noexcept {
    // DPIAware
    ::SetProcessDPIAware();
    DirectX::XMVECTOR position = { 5.f, 0.f, 0.f, 1.f };
    DirectX::XMVECTOR look_at = { 0.f, 0.f, 0.f, 1.f };
    DirectX::XMVECTOR up = { 0.f, 5.f, 0.f, 1.f };
    g_data.view = DirectX::XMMatrixLookAtLH(position, look_at, up);
    g_data.projection = DirectX::XMMatrixPerspectiveFovLH(
        45.0f,
        static_cast<float>(WINDOW_WIDTH)/static_cast<float>(WINDOW_HEIGHT),
        SCREEN_NEAR_Z,
        SCREEN_FAR_Z
    );
    // 注册窗口
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ThisWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"DemoWindowClass";
    wcex.hIcon = nullptr;
    ::RegisterClassExW(&wcex);
    // 计算窗口大小
    RECT window_rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    DWORD window_style = WS_OVERLAPPEDWINDOW;
    ::AdjustWindowRect(&window_rect, window_style, FALSE);
    window_rect.right -= window_rect.left;
    window_rect.bottom -= window_rect.top;
    window_rect.left = (::GetSystemMetrics(SM_CXFULLSCREEN) - window_rect.right) / 2;
    window_rect.top = (::GetSystemMetrics(SM_CYFULLSCREEN) - window_rect.bottom) / 2;
    // 创建窗口
    const auto hwnd = ::CreateWindowExW(
        0,
        wcex.lpszClassName, WINDOW_TITLE, window_style,
        window_rect.left, window_rect.top, window_rect.right, window_rect.bottom,
        0, 0, hInstance, nullptr
    );
    if (!hwnd) return 1;
    ::ShowWindow(hwnd, nCmdShow);
    ::UpdateWindow(hwnd);
    if (::InitD3D(hwnd)) {
        CallOnce();
        uint32_t counter = 0;
        MSG msg = { 0 };
        while (msg.message != WM_QUIT) {
            // 获取消息
            if (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessageW(&msg);
            }
            else DoRender(1, ++counter);
        }
    }
    ::ClearD3D();
    return 0;
}




LRESULT CALLBACK ThisWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    switch (msg)
    {
    case WM_CLOSE:
        ::DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}


void CallOnce() noexcept {
    // D2D
    {
        const auto ctx = g_data.d2d_context;
        ctx->BeginDraw();
        ctx->Clear(D2D1::ColorF(0.f, 0.f, 0.f, 0.f));
        ctx->DrawTextLayout({}, g_data.dw_layout, g_data.d2d_brush);
        const auto hr = ctx->EndDraw();
    }
}

void DoRender(uint32_t sync, uint32_t counter) noexcept {
    static float s_angle = 0.f;
    constexpr float fpi = 3.1415927f;
    s_angle += 1.f * fpi / 180.f;
    //s_angle = 2.33f + 0.233f + 0.233f;
    
    // D3D
    float clearColor[4] = { 0.4f, 0.8f, 1.0f, 1.0f };
    const auto ctx = g_data.device_context;
    ctx->ClearRenderTargetView(g_data.d3d_rtv, clearColor);
    ctx->ClearDepthStencilView(g_data.d3d_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);


    bool draw_texture = counter % 60 == counter % 30;
    //draw_texture = counter & 1;
    {

        MatrixBufferType mat;
        constexpr float basic_scale = 2.33333333f;
        if (draw_texture) {
            const auto mx = DirectX::XMMatrixRotationX(-0.2333f);
            const auto my = DirectX::XMMatrixRotationY(std::fmod(s_angle, fpi));
            const auto ss = DirectX::XMMatrixScaling(basic_scale, basic_scale, basic_scale);
            mat.world = DirectX::XMMatrixTranspose(mx * my * ss);
        }
        else {
            constexpr float mw = OFFSCREEN_WIDTH;
            constexpr float mh = OFFSCREEN_HEIGHT;

            const auto mo = DirectX::XMMatrixTranslation(-mw / 2, -mh / 2, 0);
            const auto mx = DirectX::XMMatrixRotationX(0.2333f);
            const auto my = DirectX::XMMatrixRotationY(std::fmod(s_angle, fpi));
            const auto mz = DirectX::XMMatrixRotationZ(fpi);
            const auto scale = basic_scale / mw * 2.f;
            const auto ss = DirectX::XMMatrixScaling(scale, scale, scale);
            mat.world = DirectX::XMMatrixTranspose(mo * mx * mz * my * ss);
        }
        mat.view = DirectX::XMMatrixTranspose(g_data.view);
        mat.projection = DirectX::XMMatrixTranspose(g_data.projection);
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        const auto hr = ctx->Map(g_data.d3d_rect_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            std::memcpy(mapped.pData, &mat, sizeof(mat));
            ctx->Unmap(g_data.d3d_rect_cbuffer, 0);
        }

    }
    {
        float factor[4] = {};
        if (draw_texture) {
            ctx->OMSetBlendState(g_data.d3d_blendstate, factor, 0xffffffff);
            const UINT stride = sizeof(VertexTex);
            const UINT offset = 0;
            ctx->IASetVertexBuffers(0, 1, &g_data.d3d_rect_vertex, &stride, &offset);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            ctx->IASetInputLayout(g_data.d3d_rect_layout);
            ctx->VSSetConstantBuffers(0, 1, &g_data.d3d_rect_cbuffer);
            ctx->PSSetShaderResources(0, 1, &g_data.d3d_texture_view);
            ctx->VSSetShader(g_data.d3d_rect_vs, nullptr, 0);
            ctx->PSSetSamplers(0, 1, &g_data.d3d_sampler);

            ctx->PSSetShader(g_data.d3d_white_ps, nullptr, 0);
            ctx->RSSetState(g_data.d3d_frame);
            ctx->Draw(6, 0);

            ctx->PSSetShader(g_data.d3d_rect_ps, nullptr, 0);
            ctx->RSSetState(nullptr);
            ctx->Draw(6, 0);
        }
        else {
            ctx->OMSetBlendState(nullptr, factor, 0xffffffff);
            ctx->IASetInputLayout(g_data.d3d_v2d_layout);
            const UINT stride = sizeof(Vertex2D);
            const UINT offset = 0;
            ctx->IASetVertexBuffers(0, 1, &g_data.d3d_text_vertex, &stride, &offset);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            ctx->VSSetConstantBuffers(0, 1, &g_data.d3d_rect_cbuffer);
            ctx->VSSetShader(g_data.d3d_vertex2d_vs, nullptr, 0);

            if (true) {
                ctx->RSSetState(nullptr);
                ctx->PSSetShader(g_data.d3d_black_ps, nullptr, 0);
                ctx->Draw(g_data.text_vertex_count, 0);
            }
            else {
                ctx->RSSetState(g_data.d3d_frame);
                ctx->PSSetShader(g_data.d3d_white_ps, nullptr, 0);
                ctx->Draw(g_data.text_vertex_count, 0);
            }

        }



    }
    const auto hr = g_data.swap_chain->Present(sync, 0);
}

bool InitD3D(HWND hwnd) noexcept {
    HRESULT hr = S_OK;
    ID3D11Texture2D* buffer = nullptr;
    IDXGIDevice1* dxgi_device = nullptr;
    IDXGISurface* dxgi_surface = nullptr;
    // 创建D3D设备与交换链
    if (SUCCEEDED(hr)) {
        // D3D11 创建flag 
        // 一定要有D3D11_CREATE_DEVICE_BGRA_SUPPORT
        // 否则创建D2D设备上下文会失败
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if !defined(NDEBUG)
        // Debug状态 有D3D DebugLayer就可以取消注释
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        const D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };
        DXGI_SWAP_CHAIN_DESC sd = { 0 };
        sd.BufferCount = 2;
        sd.BufferDesc.Width = WINDOW_WIDTH;
        sd.BufferDesc.Height = WINDOW_HEIGHT;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = MSAA;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        hr = ::D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            featureLevels,
            sizeof(featureLevels) / sizeof(featureLevels[0]),
            D3D11_SDK_VERSION,
            &sd,
            &g_data.swap_chain,
            &g_data.device,
            nullptr,
            &g_data.device_context
        );
    }
    // 获取后备缓存作为Texture2D
    if (SUCCEEDED(hr)) {
        hr = g_data.swap_chain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&buffer);
    }
    // 创建渲染目标视图
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreateRenderTargetView(buffer, nullptr, &g_data.d3d_rtv);
    }
    // 创建深度模板缓存
    if (SUCCEEDED(hr)) {
        D3D11_TEXTURE2D_DESC dsbuffer_desc = {};
        dsbuffer_desc.Width = WINDOW_WIDTH;
        dsbuffer_desc.Height = WINDOW_HEIGHT;
        dsbuffer_desc.MipLevels = 1;
        dsbuffer_desc.ArraySize = 1;
        // Depth: 24bit Stencil: 8bit
        dsbuffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsbuffer_desc.SampleDesc.Count = MSAA;
        dsbuffer_desc.SampleDesc.Quality = 0;
        dsbuffer_desc.Usage = D3D11_USAGE_DEFAULT;
        dsbuffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        dsbuffer_desc.CPUAccessFlags = 0;
        dsbuffer_desc.MiscFlags = 0;
        hr = g_data.device->CreateTexture2D(&dsbuffer_desc, nullptr, &g_data.d3d_dsbuffer);
    }
    // 创建深度模板视图
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreateDepthStencilView(g_data.d3d_dsbuffer, nullptr, &g_data.d3d_dsv);
    }
    // 创建常量缓存
    if (SUCCEEDED(hr)) {
        D3D11_BUFFER_DESC matrix_desc = { 0 };
        matrix_desc.Usage = D3D11_USAGE_DYNAMIC;
        matrix_desc.ByteWidth = sizeof(MatrixBufferType);
        matrix_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        matrix_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        matrix_desc.MiscFlags = 0;
        matrix_desc.StructureByteStride = 0;
        hr = g_data.device->CreateBuffer(&matrix_desc, nullptr, &g_data.d3d_rect_cbuffer);
    }
    // 创建彩色立方体
    if (SUCCEEDED(hr)) {
        hr = ::CreateTexRect();
    }
    // 创建D2D工厂
    if (SUCCEEDED(hr)) {
        hr = ::D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            IID_ID2D1Factory1,
            (void**)&g_data.d2d_factory
        );
    }
    // 创建 IDXGIDevice
    if (SUCCEEDED(hr)) {
        hr = g_data.device->QueryInterface(
            IID_IDXGIDevice1,
            reinterpret_cast<void**>(&dxgi_device)
        );
    }
    // 创建 D2D设备
    if (SUCCEEDED(hr)) {
        hr = g_data.d2d_factory->CreateDevice(dxgi_device, &g_data.d2d_device);
    }
    // 创建 D2D设备上下文
    if (SUCCEEDED(hr)) {
        hr = g_data.d2d_device->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &g_data.d2d_context
        );
    }
    // 创建采样器
    if (SUCCEEDED(hr)) {
        D3D11_SAMPLER_DESC sampler_desc = { };
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.MipLODBias = 0.0f;
        sampler_desc.MaxAnisotropy = 1;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        sampler_desc.MinLOD = 0;
        sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
        hr = g_data.device->CreateSamplerState(&sampler_desc, &g_data.d3d_sampler);
    }
    // 创建纹理
    if (SUCCEEDED(hr)) {
        D3D11_TEXTURE2D_DESC tex_desc = {};
        tex_desc.Height = OFFSCREEN_WIDTH;
        tex_desc.Width = OFFSCREEN_HEIGHT;
        tex_desc.MipLevels = 1;
        tex_desc.ArraySize = 1;
        tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        tex_desc.SampleDesc.Count = 1;
        tex_desc.SampleDesc.Quality = 0;
        tex_desc.Usage = D3D11_USAGE_DEFAULT;
        tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        tex_desc.CPUAccessFlags = 0;
        tex_desc.MiscFlags = 0;

        hr = g_data.device->CreateTexture2D(&tex_desc, nullptr, &g_data.d3d_texture);
    }
    // 利用纹理创建纹理视图
    if (SUCCEEDED(hr)) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = { };
        srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MostDetailedMip = 0;
        srv_desc.Texture2D.MipLevels = 1;
        hr = g_data.device->CreateShaderResourceView(
            g_data.d3d_texture, &srv_desc, &g_data.d3d_texture_view
        );
    }
    // 利用纹理获取DXGI表面
    if (SUCCEEDED(hr)) {
        hr = g_data.d3d_texture->QueryInterface(IID_IDXGISurface, (void**)&dxgi_surface);
    }
    // 利用DXGI表面创建D2D渲染承载位图
    if (SUCCEEDED(hr)) {
        D2D1_BITMAP_PROPERTIES1 bitmap_properties = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
        );
        hr = g_data.d2d_context->CreateBitmapFromDxgiSurface(
            dxgi_surface,
            &bitmap_properties,
            &g_data.d2d_target
        );
    }
    // 创建混合状态
    if (SUCCEEDED(hr)) {
        D3D11_BLEND_DESC desc = {};
        desc.RenderTarget[0].BlendEnable = TRUE;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        hr = g_data.device->CreateBlendState(&desc, &g_data.d3d_blendstate);
    }
    // 创建纯色笔刷
    if (SUCCEEDED(hr)) {
        hr = g_data.d2d_context->CreateSolidColorBrush(
            D2D1::ColorF(0.f, 0.f, 0.f, 1.f),
            &g_data.d2d_brush
        );
    }
    // 光栅化状态
    if (SUCCEEDED(hr)) {
        D3D11_RASTERIZER_DESC desc = {};
        desc.FillMode = D3D11_FILL_WIREFRAME;
        desc.CullMode = D3D11_CULL_BACK;
        desc.DepthClipEnable = true;
        hr = g_data.device->CreateRasterizerState(&desc, &g_data.d3d_frame);
    }
    // 创建DWrite工厂
    if (SUCCEEDED(hr)) {
        hr = ::DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            Demo::IID_IDWriteFactory1,
            reinterpret_cast<IUnknown**>(&g_data.dw_factory)
        );
    }
    // 创建待用字体(文本格式)
    if (SUCCEEDED(hr)) {
        hr = g_data.dw_factory->CreateTextFormat(
            FONT_NAME,
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            FONT_SIZE,
            L"",
            &g_data.dw_format
        );
    }
    // 创建待渲染的文本
    if (SUCCEEDED(hr)) {
        hr = g_data.dw_factory->CreateTextLayout(
            TEXT,
            sizeof(TEXT)/sizeof(TEXT[0]),
            g_data.dw_format,
            OFFSCREEN_WIDTH,
            OFFSCREEN_HEIGHT,
            &g_data.dw_layout
        );
    }
    // 创建文本顶点
    if (SUCCEEDED(hr)) {
        g_data.dw_layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        g_data.dw_layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        hr = ::CreateVertexFromLayout(
            *g_data.dw_layout, 
            &g_data.d3d_text_vertex, 
            g_data.text_vertex_count
        );
    }
    // 设置为输出目标
    if (SUCCEEDED(hr)) {
        g_data.device_context->OMSetRenderTargets(1, &g_data.d3d_rtv, g_data.d3d_dsv);
        g_data.d2d_context->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
        g_data.d2d_context->SetTarget(g_data.d2d_target);
    }
    // 设置视口数据
    if (SUCCEEDED(hr)) {
        D3D11_VIEWPORT vp;
        vp.Width = static_cast<FLOAT>(WINDOW_WIDTH);
        vp.Height = static_cast<FLOAT>(WINDOW_HEIGHT);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        g_data.device_context->RSSetViewports(1, &vp);
    }
    ::SafeRelease(dxgi_surface);
    ::SafeRelease(dxgi_device);
    ::SafeRelease(buffer);
    return SUCCEEDED(hr);
}

void ClearD3D() noexcept {
    ::SafeRelease(g_data.dw_factory);
    ::SafeRelease(g_data.dw_format);
    ::SafeRelease(g_data.dw_layout);

    ::SafeRelease(g_data.d2d_brush);
    ::SafeRelease(g_data.d2d_target); 
    ::SafeRelease(g_data.d2d_context);
    ::SafeRelease(g_data.d2d_device);
    ::SafeRelease(g_data.d2d_factory);
    
    
    ::SafeRelease(g_data.d3d_v2d_layout);
    ::SafeRelease(g_data.d3d_vertex2d_vs);
    ::SafeRelease(g_data.d3d_frame);
    ::SafeRelease(g_data.d3d_blendstate);
    ::SafeRelease(g_data.d3d_texture_view);
    ::SafeRelease(g_data.d3d_texture);
    ::SafeRelease(g_data.d3d_text_vertex);
    ::SafeRelease(g_data.d3d_sampler);
    ::SafeRelease(g_data.d3d_rect_cbuffer);
    ::SafeRelease(g_data.d3d_white_ps);
    ::SafeRelease(g_data.d3d_black_ps);
    
    ::SafeRelease(g_data.d3d_rect_ps);
    ::SafeRelease(g_data.d3d_rect_vs);
    ::SafeRelease(g_data.d3d_rect_layout);
    ::SafeRelease(g_data.d3d_rect_vertex);
    ::SafeRelease(g_data.d3d_dsv);
    ::SafeRelease(g_data.d3d_dsv);
    ::SafeRelease(g_data.d3d_rtv);
    ::SafeRelease(g_data.d3d_dsbuffer);
    ::SafeRelease(g_data.device_context);
    ::SafeRelease(g_data.device);
    ::SafeRelease(g_data.swap_chain);
}



const char s_demo_shader[] = u8R"shader(
// C Buffer 0 : 储存转换矩阵
cbuffer MatrixBuffer : register (b0) {
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// VS 输入
struct VertexInputType {
    float4 position     : POSITION;
    float2 tex          : TEXCOORD0;
};

// VS 输入
struct Vertex2DInputType {
    float4 position     : POSITION;
};

// VS 输出
struct PixelInputType {
    float4 position     : SV_POSITION;
    float2 tex          : TEXCOORD0;
};
Texture2D shaderTexture;
SamplerState SampleType;

// 处理
PixelInputType TexVertexShader(VertexInputType input) {
    PixelInputType output;
    // 坐标转换
    output.position = mul(float4(input.position.xyz, 1), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    // 直接输出
    output.tex = input.tex;

    return output;
}

// 像素着色器处理
float4 TexPixelShader(PixelInputType input) : SV_TARGET {
    return shaderTexture.Sample(SampleType, input.tex);
}


// 处理
PixelInputType Vertex2DShader(Vertex2DInputType input) {
    PixelInputType output;
    // 坐标转换
    output.position = mul(float4(input.position.xy, 1, 1), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = float2(0, 0);
    return output;
}

// 像素着色器处理
float4 WhiteColor() : SV_TARGET {
    return float4(1,1,1,1);
}
// 像素着色器处理
float4 BlackColor() : SV_TARGET {
    return float4(0,0,0,1);
}
)shader";



// 创建矩形
HRESULT CreateTexRect() noexcept {
    // 矩形的4(6)个顶点 与相应纹理坐标
    const VertexTex vertices[] = {
        { DirectX::XMFLOAT3(-1.f, -1.f, 0.f), DirectX::XMFLOAT2(1.f, 1.f) },
        { DirectX::XMFLOAT3(+1.f, -1.f, 0.f), DirectX::XMFLOAT2(0.f, 1.f) },
        { DirectX::XMFLOAT3(-1.f, +1.f, 0.f), DirectX::XMFLOAT2(1.f, 0.f) },
        
        { DirectX::XMFLOAT3(-1.f, +1.f, 0.f), DirectX::XMFLOAT2(1.f, 0.f) },
        { DirectX::XMFLOAT3(+1.f, -1.f, 0.f), DirectX::XMFLOAT2(0.f, 1.f) },
        { DirectX::XMFLOAT3(+1.f, +1.f, 0.f), DirectX::XMFLOAT2(0.f, 0.f) },
    };
    // 输入布局
    const D3D11_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexTex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(VertexTex, tex), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    // 输入布局
    const D3D11_INPUT_ELEMENT_DESC inputLayout2D[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex2D, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    HRESULT hr = S_OK;
    ID3DBlob* vs = nullptr, *ps = nullptr, *white=nullptr, *vs2d = nullptr, *black = nullptr;
    // 创建顶点缓存
    if (SUCCEEDED(hr)) {
        D3D11_BUFFER_DESC buffer_desc = { 0 };
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.ByteWidth = sizeof(vertices);
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA sub_data = { vertices };
        hr = g_data.device->CreateBuffer(&buffer_desc, &sub_data, &g_data.d3d_rect_vertex);
    }
    UINT flag = 0;
#if !defined(NDEBUG)
    flag |= D3DCOMPILE_DEBUG;
#endif
    // 编译VS
    if (SUCCEEDED(hr)) {
        hr = ::D3DCompile(
            s_demo_shader, sizeof(s_demo_shader), 
            nullptr, nullptr, nullptr,
            "TexVertexShader", "vs_4_0", 
            flag, 0, &vs, nullptr
        );
    }
    // 编译VS2D
    if (SUCCEEDED(hr)) {
        hr = ::D3DCompile(
            s_demo_shader, sizeof(s_demo_shader),
            nullptr, nullptr, nullptr,
            "Vertex2DShader", "vs_4_0",
            flag, 0, &vs2d, nullptr
        );
    }
    // 编译PS
    if (SUCCEEDED(hr)) {
        hr = ::D3DCompile(
            s_demo_shader, sizeof(s_demo_shader),
            nullptr, nullptr, nullptr,
            "TexPixelShader", "ps_4_0",
            flag, 0, &ps, nullptr
        );
    }
    // 编译白色PS
    if (SUCCEEDED(hr)) {
        hr = ::D3DCompile(
            s_demo_shader, sizeof(s_demo_shader),
            nullptr, nullptr, nullptr,
            "WhiteColor", "ps_4_0",
            flag, 0, &white, nullptr
        );
    }
    // 编译黑色PS
    if (SUCCEEDED(hr)) {
        hr = ::D3DCompile(
            s_demo_shader, sizeof(s_demo_shader),
            nullptr, nullptr, nullptr,
            "BlackColor", "ps_4_0",
            flag, 0, &black, nullptr
        );
    }
    // 创建VS
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreateVertexShader(
            vs->GetBufferPointer(), 
            vs->GetBufferSize(), 
            nullptr, &g_data.d3d_rect_vs
        );
    }
    // 创建VS
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreateVertexShader(
            vs2d->GetBufferPointer(),
            vs2d->GetBufferSize(),
            nullptr, &g_data.d3d_vertex2d_vs
        );
    }
    // 创建PS
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreatePixelShader(
            ps->GetBufferPointer(),
            ps->GetBufferSize(),
            nullptr, &g_data.d3d_rect_ps
        );
    }
    // 创建白色PS
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreatePixelShader(
            white->GetBufferPointer(),
            white->GetBufferSize(),
            nullptr, &g_data.d3d_white_ps
        );
    }
    // 创建黑色PS
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreatePixelShader(
            black->GetBufferPointer(),
            black->GetBufferSize(),
            nullptr, &g_data.d3d_black_ps
        );
    }
    // 创建对应输入布局
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreateInputLayout(
            inputLayout, sizeof(inputLayout)/sizeof(inputLayout[0]), 
            vs->GetBufferPointer(),
            vs->GetBufferSize(), 
            &g_data.d3d_rect_layout
        );
    }
    // 创建对应输入布局
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreateInputLayout(
            inputLayout2D, sizeof(inputLayout2D) / sizeof(inputLayout2D[0]),
            vs2d->GetBufferPointer(),
            vs2d->GetBufferSize(),
            &g_data.d3d_v2d_layout
        );
    }
    ::SafeRelease(vs);
    ::SafeRelease(ps);
    ::SafeRelease(white);
    ::SafeRelease(black);
    ::SafeRelease(vs2d);
    return hr;
}


namespace detail {
    struct get_outline final : IDWriteTextRenderer, ID2D1TessellationSink {
        get_outline(ID2D1Factory& factory) noexcept: m_refFactory(factory){ factory.AddRef(); }
        ~get_outline() noexcept { m_refFactory.Release(); }
        ULONG STDMETHODCALLTYPE AddRef() noexcept override { return 0; }
        ULONG STDMETHODCALLTYPE Release() noexcept override { return 0; }
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void**) noexcept override { return E_UNEXPECTED; }
        STDMETHOD(Close)()noexcept override { return S_OK; }

        STDMETHOD_(void, AddTriangles)(const D2D1_TRIANGLE *triangles, UINT32 trianglesCount) noexcept override {
            try {
                const auto points = &triangles->point1;
                const auto pointc = trianglesCount * 3;
                for (UINT32 i = 0; i != pointc; ++i) {
                    Vertex2D v2d = {};
                    v2d.pos.x = m_fOffsetX + points[i].x;
                    v2d.pos.y = m_fOffsetY + points[i].y;
                    m_vList.push_back(v2d);
                }
            }
            catch (...) { }
        }

        //  DrawGlyphRun
        HRESULT STDMETHODCALLTYPE DrawGlyphRun(
            void* /*clientDrawingContext*/,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE /*measuringMode*/,
            DWRITE_GLYPH_RUN const* glyphRun,
            DWRITE_GLYPH_RUN_DESCRIPTION const* /*glyphRunDescription*/,
            IUnknown* /*clientDrawingEffect*/
            ) noexcept override
        {
            HRESULT hr = S_OK;
            ID2D1PathGeometry *pPathGeometry = nullptr;
            hr = m_refFactory.CreatePathGeometry(&pPathGeometry);
            if (SUCCEEDED(hr)) {
                ID2D1GeometrySink *pSink = nullptr;
                hr = pPathGeometry->Open(&pSink);
                if (SUCCEEDED(hr)) {
                    hr = glyphRun->fontFace->GetGlyphRunOutline(
                        glyphRun->fontEmSize,
                        glyphRun->glyphIndices,
                        glyphRun->glyphAdvances,
                        glyphRun->glyphOffsets,
                        glyphRun->glyphCount,
                        glyphRun->isSideways,
                        glyphRun->bidiLevel & 1,
                        pSink
                    );
                    if (SUCCEEDED(hr)) {
                        hr = pSink->Close();
                    }
                    if (SUCCEEDED(hr)) {
                        m_fOffsetX = baselineOriginX;
                        m_fOffsetY = baselineOriginY;
                        ID2D1TessellationSink* const this_sink = this;
                        constexpr float ft = D2D1_DEFAULT_FLATTENING_TOLERANCE;
                        hr = pPathGeometry->Tessellate(nullptr, ft, this_sink);
                    }
                    pSink->Release();
                }
                pPathGeometry->Release();
            }
            return hr;
        }

        STDMETHOD(DrawUnderline)(
            void* /*clientDrawingContext*/,
            FLOAT /*baselineOriginX*/,
            FLOAT /*baselineOriginY*/,
            DWRITE_UNDERLINE const* /*underline*/,
            IUnknown* /*clientDrawingEffect*/
            ) noexcept override {
            return E_NOTIMPL;
        }

        STDMETHOD(DrawStrikethrough)(
            void* /*clientDrawingContext*/,
            FLOAT /*baselineOriginX*/,
            FLOAT /*baselineOriginY*/,
            DWRITE_STRIKETHROUGH const* /*strikethrough*/,
            IUnknown* /*clientDrawingEffect*/
            ) noexcept override {
            return E_NOTIMPL;
        }

        STDMETHOD(DrawInlineObject)(
            void* /*clientDrawingContext*/,
            FLOAT /*originX*/,
            FLOAT /*originY*/,
            IDWriteInlineObject* /*inlineObject*/,
            BOOL /*isSideways*/,
            BOOL /*isRightToLeft*/,
            IUnknown* /*clientDrawingEffect*/
            ) noexcept override {
            return E_NOTIMPL;
        }

        STDMETHOD(IsPixelSnappingDisabled)(void*, BOOL* isDisabled) noexcept override {
            *isDisabled = TRUE;
            return S_OK;
        }

        STDMETHOD(GetCurrentTransform)(
            void* /*clientDrawingContext*/,
            DWRITE_MATRIX* transform
            )  noexcept override {
            DWRITE_MATRIX matrix = { 1, 0, 0, 1, 0, 0 };
            *transform = matrix;
            return S_OK;
        }
        STDMETHOD(GetPixelsPerDip)(void*, FLOAT* pixelsPerDip)  noexcept override { *pixelsPerDip = 1.0f; return S_OK; }
        auto& RefList() const noexcept { return m_vList; }
    private:
        ID2D1Factory &  m_refFactory;
        float           m_fOffsetX = 0.f;
        float           m_fOffsetY = 0.f;
        std::vector<Vertex2D>      m_vList;
    };

}

HRESULT CreateVertexFromLayout(IDWriteTextLayout& layout, ID3D11Buffer** ppb, uint32_t& count) noexcept {
    detail::get_outline renderer{ *g_data.d2d_factory };
    layout.Draw(nullptr, &renderer, 0.f, 0.f);
    const auto ptr = &renderer.RefList().front();
    const auto len = renderer.RefList().size();
    if ((count = len)) {
        D3D11_BUFFER_DESC buffer_desc = { 0 };
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.ByteWidth = sizeof(ptr[0]) * len;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA sub_data = { ptr };
        return g_data.device->CreateBuffer(&buffer_desc, &sub_data, ppb);
    }
    return E_FAIL;
}