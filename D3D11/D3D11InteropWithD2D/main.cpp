#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d2d1_1.h>
#include <cstdint>
#include <cstring>
#include <cmath>


struct alignas(sizeof(float)*4) GlobalData {
    DirectX::XMMATRIX       view;
    DirectX::XMMATRIX       projection;
    IDXGISwapChain*         swap_chain;
    ID3D11Device*           device;
    ID3D11DeviceContext*    device_context;
    ID3D11RenderTargetView* d3d_rtv;
    ID3D11DepthStencilView* d3d_dsv;
    ID3D11Texture2D*        d3d_dsbuffer;
    ID3D11Buffer*           d3d_cube_vertex;
    ID3D11Buffer*           d3d_cube_index;
    ID3D11InputLayout*      d3d_cube_layout;
    ID3D11VertexShader*     d3d_cube_vs;
    ID3D11PixelShader*      d3d_cube_ps;
    ID3D11Buffer*           d3d_cube_cbuffer;
    ID3D11SamplerState*     d3d_sampler;
    ID3D11Texture2D*        d3d_texture;
    ID3D11ShaderResourceView*   d3d_texture_view;

    ID2D1Factory1*          d2d_factory;
    ID2D1Device*            d2d_device;
    ID2D1DeviceContext*     d2d_context;
    ID2D1Bitmap1*           d2d_target;
    ID2D1SolidColorBrush*   d2d_brush;
} g_data = {  };

// 顶点-纹理 模型
struct VertexTex {
    DirectX::XMFLOAT3   pos;
    DirectX::XMFLOAT2   tex;
};
// 常量缓存
struct MatrixBufferType {
    DirectX::XMMATRIX   world;
    DirectX::XMMATRIX   view;
    DirectX::XMMATRIX   projection;
};


