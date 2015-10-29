#include "stdafx.h"
#include "included.h"
#include <d3dcompiler.h>

#undef PixelFormat

//
#ifdef _DEBUG
#define SetDebugName(name) \
    if (SUCCEEDED(hr)) { \
         hr = name->SetName(L#name);\
    }
#else
#define SetDebugName(name) (void)(0)
#endif

// 创建带颜色的正方体
auto SceneRenderer::CreateColoredCube(
    ID3D12Resource*& vibuffer,
    D3D12_VERTEX_BUFFER_VIEW& vbuffer_view,
    D3D12_INDEX_BUFFER_VIEW& ibuffer_view) noexcept -> HRESULT {
    HRESULT hr = S_OK;
    ID3D12Resource* pVIBuffer = nullptr;
    // 立方体的8个顶点 与相应颜色
    VertexColor vertices[] = {
            { DirectX::XMFLOAT3(-1.f, -1.f, -1.f), DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f) },
            { DirectX::XMFLOAT3(-1.f,  1.f, -1.f), DirectX::XMFLOAT4(1.f, 0.f, 0.f, 1.f) },
            { DirectX::XMFLOAT3(1.f,  1.f, -1.f), DirectX::XMFLOAT4(0.f, 1.f, 0.f, 1.f) },
            { DirectX::XMFLOAT3(1.f, -1.f, -1.f), DirectX::XMFLOAT4(0.f, 0.f, 1.f, 1.f) },
            { DirectX::XMFLOAT3(-1.f, -1.f,  1.f), DirectX::XMFLOAT4(0.f, 1.f, 1.f, 1.f) },
            { DirectX::XMFLOAT3(-1.f,  1.f,  1.f), DirectX::XMFLOAT4(1.f, 1.f, 0.f, 1.f) },
            { DirectX::XMFLOAT3(1.f,  1.f,  1.f), DirectX::XMFLOAT4(1.f, 0.f, 1.f, 1.f) },
            { DirectX::XMFLOAT3(1.f, -1.f,  1.f), DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f) }
    };
    // 立方体 6个面 12个三角面 36个顶点
    uint16_t indices[] = {
        0, 1, 2, 0, 2, 3,
        4, 5, 1, 4, 1, 0,
        7, 6, 5, 7, 5, 4,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };

    // 创建顶点缓存-索引缓存共用缓冲区
    if (SUCCEEDED(hr)) {
        D3D12_HEAP_PROPERTIES prop;
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 0;
        prop.VisibleNodeMask = 0;
        //
        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_BUFFER,
            0, sizeof(vertices) + sizeof(indices), 1,
            1, 1,
            DXGI_FORMAT_UNKNOWN, 1, 0,
            D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            D3D12_RESOURCE_FLAG_NONE
        };
        hr = m_pd3dDevice->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_ID3D12Resource,
            reinterpret_cast<void**>(&pVIBuffer)
            );
    }
    // 映射
    void* buffer = nullptr;
    if (SUCCEEDED(hr)) {
        hr = pVIBuffer->Map(0, nullptr, &buffer);
    }
    // 复制-取消映射-设置
    if (SUCCEEDED(hr)) {
        ::memcpy(buffer, vertices, sizeof(vertices));
        ::memcpy(
            reinterpret_cast<uint8_t*>(buffer)+ sizeof(vertices), 
            indices, sizeof(indices)
            );
        pVIBuffer->Unmap(0, nullptr);
        // 设置
        vbuffer_view.BufferLocation = pVIBuffer->GetGPUVirtualAddress();
        vbuffer_view.StrideInBytes = sizeof(VertexColor);
        vbuffer_view.SizeInBytes = sizeof(vertices);
        ibuffer_view.BufferLocation = vbuffer_view.BufferLocation + sizeof(vertices);
        ibuffer_view.Format = DXGI_FORMAT_R16_UINT;
        ibuffer_view.SizeInBytes = sizeof(indices);
        vibuffer = ::SafeAcquire(pVIBuffer);
    }
    ::SafeRelease(pVIBuffer);
    return hr;
}

// SceneRenderer类构造函数
SceneRenderer::SceneRenderer() noexcept {
    ZeroMemory(m_pTargetBuffer, sizeof(m_pTargetBuffer));
    ZeroMemory(m_pCmdClear, sizeof(m_pCmdClear));
    ZeroMemory(m_pCmdDraw, sizeof(m_pCmdDraw));
}

