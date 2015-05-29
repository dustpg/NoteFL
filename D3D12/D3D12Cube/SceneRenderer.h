// SceneRenderer类 主管图形图像渲染

#pragma once


// 常量缓存
struct MatrixBufferType {
    // 世界转换
    DirectX::XMMATRIX   world;
    // 视角转换
    DirectX::XMMATRIX   view;
    // 透视转换
    DirectX::XMMATRIX   projection;
};


// 顶点-颜色 模型
struct VertexColor {
    // 位置
    DirectX::XMFLOAT3   pos;
    // 颜色
    DirectX::XMFLOAT4   color;
};

// 近裁剪距离
static constexpr float  SCREEN_NEAR_Z = (0.01f);
// 远裁剪距离
static constexpr float  SCREEN_FAR_Z = (1000.f);

// 场景渲染
class SceneRenderer {
    // CBV SRV UAV描述符
    enum DescriptorIndexOfCSU : uint32_t {
        // 转换矩阵常量缓存
        CSU_MatrixCBuffer,
        // 大小
        CSU_SIZE,
    };
    // SAMPLER 描述符
    enum DescriptorIndexOfSAM : uint32_t {
        // 保留
        SAM_Unused,
        // 大小
        SAM_SIZE,
    };
    // RTV 描述符
    enum DescriptorIndexOfRTV : uint32_t {
        // 主输出 缓存1
        RTV_MainRTV1,
        // 主输出 缓存2
        RTV_MainRTV2,
        // 缓存数量
        RTV_BUFFER_COUNT,
        // 大小
        RTV_SIZE = RTV_BUFFER_COUNT,
    };
    // DSV 描述符
    enum DescriptorIndexOfDSV : uint32_t {
        // 深度缓存
        DSV_MainDSV,
        // 大小
        DSV_SIZE,
    };
public:
    // 创建带颜色的正方体
    auto CreateColoredCube(
        ID3D12Resource*& vibuffer,
        D3D12_VERTEX_BUFFER_VIEW& vbuffer_view,
        D3D12_INDEX_BUFFER_VIEW& ibuffer_view
        )noexcept->HRESULT;
public:
    // 构造函数
    SceneRenderer() noexcept;
    // 析构函数
    ~SceneRenderer() noexcept;
    // 渲染帧
    auto OnRender(UINT syn) noexcept->HRESULT;
    // 设置窗口句柄
    auto SetHwnd(HWND hwnd) noexcept { m_hwnd = hwnd; return CreateDeviceIndependentResources(); }
private:
    // 初始化命令列表
    auto init_commandlists() noexcept->HRESULT;
    // 更新转换矩阵常量缓存
    void refresh_matrix_cbuffer(float y) noexcept;
public:
    // 创建设备无关资源
    auto CreateDeviceIndependentResources() noexcept->HRESULT;
    // 创建设备有关资源
    auto CreateDeviceResources() noexcept->HRESULT;
    // 丢弃设备有关资源
    void DiscardDeviceResources() noexcept;
    // 设置资源路障(ResourceBarrier)
    void SetResourceBarrier(ID3D12GraphicsCommandList* , ID3D12Resource* , D3D12_RESOURCE_STATES, D3D12_RESOURCE_STATES) noexcept;
private:
    // 缓存宽度
    uint32_t                            m_uBufferWidth = 0;
    // 缓存高度
    uint32_t                            m_uBufferHeight = 0;
    // 帧计数器
    uint32_t                            m_uFrameCount = 0;
    // 上次时间
    uint32_t                            m_dwLastTime = 0;
    // 矩阵输出
    DirectX::XMFLOAT4X4*                m_pCBMatrix = nullptr;
private:
    // 摄像机
    DirectX::XMFLOAT4X4                 m_matCamera;
    // 透视投影矩阵
    DirectX::XMFLOAT4X4                 m_matPerspective;
private:
    // 正方体用资源
    ID3D12Resource*                     m_pResCube = nullptr;
    // 正方体VBV
    D3D12_VERTEX_BUFFER_VIEW            m_cubeVBV;
    // 正方体IBV
    D3D12_INDEX_BUFFER_VIEW             m_cubeIBV;
private:
    // D3D 设备
    ID3D12Device*                       m_pd3dDevice = nullptr;
    // D3D 命令分配器
    ID3D12CommandAllocator*             m_pCmdAllocator = nullptr;
    // D3D 命令队列
    ID3D12CommandQueue*                 m_pCmdQueue = nullptr;
    // 全体CBV, SRV, UAV目标描述符
    ID3D12DescriptorHeap*               m_pCSUDescriptors = nullptr;
    // 全体SAMPLER目标描述符
    ID3D12DescriptorHeap*               m_pSAMDescriptors = nullptr;
    // 全体RTV目标描述符
    ID3D12DescriptorHeap*               m_pRTVDescriptors = nullptr;
    // 全体DSV目标描述符
    ID3D12DescriptorHeap*               m_pDSVDescriptors = nullptr;
    // DXGI 工厂
    IDXGIFactory2*                      m_pDxgiFactory = nullptr;
    // DXGI 交换链
    IDXGISwapChain1*                    m_pSwapChain = nullptr;
    // 渲染管线的 Root Signature
    ID3D12RootSignature*                m_prsPipeline = nullptr;
    // 渲染管线状态对象
    ID3D12PipelineState*                m_pPipelineState = nullptr;
    // D3D Fence
    ID3D12Fence*                        m_pFence = nullptr;
    // D3D 渲染呈现缓冲区
    ID3D12Resource*                     m_pTargetBuffer[RTV_BUFFER_COUNT];
    // D3D 清空图像命令列表
    ID3D12GraphicsCommandList*          m_pCmdClear[RTV_BUFFER_COUNT];
    // D3D 刻画图像命令列表
    ID3D12GraphicsCommandList*          m_pCmdDraw[RTV_BUFFER_COUNT];
    // Fence等待句柄
    HANDLE                              m_hFenceWait = nullptr;
    // 深度缓冲区
    ID3D12Resource*                     m_pDepthBuffer = nullptr;
    // 转换矩阵缓冲区
    ID3D12Resource*                     m_pCBufferMatrix = nullptr;
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
private:
    // CBV SRV UAV描述符句柄
    D3D12_CPU_DESCRIPTOR_HANDLE         m_aCpuHandleCSU[CSU_SIZE];
    // SAMPLER描述符句柄
    D3D12_CPU_DESCRIPTOR_HANDLE         m_aCpuHandleSAM[SAM_SIZE];
    // RTV描述符句柄
    D3D12_CPU_DESCRIPTOR_HANDLE         m_aCpuHandleRTV[RTV_SIZE];
    // DSV描述符句柄
    D3D12_CPU_DESCRIPTOR_HANDLE         m_aCpuHandleDSV[DSV_SIZE];
};