static constexpr float SCREEN_NEAR_Z = (0.01f);
static constexpr float SCREEN_FAR_Z = (1000.f);
enum { WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 720 };
enum { OFFSCREEN_WIDTH = 512, OFFSCREEN_HEIGHT = 512 };
static const wchar_t WINDOW_TITLE[] = L"D3D11 Interop With D2D";
LRESULT CALLBACK ThisWndProc(HWND , UINT , WPARAM , LPARAM ) noexcept;
void DoRender(uint32_t sync) noexcept;
bool InitD3D(HWND) noexcept;
void ClearD3D() noexcept;
HRESULT CreateColoredCube() noexcept;

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
    AdjustWindowRect(&window_rect, window_style, FALSE);
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
        MSG msg = { 0 };
        while (msg.message != WM_QUIT) {
            // 获取消息
            if (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessageW(&msg);
            }
            else DoRender(1);
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


void DoRender(uint32_t sync) noexcept {
    static float s_angle = 0.f;
    s_angle += 1.f * 3.1415927f / 180.f;
    // D2D
    {
        const auto ctx = g_data.d2d_context;
        ctx->BeginDraw();
        ctx->Clear(D2D1::ColorF(1.f, 1.f, 1.f, 1.f));
        const auto base = s_angle * 60.f;
        const auto width = std::fmodf(base, OFFSCREEN_WIDTH / 2);
        ctx->FillEllipse(
            { { OFFSCREEN_WIDTH / 2, OFFSCREEN_WIDTH / 2 }, width, width },
            g_data.d2d_brush
        );
        const auto hr = ctx->EndDraw();
        int bk = 9;
    }

    // D3D
    float clearColor[4] = { 0.4f, 0.8f, 1.0f, 1.0f };
    const auto ctx = g_data.device_context;
    ctx->ClearRenderTargetView(g_data.d3d_rtv, clearColor);
    ctx->ClearDepthStencilView(g_data.d3d_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
    {

        MatrixBufferType mat;
        mat.world = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(s_angle));
        mat.view = DirectX::XMMatrixTranspose(g_data.view);
        mat.projection = DirectX::XMMatrixTranspose(g_data.projection);
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        const auto hr = ctx->Map(g_data.d3d_cube_cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            std::memcpy(mapped.pData, &mat, sizeof(mat));
            ctx->Unmap(g_data.d3d_cube_cbuffer, 0);
        }

    }
    {
        const UINT stride = sizeof(VertexTex);
        const UINT offset = 0;
        ctx->IASetVertexBuffers(0, 1, &g_data.d3d_cube_vertex, &stride, &offset);
        ctx->IASetIndexBuffer(g_data.d3d_cube_index, DXGI_FORMAT_R16_UINT, 0);

        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->IASetInputLayout(g_data.d3d_cube_layout);
        ctx->VSSetConstantBuffers(0, 1, &g_data.d3d_cube_cbuffer);
        ctx->VSSetShader(g_data.d3d_cube_vs, nullptr, 0);
        ctx->PSSetShader(g_data.d3d_cube_ps, nullptr, 0);
        ctx->PSSetShaderResources(0, 1, &g_data.d3d_texture_view);
        ctx->PSSetSamplers(0, 1, &g_data.d3d_sampler);
        ctx->DrawIndexed(36, 0, 0);
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
        sd.SampleDesc.Count = 1;
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
        dsbuffer_desc.SampleDesc.Count = 1;
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
        hr = g_data.device->CreateBuffer(&matrix_desc, nullptr, &g_data.d3d_cube_cbuffer);
    }
    // 创建彩色立方体
    if (SUCCEEDED(hr)) {
        hr = ::CreateColoredCube();
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
    // 创建纯色笔刷
    if (SUCCEEDED(hr)) {
        hr = g_data.d2d_context->CreateSolidColorBrush(
            D2D1::ColorF(0.f, 0.f, 0.f, 1.f),
            &g_data.d2d_brush
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
    ::SafeRelease(g_data.d2d_brush);
    ::SafeRelease(g_data.d2d_target); 
    ::SafeRelease(g_data.d2d_context);
    ::SafeRelease(g_data.d2d_device);
    ::SafeRelease(g_data.d2d_factory);
    
    ::SafeRelease(g_data.d3d_texture_view);
    ::SafeRelease(g_data.d3d_texture);
    ::SafeRelease(g_data.d3d_sampler);
    ::SafeRelease(g_data.d3d_cube_cbuffer);
    ::SafeRelease(g_data.d3d_cube_ps);
    ::SafeRelease(g_data.d3d_cube_vs);
    ::SafeRelease(g_data.d3d_cube_layout);
    ::SafeRelease(g_data.d3d_cube_index);
    ::SafeRelease(g_data.d3d_cube_vertex);
    ::SafeRelease(g_data.d3d_dsv);
    ::SafeRelease(g_data.d3d_dsv);
    ::SafeRelease(g_data.d3d_rtv);
    ::SafeRelease(g_data.d3d_dsbuffer);
    ::SafeRelease(g_data.device_context);
    ::SafeRelease(g_data.device);
    ::SafeRelease(g_data.swap_chain);
}



const char s_cube_shader[] = u8R"shader(
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
)shader";



// 创建带颜色的正方体
HRESULT CreateColoredCube() noexcept {
    // 立方体的8个顶点 与相应纹理坐标
    const VertexTex vertices[] = {
        { DirectX::XMFLOAT3(-1.f, -1.f, -1.f), DirectX::XMFLOAT2(1.f, 1.f) },
        { DirectX::XMFLOAT3(-1.f, +1.f, -1.f), DirectX::XMFLOAT2(0.f, 1.f) },
        { DirectX::XMFLOAT3(+1.f, +1.f, -1.f), DirectX::XMFLOAT2(0.f, 0.f) },
        { DirectX::XMFLOAT3(+1.f, -1.f, -1.f), DirectX::XMFLOAT2(1.f, 0.f) },

        { DirectX::XMFLOAT3(-1.f, -1.f, +1.f), DirectX::XMFLOAT2(1.f, 0.f) },
        { DirectX::XMFLOAT3(-1.f, +1.f, +1.f), DirectX::XMFLOAT2(0.f, 0.f) },
        { DirectX::XMFLOAT3(+1.f, +1.f, +1.f), DirectX::XMFLOAT2(0.f, 1.f) },
        { DirectX::XMFLOAT3(+1.f, -1.f, +1.f), DirectX::XMFLOAT2(1.f, 1.f) }
    };
    // 立方体 6个面 12个三角面 36个顶点
    const uint16_t indices[] = {
        0, 1, 2, 0, 2, 3,
        4, 5, 1, 4, 1, 0,
        7, 6, 5, 7, 5, 4,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };
    // 输入布局
    const D3D11_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexTex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexTex, tex), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    HRESULT hr = S_OK;
    ID3DBlob* vs = nullptr, *ps = nullptr;
    // 创建顶点缓存
    if (SUCCEEDED(hr)) {
        D3D11_BUFFER_DESC buffer_desc = { 0 };
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.ByteWidth = sizeof(vertices);
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA sub_data = { vertices };
        hr = g_data.device->CreateBuffer(&buffer_desc, &sub_data, &g_data.d3d_cube_vertex);
    }
    // 创建索引缓存
    if (SUCCEEDED(hr)) {
        D3D11_BUFFER_DESC buffer_desc = { 0 };
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.ByteWidth = sizeof(indices);
        buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA sub_data = { indices };
        hr = g_data.device->CreateBuffer(&buffer_desc, &sub_data, &g_data.d3d_cube_index);
    }
    UINT flag = 0;
#if !defined(NDEBUG)
    flag |= D3DCOMPILE_DEBUG;
#endif
    // 编译VS
    if (SUCCEEDED(hr)) {
        hr = ::D3DCompile(
            s_cube_shader, sizeof(s_cube_shader), 
            nullptr, nullptr, nullptr,
            "TexVertexShader", "vs_4_0", 
            flag, 0, &vs, nullptr
        );
    }
    // 编译PS
    if (SUCCEEDED(hr)) {
        hr = ::D3DCompile(
            s_cube_shader, sizeof(s_cube_shader),
            nullptr, nullptr, nullptr,
            "TexPixelShader", "ps_4_0",
            flag, 0, &ps, nullptr
        );
    }
    // 创建VS
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreateVertexShader(
            vs->GetBufferPointer(), 
            vs->GetBufferSize(), 
            nullptr, &g_data.d3d_cube_vs
        );
    }
    // 创建PS
    if (SUCCEEDED(hr)) {
        hr = g_data.device->CreatePixelShader(
            ps->GetBufferPointer(),
            ps->GetBufferSize(),
            nullptr, &g_data.d3d_cube_ps
        );
    }
    // 创建对应输入布局
    if (SUCCEEDED(hr)) {
        g_data.device->CreateInputLayout(
            inputLayout, sizeof(inputLayout)/sizeof(inputLayout[0]), 
            vs->GetBufferPointer(),
            vs->GetBufferSize(), 
            &g_data.d3d_cube_layout
        );
    }
    ::SafeRelease(vs);
    ::SafeRelease(ps);
    return hr;
}