// 初始化命令列表
auto SceneRenderer::init_commandlists() noexcept -> HRESULT {
    HRESULT hr = S_OK;
    D3D12_VIEWPORT view = {
        0.f, 0.f,
        static_cast<float>(m_uBufferWidth),
        static_cast<float>(m_uBufferHeight),
        0.f, 1.f
    };
    // 创建清屏命令
    for (int i = 0;i < lengthof(m_pCmdClear);++i) {
        if (SUCCEEDED(hr)) {
            hr = m_pd3dDevice->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_pCmdAllocator,
                nullptr,
                IID_ID3D12GraphicsCommandList,
                reinterpret_cast<void**>(m_pCmdClear + i)
                );
        }
        // 执行清空命令
        if (SUCCEEDED(hr)) {
            this->SetResourceBarrier(m_pCmdClear[i], m_pTargetBuffer[i], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            m_pCmdClear[i]->RSSetViewports(1, &view);
            //float clearColor[4] = { 0.4f, 0.8f, 1.0f, 1.0f };
            float clearColor[4] = { 0.0f };
            m_pCmdClear[i]->ClearRenderTargetView(
                m_aCpuHandleRTV[RTV_MainRTV1 + i],
                clearColor, 0, nullptr
                );
            m_pCmdClear[i]->ClearDepthStencilView(
                m_aCpuHandleDSV[DSV_MainDSV], D3D12_CLEAR_FLAG_DEPTH, 1.0f,
                0, 0, nullptr
                );
            this->SetResourceBarrier(m_pCmdClear[i], m_pTargetBuffer[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            hr = m_pCmdClear[i]->Close();
        }
    }
    // 创建刻画命令数组
    for (int i = 0;i < lengthof(m_pCmdDraw);++i) {
        // 创建刻画命令
        if (SUCCEEDED(hr)) {
            hr = m_pd3dDevice->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                m_pCmdAllocator,
                nullptr,
                IID_ID3D12GraphicsCommandList,
                reinterpret_cast<void**>(m_pCmdDraw + i)
                );
        }
        // 执行命令
        if (SUCCEEDED(hr)) {
            ID3D12DescriptorHeap* heaps[] = {
                m_pCSUDescriptors
            };
            D3D12_RECT scissor = {};
            scissor.right = m_uBufferWidth;
            scissor.bottom = m_uBufferHeight;
            //
            this->SetResourceBarrier(m_pCmdDraw[i], m_pTargetBuffer[i], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            m_pCmdDraw[i]->RSSetViewports(1, &view);
            m_pCmdDraw[i]->RSSetScissorRects(1, &scissor);
            m_pCmdDraw[i]->OMSetRenderTargets(1, m_aCpuHandleRTV + RTV_MainRTV1 + i, true, m_aCpuHandleDSV + DSV_MainDSV);
            m_pCmdDraw[i]->SetGraphicsRootSignature(m_prsPipeline);
            m_pCmdDraw[i]->SetDescriptorHeaps(lengthof(heaps), heaps);
            m_pCmdDraw[i]->SetGraphicsRootDescriptorTable(0, m_pCSUDescriptors->GetGPUDescriptorHandleForHeapStart());
            m_pCmdDraw[i]->SetPipelineState(m_pPipelineState);
            //m_pCmdDraw->SetGraphicsRootConstantBufferView(0, m_pCBufferMatrix->GetGPUVirtualAddress());
            m_pCmdDraw[i]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            m_pCmdDraw[i]->IASetVertexBuffers(0, 1, &m_cubeVBV);
            m_pCmdDraw[i]->IASetIndexBuffer(&m_cubeIBV);
            m_pCmdDraw[i]->DrawIndexedInstanced(36, 1, 0, 0, 0);
            this->SetResourceBarrier(m_pCmdDraw[i], m_pTargetBuffer[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            hr = m_pCmdDraw[i]->Close();
        }

    }
    // 设置调试名称
    SetDebugName(m_pCmdClear[RTV_MainRTV1]);
    SetDebugName(m_pCmdClear[RTV_MainRTV2]);
    // 设置调试名称
    SetDebugName(m_pCmdDraw[RTV_MainRTV1]);
    SetDebugName(m_pCmdDraw[RTV_MainRTV2]);
    return hr;;
}

// 更新转换矩阵常量缓存
void SceneRenderer::refresh_matrix_cbuffer(float y) noexcept {
    DirectX::XMStoreFloat4x4(
        m_pCBMatrix,
        DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(y))
        );
}

// 创建设备无关资源
auto SceneRenderer::CreateDeviceIndependentResources() noexcept ->HRESULT {
    HRESULT hr = S_OK;
    // 创建Fence事件
    if(SUCCEEDED(hr)) {
        assert(!m_hFenceWait);
        m_hFenceWait = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (!m_hFenceWait || m_hFenceWait == INVALID_HANDLE_VALUE) hr = E_HANDLE;
    }
    
    return hr;
}


// 创建设备资源
auto SceneRenderer::CreateDeviceResources() noexcept ->HRESULT {
    ZeroMemory(&m_matCamera, sizeof(m_matCamera));
    ZeroMemory(&m_matPerspective, sizeof(m_matPerspective));
    HRESULT hr = S_OK;
    // 检查窗口
    if (SUCCEEDED(hr)) {
        RECT rect = { 0 };  ::GetClientRect(m_hwnd, &rect);
        m_uBufferWidth = rect.right - rect.left;
        m_uBufferHeight = rect.bottom - rect.top;
    }
    // 初始化摄像机
    if (SUCCEEDED(hr)) {
        DirectX::XMVECTOR position = { 5.f, 0.f, 0.f, 1.f };
        DirectX::XMVECTOR look_at = { 0.f, 0.f, 0.f, 1.f };
        DirectX::XMVECTOR up = { 0.f, 5.f, 0.f, 1.f };
        auto camera = DirectX::XMMatrixLookAtLH(
            position, look_at, up
            );
        DirectX::XMStoreFloat4x4(&m_matCamera, camera);
    }
    // 初始化透视投影
    if(SUCCEEDED(hr)) {
        // * 视角弧度: 45°
        // * 宽高比: with / height
        // * 近裁剪距离: SCREEN_NEAR_Z
        // * 远裁剪距离: SCREEN_FAR_Z
        auto perspective = DirectX::XMMatrixPerspectiveFovLH(
            45.0f,
            static_cast<float>(m_uBufferWidth) / (static_cast<float>(m_uBufferHeight)),
            SCREEN_NEAR_Z,
            SCREEN_FAR_Z
            );
        DirectX::XMStoreFloat4x4(&m_matPerspective, perspective);
    }
    // 创建Dxgi工厂
    if (SUCCEEDED(hr)) {
        hr = ::CreateDXGIFactory2(
#ifdef _DEBUG
            DXGI_CREATE_FACTORY_DEBUG,
#else
            0,
#endif
            IID_IDXGIFactory2, 
            reinterpret_cast<void**>(&m_pDxgiFactory)
            );
    }
    // 枚举设备
    IDXGIAdapter1* adapter = nullptr;
    if (SUCCEEDED(hr)) {
        UINT32 index = 0;
        while (m_pDxgiFactory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND){
            ++index;
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);
            if (!::wcsncmp(desc.Description, L"NVI", 3))
                break;
            if (!::wcsncmp(desc.Description, L"Mic", 3)) 
                break;
            ::SafeRelease(adapter);
        }
    }
    ID3D12Debug* debug = nullptr;
#ifdef _DEBUG
    if (SUCCEEDED(hr)) {
        hr = ::D3D12GetDebugInterface(
            IID_ID3D12Debug, 
            reinterpret_cast<void**>(&debug)
            );
    }
    // DEBUG
    if(SUCCEEDED(hr)) {
        debug->EnableDebugLayer();
    }
#endif
    // 创建 D3D12设备: 目前使用 WARP 驱动
    if (SUCCEEDED(hr)) {
        hr = ::D3D12CreateDevice(
            adapter,
            D3D_FEATURE_LEVEL_11_0,
            IID_ID3D12Device,
            reinterpret_cast<void**>(&m_pd3dDevice)
            );
    }
    ::SafeRelease(debug);
    ::SafeRelease(adapter);
    // 创建命令分配器
    if (SUCCEEDED(hr)) {
        hr = m_pd3dDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_ID3D12CommandAllocator,
            reinterpret_cast<void**>(&m_pCmdAllocator)
            );
    }
    // 创建命令队列
    if (SUCCEEDED(hr)) {
        D3D12_COMMAND_QUEUE_DESC desc = {
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            0,
            D3D12_COMMAND_QUEUE_FLAG_NONE,
            0
        };
        hr = m_pd3dDevice->CreateCommandQueue(
            &desc,
            IID_ID3D12CommandQueue,
            reinterpret_cast<void**>(&m_pCmdQueue)
            );
    }
    // 创建交换链
    if (SUCCEEDED(hr)) {
        // 交换链信息
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
        swapChainDesc.Width = m_uBufferWidth;
        swapChainDesc.Height = m_uBufferHeight;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = RTV_BUFFER_COUNT;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.Flags = 0;
#ifdef USING_DirectComposition
        swapChainDesc.Flags = 
            DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        // DirectComposition桌面应用程序
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        // 创建DirectComposition交换链
        hr = m_pDxgiFactory->CreateSwapChainForComposition(
            m_pCmdQueue,
            &swapChainDesc,
            nullptr,
            &m_pSwapChain
            );
        if (SUCCEEDED(hr)) {
            IDXGISwapChain3* chain = nullptr;
            hr = m_pSwapChain->QueryInterface(
                IID_IDXGISwapChain3,
                reinterpret_cast<void**>(&chain)
                );
            if (SUCCEEDED(hr)) {
                hr = chain->SetMaximumFrameLatency(1);
            }
            ::SafeRelease(chain);
        }
#else
        // 一般桌面应用程序
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        // 利用窗口句柄创建交换链
        hr = m_pDxgiFactory->CreateSwapChainForHwnd(
            m_pCmdQueue,
            m_hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &m_pSwapChain
            );
#endif
    }
    /*// 确保DXGI队列里边不会超过一帧
    if (SUCCEEDED(hr)) {
        hr = m_pDxgiDevice->SetMaximumFrameLatency(1);
    }*/
    // 创建CBV, SRV, UAV描述符
    if (SUCCEEDED(hr)) {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            CSU_SIZE,
            D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            0
        };
        hr = m_pd3dDevice->CreateDescriptorHeap(
            &desc, IID_ID3D12DescriptorHeap,
            reinterpret_cast<void**>(&m_pCSUDescriptors)
            );
    }
    // 创建SAM描述符
    if (SUCCEEDED(hr)) {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {
            D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
            SAM_SIZE,
            D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            0
        };
        hr = m_pd3dDevice->CreateDescriptorHeap(
            &desc, IID_ID3D12DescriptorHeap,
            reinterpret_cast<void**>(&m_pSAMDescriptors)
            );
    }
    // 创建RTV描述符
    if (SUCCEEDED(hr)) {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            RTV_SIZE,
            D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            0
        };
        hr = m_pd3dDevice->CreateDescriptorHeap(
            &desc, IID_ID3D12DescriptorHeap,
            reinterpret_cast<void**>(&m_pRTVDescriptors)
            );
    }
    // 创建DSV描述符
    if (SUCCEEDED(hr)) {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {
            D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            DSV_SIZE,
            D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            0
        };
        hr = m_pd3dDevice->CreateDescriptorHeap(
            &desc, IID_ID3D12DescriptorHeap,
            reinterpret_cast<void**>(&m_pDSVDescriptors)
            );
    }
    // 设置描述符句柄
    if (SUCCEEDED(hr)) {
#define SETUP_CPU_DESCRIPTOR(v, a) \
        {\
            auto first = v->GetCPUDescriptorHandleForHeapStart();\
            for(int i = 0; i < lengthof(a); ++i){\
                a[i] = first;\
                first.ptr += offset;\
            }\
        }
#define SETUP_CPU_DESCRIPTOR_LEVEL2(a) SETUP_CPU_DESCRIPTOR(m_p##a##Descriptors, m_aCpuHandle##a)
        auto offset = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        SETUP_CPU_DESCRIPTOR_LEVEL2(CSU);
        offset = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        SETUP_CPU_DESCRIPTOR_LEVEL2(SAM);
        offset = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        SETUP_CPU_DESCRIPTOR_LEVEL2(RTV);
        offset = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        SETUP_CPU_DESCRIPTOR_LEVEL2(DSV);
    }
    // 获取所有缓冲区
    for (int i = 0; i < RTV_BUFFER_COUNT; ++i) {
        // 获取缓冲区
        if (SUCCEEDED(hr)) {
            hr = m_pSwapChain->GetBuffer(
                i, IID_ID3D12Resource,
                reinterpret_cast<void**>(m_pTargetBuffer+i)
                );
        }
        // 绑定目标缓存到RTV
        if (SUCCEEDED(hr)) {
            m_pd3dDevice->CreateRenderTargetView(
                m_pTargetBuffer[i],
                nullptr,
                m_aCpuHandleRTV[RTV_MainRTV1 + i]
                );
        }
    }
    // 创建常量缓存
    if (SUCCEEDED(hr)) {
        D3D12_HEAP_PROPERTIES prop;
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 0;
        prop.VisibleNodeMask = 0;
        //
        D3D12_RESOURCE_DESC desc = {
            D3D12_RESOURCE_DIMENSION_BUFFER,
            0, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, 1,
            1, 1,
            DXGI_FORMAT_UNKNOWN, 1, 0,
            D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            D3D12_RESOURCE_FLAG_NONE
        };
        hr = m_pd3dDevice->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_ID3D12Resource,
            reinterpret_cast<void**>(&m_pCBufferMatrix)
            );
        static_assert(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT >= sizeof(MatrixBufferType), "reset it");
    }
    // 绑定到常量缓存视图
    if (SUCCEEDED(hr)) {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_pCBufferMatrix->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
        m_pd3dDevice->CreateConstantBufferView(
            &cbvDesc,
            m_aCpuHandleCSU[CSU_MatrixCBuffer]
            );
        hr = m_pCBufferMatrix->Map(0, nullptr, reinterpret_cast<void**>(&m_pCBMatrix));
        if (SUCCEEDED(hr)) {
            MatrixBufferType mat;
            mat.world = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(0.f));
            mat.view = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_matCamera));
            mat.projection = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&m_matPerspective));
            ::memcpy(m_pCBMatrix, &mat, sizeof(mat));
        }
    }
    // 创建Fence
    if (SUCCEEDED(hr)) {
        m_pd3dDevice->CreateFence(
            0, D3D12_FENCE_FLAG_NONE,
            IID_ID3D12Fence,
            reinterpret_cast<void**>(&m_pFence)
            );
    }
    // 创建深度缓存
    if (SUCCEEDED(hr)) {
        /*
    static CD3D12_RESOURCE_DESC Tex2D( 
        DXGI_FORMAT format,
        UINT64 width,
        UINT height,
        UINT16 arraySize = 1,
        UINT16 mipLevels = 0,
        UINT sampleCount = 1,
        UINT sampleQuality = 0,
        D3D12_RESOURCE_MISC_FLAG miscFlags = D3D12_RESOURCE_MISC_NONE,
        D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        UINT64 alignment = 0 )
    {
        return CD3D12_RESOURCE_DESC( D3D12_RESOURCE_DIMENSION_TEXTURE_2D, alignment, width, height, arraySize, 
            mipLevels, format, sampleCount, sampleQuality, layout, miscFlags );
    }
        */
        D3D12_RESOURCE_DESC resourceDesc = {
            D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            0, 
            m_uBufferWidth, m_uBufferHeight, 1,
            1, DXGI_FORMAT_R32_TYPELESS,
            1 , 0,
            D3D12_TEXTURE_LAYOUT_UNKNOWN,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        };

        D3D12_HEAP_PROPERTIES prop;
        prop.Type = D3D12_HEAP_TYPE_DEFAULT;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 0;

        D3D12_CLEAR_VALUE dsvClearValue;
        dsvClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        dsvClearValue.DepthStencil.Depth = 1.0f;
        dsvClearValue.DepthStencil.Stencil = 0;
        hr = m_pd3dDevice->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_DEPTH_READ,
            &dsvClearValue,
            IID_ID3D12Resource,
            reinterpret_cast<void**>(&m_pDepthBuffer)
            );
    }
    // 绑定深度缓存到DSV
    if (SUCCEEDED(hr)) {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.Texture2D.MipSlice = 0;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        /*no return*/m_pd3dDevice->CreateDepthStencilView(
            m_pDepthBuffer, &dsvDesc,
            m_aCpuHandleDSV[DSV_MainDSV]
            );
    }
    /*// 绑定深度缓存到SRV
    if (SUCCEEDED(hr)) {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0; // No MIP
        srvDesc.Texture2D.PlaneSlice = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        m_pd3dDevice->CreateShaderResourceView(
            m_pDepthBuffer, &srvDesc,
            m_aCpuHandleCSU[CSU_DepthBuffer]
            );
    }*/
    // 创建渲染管线状态对象
    if (SUCCEEDED(hr)) {
        ID3DBlob *sig = nullptr, *info = nullptr;
        ID3DBlob *ps = nullptr, *vs = nullptr;
        D3D12_ROOT_SIGNATURE_DESC rootSigDesc ;
        D3D12_ROOT_PARAMETER params[1];
        D3D12_DESCRIPTOR_RANGE descRange[1];
        descRange[0] = {
            D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
            1, 0,
            0,D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
        };
        // 初始化为DescriptorTable
        params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[0].DescriptorTable = { lengthof(descRange), descRange };
        params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        // 初始化
        rootSigDesc.NumParameters = lengthof(params);
        rootSigDesc.pParameters = params;
        rootSigDesc.NumStaticSamplers = 0;
        rootSigDesc.pStaticSamplers = nullptr;
        rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        // 先序列化RootSignature
        if (SUCCEEDED(hr = ::D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &info))) {
            // 再创建RootSignature
            hr = m_pd3dDevice->CreateRootSignature(
                0, sig->GetBufferPointer(), sig->GetBufferSize(),
                IID_ID3D12RootSignature,
                reinterpret_cast<void**>(&m_prsPipeline)
                );
        }
        UINT flag = 0;
#if _DEBUG
        flag |= D3DCOMPILE_DEBUG;
#endif
        // 编译VS
        if (SUCCEEDED(hr)) {
            ::SafeRelease(info);
            hr = ::D3DCompileFromFile(
                L"shader2in1.hlsl", nullptr, nullptr, 
                "ColorVertexShader", "vs_5_0", flag, 0,
                &vs, &info
                );
            assert(SUCCEEDED(hr) && "Failed to D3DCompileFromFile");
        }
        // 编译PS
        if (SUCCEEDED(hr)) {
            ::SafeRelease(info);
            hr = ::D3DCompileFromFile(
                L"shader2in1.hlsl", nullptr, nullptr, 
                "ColorPixelShader", "ps_5_0", flag, 0, 
                &ps, &info
                );
            assert(SUCCEEDED(hr) && "Failed to D3DCompileFromFile");
        }
        // 创建PSO
        if (SUCCEEDED(hr)) {
#if 1
            struct  CD3D12_DEFAULT  {  } D3D12_DEFAULT;
            struct CD3D12_BLEND_DESC : public D3D12_BLEND_DESC
            {
                CD3D12_BLEND_DESC()
                {}
                explicit CD3D12_BLEND_DESC(const D3D12_BLEND_DESC& o) :
                    D3D12_BLEND_DESC(o)
                {}
                explicit CD3D12_BLEND_DESC(CD3D12_DEFAULT)
                {
                    AlphaToCoverageEnable = FALSE;
                    IndependentBlendEnable = FALSE;
                    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
                    {
                        FALSE,FALSE,
                        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                        D3D12_LOGIC_OP_NOOP,
                        D3D12_COLOR_WRITE_ENABLE_ALL,
                    };
                    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
                        RenderTarget[i] = defaultRenderTargetBlendDesc;
                }
                ~CD3D12_BLEND_DESC() {}
                operator const D3D12_BLEND_DESC&() const { return *this; }
            };
            struct CD3D12_RASTERIZER_DESC : public D3D12_RASTERIZER_DESC
            {
                CD3D12_RASTERIZER_DESC()
                {}
                explicit CD3D12_RASTERIZER_DESC(const D3D12_RASTERIZER_DESC& o) :
                    D3D12_RASTERIZER_DESC(o)
                {}
                explicit CD3D12_RASTERIZER_DESC(CD3D12_DEFAULT)
                {
                    FillMode = D3D12_FILL_MODE_SOLID;
                    CullMode = D3D12_CULL_MODE_BACK;
                    FrontCounterClockwise = FALSE;
                    DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
                    DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
                    SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
                    DepthClipEnable = TRUE;
                    MultisampleEnable = FALSE;
                    AntialiasedLineEnable = FALSE;
                    ForcedSampleCount = 0;
                    ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
                }
                explicit CD3D12_RASTERIZER_DESC(
                    D3D12_FILL_MODE fillMode,
                    D3D12_CULL_MODE cullMode,
                    BOOL frontCounterClockwise,
                    INT depthBias,
                    FLOAT depthBiasClamp,
                    FLOAT slopeScaledDepthBias,
                    BOOL depthClipEnable,
                    BOOL multisampleEnable,
                    BOOL antialiasedLineEnable,
                    UINT forcedSampleCount,
                    D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster)
                {
                    FillMode = fillMode;
                    CullMode = cullMode;
                    FrontCounterClockwise = frontCounterClockwise;
                    DepthBias = depthBias;
                    DepthBiasClamp = depthBiasClamp;
                    SlopeScaledDepthBias = slopeScaledDepthBias;
                    DepthClipEnable = depthClipEnable;
                    MultisampleEnable = multisampleEnable;
                    AntialiasedLineEnable = antialiasedLineEnable;
                    ForcedSampleCount = forcedSampleCount;
                    ConservativeRaster = conservativeRaster;
                }
                ~CD3D12_RASTERIZER_DESC() {}
                operator const D3D12_RASTERIZER_DESC&() const { return *this; }
            };
#endif
            // 输入布局
            D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(DirectX::XMFLOAT3), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            };
            // 配置PSO
            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {
                // Root Signature
                m_prsPipeline,
                // VS
                { vs->GetBufferPointer(), vs->GetBufferSize() },
                // PS
                { ps->GetBufferPointer(), ps->GetBufferSize() },
                // DS
                { nullptr, 0 },
                // HS
                { nullptr, 0 },
                // GS
                { nullptr, 0 },
                // SO
                { nullptr, 0, nullptr, 0, 0 },
                // 合成
                CD3D12_BLEND_DESC(D3D12_DEFAULT),
                // 采样掩码
                UINT32_MAX,
                // 光栅化
                CD3D12_RASTERIZER_DESC(D3D12_DEFAULT),
                // 深度/模板
                { TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS_EQUAL, 0 },
                // 输入布局
                { inputLayout, lengthof(inputLayout) },
                // 索引缓存
                D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
                // 拓扑类型设置为三角
                D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
                // 1个RTV
                1,
                // RGBA
                { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN },
                // DSV
                DXGI_FORMAT_D32_FLOAT,
                // 采样
                { 1, 0 },
                // 节点掩码
                0,
                // 缓存管线
                { nullptr, 0 }
            };
            desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            // 创建PSO
            hr = m_pd3dDevice->CreateGraphicsPipelineState(
                &desc, IID_ID3D12PipelineState,
                reinterpret_cast<void**>(&m_pPipelineState)
                );

        }
        ::SafeRelease(sig);
        ::SafeRelease(info);
        ::SafeRelease(ps);
        ::SafeRelease(vs);
    }
#ifdef USING_DirectComposition
    // 创建直接组合(Direct Composition)设备
    if (SUCCEEDED(hr)) {
        hr = ::DCompositionCreateDevice(
            nullptr,
            IID_PPV_ARGS(&m_pDcompDevice)
            );
    }
    // 创建直接组合(Direct Composition)目标
    if (SUCCEEDED(hr)) {
        hr = m_pDcompDevice->CreateTargetForHwnd(
            m_hwnd, false, &m_pDcompTarget
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
    // 创建正方体
    if (SUCCEEDED(hr)) {
        hr = this->CreateColoredCube(m_pResCube, m_cubeVBV, m_cubeIBV);
    }
    // 设置调试名称
    SetDebugName(m_pd3dDevice);
    // 设置调试名称
    SetDebugName(m_pCmdAllocator);
    // 设置调试名称
    SetDebugName(m_pCmdQueue);
    // 设置调试名称
    SetDebugName(m_pTargetBuffer[RTV_MainRTV1]);
    // 设置调试名称
    SetDebugName(m_pTargetBuffer[RTV_MainRTV2]);
    // 设置调试名称
    SetDebugName(m_pSAMDescriptors);
    // 设置调试名称
    SetDebugName(m_pCSUDescriptors);
    // 设置调试名称
    SetDebugName(m_pRTVDescriptors);
    // 设置调试名称
    SetDebugName(m_pDSVDescriptors);
    // 设置调试名称
    SetDebugName(m_prsPipeline);
    // 设置调试名称
    SetDebugName(m_pPipelineState);
    // 设置调试名称
    SetDebugName(m_pDepthBuffer);
    // 设置调试名称
    SetDebugName(m_pCBufferMatrix);
    // 设置调试名称
    SetDebugName(m_pResCube);
    // 设置命令
    if (SUCCEEDED(hr)) {
        hr = this->init_commandlists();
    }
    return hr;
}

// SceneRenderer析构函数
SceneRenderer::~SceneRenderer() noexcept {
    this->DiscardDeviceResources();
    // 关闭事件
    if (m_hFenceWait) {
        ::CloseHandle(m_hFenceWait);
        m_hFenceWait = nullptr;
    }
}

// 丢弃设备相关资源
void SceneRenderer::DiscardDeviceResources() noexcept {
    // DirectComposition
#ifdef USING_DirectComposition
    ::SafeRelease(m_pDcompDevice);
    ::SafeRelease(m_pDcompTarget);
    ::SafeRelease(m_pDcompVisual);
#endif
    if (m_pCBufferMatrix) {
        m_pCBufferMatrix->Unmap(0, nullptr);
    }
    ::SafeRelease(m_pFence);
    ::SafeRelease(m_pDepthBuffer);
    ::SafeRelease(m_pCBufferMatrix);
    ::SafeRelease(m_pResCube);
    ::SafeRelease(m_prsPipeline);
    ::SafeRelease(m_pPipelineState);
    ::SafeRelease(m_pCmdQueue);
    ::SafeRelease(m_pSAMDescriptors);
    ::SafeRelease(m_pCSUDescriptors);
    ::SafeRelease(m_pRTVDescriptors);
    ::SafeRelease(m_pDSVDescriptors);
    for (auto& i : m_pTargetBuffer) ::SafeRelease(i);
    for (auto& i : m_pCmdDraw) ::SafeRelease(i);
    for (auto& i : m_pCmdClear) ::SafeRelease(i);
    ::SafeRelease(m_pCmdAllocator);
    ::SafeRelease(m_pSwapChain);
    ::SafeRelease(m_pd3dDevice);
    ::SafeRelease(m_pDxgiFactory);
}


// 设置资源路障(ResourceBarrier)
void SceneRenderer::SetResourceBarrier(ID3D12GraphicsCommandList* commandList,
    ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter) noexcept {
    D3D12_RESOURCE_BARRIER barrier = {};

    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = resource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = StateBefore;
    barrier.Transition.StateAfter = StateAfter;

    commandList->ResourceBarrier(1, &barrier);
}

// 渲染图形图像
auto SceneRenderer::OnRender(UINT syn) noexcept->HRESULT {
    HRESULT hr = S_OK;
    // 没有就创建
    if (!m_pd3dDevice) {
        hr = this->CreateDeviceResources();
        assert(SUCCEEDED(hr) && "Failed to call CreateDeviceResources");
#ifdef _DEBUG
        static bool first = true;
        if(!first) ::MessageBoxW(m_hwnd, L"Create SUCCEEDED", L"Tips", MB_OK);
        first = false;
#endif
        m_dwLastTime = ::timeGetTime();
        ++m_uFrameCount;
    }
    // 成功就渲染
    if (SUCCEEDED(hr)) {
        static float y = 0.f; y += 0.08f;
        if (y > DirectX::XM_2PI) y = 0.f;
        this->refresh_matrix_cbuffer(y);
        // 执行命令列表
        //::OutputDebugStringW(L"+1");
        ID3D12CommandList* const lists[] = { 
            m_pCmdClear[(m_uFrameCount + 1) % RTV_BUFFER_COUNT],
            m_pCmdDraw[(m_uFrameCount+1) % RTV_BUFFER_COUNT]
        };
        m_pCmdQueue->ExecuteCommandLists(lengthof(lists), lists);
        // 呈现目标
        hr = m_pSwapChain->Present(syn, 0);
        assert(SUCCEEDED(hr));
        // 设置命令队列flush事件
        m_pFence->SetEventOnCompletion(m_uFrameCount, m_hFenceWait);
        // 等待执行
        hr = m_pCmdQueue->Signal(m_pFence, m_uFrameCount);
        assert(SUCCEEDED(hr));
        DWORD wait = ::WaitForSingleObject(m_hFenceWait, 1000);
        assert(wait == WAIT_OBJECT_0);
        ++m_uFrameCount;
        // 显示FPS
        constexpr uint32_t FRAME_CHECK_COUNT = 41;
        if (m_uFrameCount % FRAME_CHECK_COUNT == 10) {
            register auto dwTime = ::timeGetTime();
            register auto dwDelta = dwTime - m_dwLastTime;
            m_dwLastTime = dwTime;
            register float fps = float(FRAME_CHECK_COUNT) / (float(dwDelta) * 0.001f);
            wchar_t buffer[1024];
            ::swprintf(buffer, 1024, L"%s -- %3.2f FPS", ::WINDOW_TITLE, fps);
            ::SetWindowTextW(m_hwnd, buffer);
        }
    }

    // 设备丢失?
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        this->CreateDeviceResources();
        hr = S_FALSE;
    }
    return hr;
}

