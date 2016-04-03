#include "pfdUIMap.h"
#include <Control/UIContainer.h>
#include <Core/luiManager.h>
#include  "pfdAlgorithm.h"



/// <summary>
/// Initializes a new instance of the <see cref="UIMapControl"/> class.
/// </summary>
/// <param name="cp">The cp.</param>
PathFD::UIMapControl::UIMapControl(LongUI::UIContainer * cp) noexcept : Super(cp) {
    std::memset(m_bufCharData, 0, sizeof(m_bufCharData));
    // 地图数据
    m_dataMap.cell_width = CELL_WIDTH_INIT;
    m_dataMap.cell_height = CELL_HEIGHT_INIT;
    // 角色数据
    auto& chardata = this->get_char_data();
    chardata.width = 32;
    chardata.height = 32;
    chardata.atime = 0.5f;
    chardata.speed = 4.f;
    chardata.acount = 4;
    chardata.action[0] = 1;
    chardata.action[1] = 0;
    chardata.action[2] = 1;
    chardata.action[3] = 2;
}

/// <summary>
/// Initializes the specified node.
/// </summary>
/// <param name="node">The node.</param>
/// <returns></returns>
void PathFD::UIMapControl::initialize(pugi::xml_node node) noexcept {
    // 链式调用
    Super::initialize(node);
    const char* str = nullptr;
    // 获取
    if ((str = node.attribute("charbitmap").value())) {
        m_uCharBitmap = static_cast<uint16_t>(LongUI::AtoI(str));
    }
    // 获取
    if ((str = node.attribute("mapbitmap").value())) {
        m_uMapBitmap = static_cast<uint16_t>(LongUI::AtoI(str));
    }
    // 获取
    if ((str = node.attribute("mapicon").value())) {
        m_uMapIcon = static_cast<uint16_t>(LongUI::AtoI(str));
    }
    assert(m_uCharBitmap && m_uMapBitmap && m_uMapIcon);
}



// ------------------------- 地图逻辑

// 地图
void PathFD::UIMapControl::ResizeCellSize(uint32_t width, uint32_t height) noexcept {
    assert(width && height && "bad arguments");
    m_dataMap.cell_width = width;
    m_dataMap.cell_height = height;
    assert(!"NOIMPL!");
}


/// <summary>
/// 生成地图
/// </summary>
/// <param name="width">The width.</param>
/// <param name="height">The height.</param>
/// <returns></returns>
void PathFD::UIMapControl::GenerateMap(uint32_t width, uint32_t height) noexcept {
    if (!m_pMapSpriteBatch) return;
    assert(width > 1 && height > 1 && "bad arguments");
    assert(width <= MAX_WIDTH && height <= MAX_HEIGHT && "bad arguments");
    m_dataMap.map_width = width;
    m_dataMap.map_height = height;
    auto sz = width * height;
    // 需要重新申请数据
    if (m_uMapSpriteCount < sz) {
        LongUI::NormalFree(m_pMapCells);
        m_pMapCells = LongUI::NormalAllocT<uint8_t>(sz);
        // 内存不足
        if (!m_pMapCells) return assert(!"ERROR");
        auto added = sz -m_uMapSpriteCount ;
        D2D1_RECT_F rect = { 0.f };
        // 申请精灵
        auto hr = m_pMapSpriteBatch->AddSprites(
            added,
            &rect, nullptr, nullptr, nullptr,
            0,        0,     0,       0
        );
        // 失败
        if (FAILED(hr)) {
            m_pMapSpriteBatch->Release();
            m_pMapSpriteBatch = nullptr;
            UIManager.ShowError(hr);
            return;
        }
        m_uMapSpriteCount = sz;
    }
    // 清空
    std::memset(m_pMapCells, 0, sz);
    uint32_t pos[2] = { 0 };
    // 生成地图
    m_fnGeneration(m_pMapCells, width, height, pos);
    // 重置角色地图
    {
        m_dataMap.map_data = m_pMapCells;
        m_dataMap.char_x = pos[0] % m_dataMap.map_width;
        m_dataMap.char_y = pos[0] / m_dataMap.map_width;
        m_char.ResetMap(m_dataMap);
        // 终点
        m_uGoalX = pos[1] % m_dataMap.map_width;
        m_uGoalY = pos[1] / m_dataMap.map_width;
    }
    // 设置控件大小
    this->SetWidth(float(m_dataMap.cell_width * width));
    this->SetHeight(float(m_dataMap.cell_height * height));
    // 重置地图精灵
    this->reset_sprites();
    // 重绘地图
    this->parent->SetControlLayoutChanged();
    this->parent->InvalidateThis();
}


/// <summary>
/// 重置精灵
/// </summary>
/// <returns></returns>
void PathFD::UIMapControl::reset_sprites() noexcept {
    // 无效
    if (!m_pMapSpriteBatch) return;
    float cllw = float(m_dataMap.cell_width);
    float cllh = float(m_dataMap.cell_height);
    // 源矩形
    D2D1_RECT_U srcs[] = {
        { 
            0, 
            0, 
            m_dataMap.cell_width, 
            m_dataMap.cell_height 
        },
        { 
            m_dataMap.cell_width, 
            0, 
            m_dataMap.cell_width + m_dataMap.cell_width, 
            m_dataMap.cell_height
        },
    };
    // TODO: SetSpriteSSSSS
    // 遍历设置
    D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::White, 1.f);
    for (uint32_t y = 0; y != m_dataMap.map_height; ++y) {
        for (uint32_t x = 0; x < m_dataMap.map_width; ++x) {
            uint32_t index = x + y * m_dataMap.map_width;
            D2D1_RECT_F des;
            // 设置目标矩形
            des.left = float(x) * cllw;
            des.top = float(y) * cllh;
            des.right =  des.left +  cllw;
            des.bottom = des.top + cllh;
            // 设置
            m_pMapSpriteBatch->SetSprites(
                index, 1,
                &des, srcs + m_pMapCells[index],
                &color, nullptr, 0, 0, 0, 0
            );
        }
    }
}

// ------------------------- 地图控制


/// <summary>
/// 清理本控件
/// </summary>
/// <returns></returns>
void PathFD::UIMapControl::cleanup() noexcept {
    // 删前调用
    this->before_deleted();
    // 释放空间
    delete this;
}

/// <summary>
/// 释放设备资源
/// </summary>
/// <returns></returns>
void PathFD::UIMapControl::release_resource() noexcept {
    LongUI::SafeRelease(m_pCellBoundaryBrush);
    LongUI::SafeRelease(m_pMapSpriteBatch);
    LongUI::SafeRelease(m_pAutoTileCache);
    LongUI::SafeRelease(m_pPathDisplay);
    LongUI::SafeRelease(m_pMapIcon);
    LongUI::SafeRelease(m_pMapSkin);
}

/// <summary>
/// <see cref="UIMapControl"/> 析构函数
/// </summary>
/// <returns></returns>
PathFD::UIMapControl::~UIMapControl() noexcept {
    this->release_resource(); 
    if (m_pMapCells) {
        LongUI::NormalFree(m_pMapCells);
        m_pMapCells = nullptr;
    }
    if (m_pPath) {
        std::free(m_pPath);
        m_pPath = nullptr;
    }
}

/// <summary>
/// 渲染链-渲染背景
/// </summary>
/// <returns></returns>
void PathFD::UIMapControl::render_chain_background() const noexcept {
    // 父类背景
    Super::render_chain_background();
    // 强行刷新
    UIManager_RenderTarget->Flush();
    // 本类背景
    if (!m_pMapSpriteBatch || !m_uMapSpriteCount) return;
    // 对齐网格
#ifdef PATHFD_ALIGNED
    D2D1_MATRIX_3X2_F transform1, transform2; 
    UIManager_RenderTarget->GetTransform(&transform1);
    transform2 = transform1;
#if 1
    transform2._31 = float(int(transform1._31));
    transform2._32 = float(int(transform1._32));
#else
    transform2._31 = std::floor(transform1._31);
    transform2._32 = std::floor(transform1._32);
#endif
    UIManager_RenderTarget->SetTransform(&transform2);
#endif
    // 精灵集需要取消抗锯齿模式
    UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    // 渲染精灵集
    UIManager_RenderTarget->DrawSpriteBatch(
        m_pMapSpriteBatch, 
        0, m_dataMap.map_width * m_dataMap.map_height,
        m_pMapIcon,
        D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
    );
    // 地图分界线
    UIManager_RenderTarget->FillRectangle(
        D2D1::RectF(
            0.f,
            0.f,
            float(m_dataMap.cell_width * m_dataMap.map_width),
            float(m_dataMap.cell_height * m_dataMap.map_height)
        ),
        m_pCellBoundaryBrush
    );
    // 渲染起点
    {
        D2D1_RECT_F des;
        // 目标矩形
        des.left = float(m_dataMap.char_x * m_dataMap.cell_width);
        des.top = float(m_dataMap.char_y * m_dataMap.cell_height);
        des.right =  des.left + float(m_dataMap.cell_width);
        des.bottom = des.top + float(m_dataMap.cell_height);
        // 源矩形
        D2D1_RECT_F src;
        src.left = 0.f;
        src.top = float(m_dataMap.cell_height);
        src.right = src.left + float(m_dataMap.cell_width);
        src.bottom = src.top + float(m_dataMap.cell_height);
        // 刻画
        UIManager_RenderTarget->DrawBitmap(
            m_pMapIcon,
            &des,
            1.f,
            //D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            &src
        );
    }
    // 渲染终点
    {
        D2D1_RECT_F des;
        // 目标矩形
        des.left = float(m_uGoalX * m_dataMap.cell_width);
        des.top = float(m_uGoalY * m_dataMap.cell_height);
        des.right =  des.left + float(m_dataMap.cell_width);
        des.bottom = des.top + float(m_dataMap.cell_height);
        // 源矩形
        D2D1_RECT_F src;
        src.left = float(m_dataMap.cell_width);
        src.top = float(m_dataMap.cell_height);
        src.right = src.left + float(m_dataMap.cell_width);
        src.bottom = src.top + float(m_dataMap.cell_height);
        // 刻画
        UIManager_RenderTarget->DrawBitmap(
            m_pMapIcon,
            &des,
            1.f,
            //D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
            &src
        );
    }
    // 渲染角色
    m_char.Render();
    // 渲染选择框
    if (m_uClickX < m_dataMap.map_width && m_uClickY < m_dataMap.map_height) {
        D2D1_RECT_F rect;
        // 设置
        rect.left = float(m_uClickX * m_dataMap.cell_width) + CEEL_SELECT_WIDTH * 0.5f;
        rect.top = float(m_uClickY * m_dataMap.cell_height) + CEEL_SELECT_WIDTH * 0.5f;
        rect.right = rect.left + float(m_dataMap.cell_width) - CEEL_SELECT_WIDTH;
        rect.bottom = rect.top + float(m_dataMap.cell_height) - CEEL_SELECT_WIDTH;
        // 渲染
        m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        UIManager_RenderTarget->DrawRectangle(&rect, m_pBrush_SetBeforeUse, CEEL_SELECT_WIDTH);
        m_pBrush_SetBeforeUse->SetColor(D2D1::ColorF(D2D1::ColorF::White));
        UIManager_RenderTarget->DrawRectangle(&rect, m_pBrush_SetBeforeUse, CEEL_SELECT_WIDTH * 0.5f);
    }
    // 设置回来
    UIManager_RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
#ifdef PATHFD_ALIGNED
    UIManager_RenderTarget->SetTransform(&transform1);
#endif
}


/// <summary>
/// 渲染链-渲染主体
/// </summary>
/// <returns></returns>
void PathFD::UIMapControl::render_chain_main() const noexcept {
    return Super::render_chain_main();
}

/// <summary>
/// 渲染链-渲染前景
/// </summary>
/// <returns></returns>
void PathFD::UIMapControl::render_chain_foreground() const noexcept {
    return Super::render_chain_foreground();
}

/// <summary>
/// 渲染本控件
/// </summary>
/// <returns></returns>
void PathFD::UIMapControl::Render() const noexcept {
    // 背景渲染
    this->render_chain_background();
    // 主景渲染
    this->render_chain_main();
    // 前景渲染
    this->render_chain_foreground();
}


/// <summary>
/// 重建控件资源
/// </summary>
/// <returns></returns>
auto PathFD::UIMapControl::Recreate() noexcept -> HRESULT {
    // 初始化
    HRESULT hr = S_OK;
    ID2D1Bitmap1* pCellBitmapBoundary = nullptr;
    ID2D1BitmapRenderTarget* pBitmapRenderTarget = nullptr;
    size_t count = 4 * m_dataMap.cell_width * m_dataMap.cell_height;
    // 先释放
    this->release_resource();
    // 创建位图呈现器
    if (SUCCEEDED(hr)) {
        D2D1_PIXEL_FORMAT format = D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM, 
            D2D1_ALPHA_MODE_PREMULTIPLIED
        );
        D2D1_SIZE_F size = D2D1::SizeF(
            float(m_dataMap.cell_width * 2), 
            float(m_dataMap.cell_height * 2)
        );
        hr = UIManager_RenderTarget->CreateCompatibleRenderTarget(
            &size,
            nullptr,
            &format,
            D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE,
            &pBitmapRenderTarget
        );
    }
    // 创建地图单元分界线位图
    if (SUCCEEDED(hr)) {
        // 创建
        hr = UIManager_RenderTarget->CreateBitmap(
            D2D1::SizeU(m_dataMap.cell_width, m_dataMap.cell_height),
            nullptr, 0,
            D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_NONE,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
            ),
            &pCellBitmapBoundary
        );
    }
    // 写入分界线颜色
    if (SUCCEEDED(hr)) {
        const float w = float(m_dataMap.cell_width);
        const float h = float(m_dataMap.cell_height);
        const D2D1_RECT_F rect = { w * 0.f + 1.f, h * 0.f + 1.f, w * 1.f - 1.f, h * 1.f - 1.f };
        pBitmapRenderTarget->BeginDraw();
        pBitmapRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
        pBitmapRenderTarget->PushAxisAlignedClip(&rect, D2D1_ANTIALIAS_MODE_ALIASED);
        pBitmapRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.f));
        pBitmapRenderTarget->PopAxisAlignedClip();
        hr = pBitmapRenderTarget->EndDraw();
    }
    // 复制数据
    if (SUCCEEDED(hr)) {
        hr = pCellBitmapBoundary->CopyFromRenderTarget(nullptr, pBitmapRenderTarget, nullptr);
    }
    // 创建地图单元分界线笔刷
    if (SUCCEEDED(hr)) {
        D2D1_BITMAP_BRUSH_PROPERTIES bbp;
        bbp.extendModeX = bbp.extendModeY = D2D1_EXTEND_MODE_WRAP;
        bbp.interpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
        hr = UIManager_RenderTarget->CreateBitmapBrush(
            pCellBitmapBoundary,
            &bbp, nullptr,
            &m_pCellBoundaryBrush
        );
    }
    // 设置角色
    if (SUCCEEDED(hr)) {
        m_pMapSkin = UIManager.GetBitmap(m_uMapBitmap);
        assert(m_pMapSkin && "bad action");
        if (!m_pMapSkin) hr = E_NOT_SET;
    }
#if 1
    // 设置图标位图
    if (SUCCEEDED(hr)) {
        m_pMapIcon = UIManager.GetBitmap(m_uMapIcon);
        assert(m_pMapSkin && "bad action");
        if (!m_pMapSkin) hr = E_NOT_SET;
    }
#else
    // 写入颜色
    if (SUCCEEDED(hr)) {
        // 使用数据集
        const D2D1_COLOR_F colors[] = {
            D2D1::ColorF(D2D1::ColorF::Black),
            D2D1::ColorF(D2D1::ColorF::White),
            D2D1::ColorF(D2D1::ColorF::Blue),
            D2D1::ColorF(D2D1::ColorF::Red),
        };
        const float w = float(m_dataMap.cell_width);
        const float h = float(m_dataMap.cell_height);
        const D2D1_RECT_F rects[] = {
            { w * 0.f, h * 0.f, w * 1.f, h * 1.f },
            { w * 1.f, h * 0.f, w * 2.f, h * 1.f },
            { w * 0.f, h * 1.f, w * 1.f, h * 2.f },
            { w * 1.f, h * 1.f, w * 2.f, h * 2.f },
        };
        static_assert(
            sizeof(colors) / sizeof(colors[0]) == sizeof(rects) / sizeof(rects[0]),
            "bad size of them"
            );
        // 渲染颜色
        pBitmapRenderTarget->BeginDraw();
        constexpr int size = sizeof(colors) / sizeof(colors[0]);
        for (int i = 0; i != size; ++i) {
            pBitmapRenderTarget->PushAxisAlignedClip(rects + i, D2D1_ANTIALIAS_MODE_ALIASED);
            pBitmapRenderTarget->Clear(colors + i);
            pBitmapRenderTarget->PopAxisAlignedClip();
        }
        pBitmapRenderTarget->DrawBitmap(m_pMapSkin);
        hr = pBitmapRenderTarget->EndDraw();
    }
    // 创建皮肤位图
    if (SUCCEEDED(hr)) {
        ID2D1Bitmap* bitmap = nullptr;
        hr = pBitmapRenderTarget->GetBitmap(&bitmap);
        // 尝试获取位图
        if (SUCCEEDED(hr)) {
            hr = bitmap->QueryInterface(IID_ID2D1Bitmap1, reinterpret_cast<void**>(&m_pMapIcon));
        }
        // 创建皮肤位图
        LongUI::SafeRelease(bitmap);
    }
#endif
    // 设置笔刷透明度
    if (SUCCEEDED(hr)) {
        m_pCellBoundaryBrush->SetOpacity(0.25f);
    }
    // 创建精灵集1
    if (SUCCEEDED(hr)) {
        hr = UIManager_RenderTarget->CreateSpriteBatch(&m_pMapSpriteBatch);
    }
    // 创建精灵集2
    if (SUCCEEDED(hr)) {
        hr = UIManager_RenderTarget->CreateSpriteBatch(&m_pPathDisplay);
    }
    // 创建自动瓦片位图缓存
    if (SUCCEEDED(hr)) {
        hr = UIManager_RenderTarget->CreateBitmap(
            D2D1::SizeU(m_dataMap.cell_width * 8, m_dataMap.cell_height * 8),
            nullptr, 0,
            D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_NONE,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
            ),
            &m_pAutoTileCache
        );
    }
    // 复制数据
    if (SUCCEEDED(hr)) {
        auto halfw = m_dataMap.cell_width / 2;
        auto halfh = m_dataMap.cell_height / 2;
        uint32_t offx = 320;
        uint32_t offy = 320;
        D2D1_RECT_U src = {
            offx, offy,
            offx + m_dataMap.cell_width * 2,
            offy + m_dataMap.cell_height * 3,
        };
        D2D1_POINT_2U des = { 0, 0};
        /*constexpr int END = 256;
        constexpr int WIDTH = 16;
        enum { NONE, YONLY, XONLY, XANDY, XYZ };
        const DT OFFSET[] = {
            { m_dataMap.cell_width, m_dataMap.cell_height * 2 },
            { m_dataMap.cell_width, m_dataMap.cell_height },
            { 0, m_dataMap.cell_height * 2  },
            { m_dataMap.cell_width, 0 },
            { 0, m_dataMap.cell_height },
        };
        for (int i = 0; i < END; ++i) {
            int x = i & (WIDTH - 1);
            int y = i / WIDTH;

        }*/
        m_pAutoTileCache->CopyFromBitmap(&des, m_pMapSkin, &src);
    }
    // 设置角色
    if (SUCCEEDED(hr)) {
        auto bitmap = UIManager.GetBitmap(m_uCharBitmap);
        assert(bitmap && "bad action");
        if (bitmap) {
            m_char.ResetChar(bitmap, this->get_char_data());
            m_char.ResetRenderTarget(UIManager_RenderTarget);
            LongUI::SafeRelease(bitmap);
        }
        else {
            hr = E_NOT_SET;
        }
    }
    // 父类重建
    if (SUCCEEDED(hr)) {
        hr = Super::Recreate();
    }
    LongUI::SafeRelease(pCellBitmapBoundary);
    LongUI::SafeRelease(pBitmapRenderTarget);
    return hr;
}


/// <summary>
/// 一般时间处理
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
bool PathFD::UIMapControl::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // 父类处理
    return Super::DoEvent(arg);
}



// 鼠标事件
bool PathFD::UIMapControl::DoMouseEvent(const LongUI::MouseEventArgument& arg) noexcept {
    // 不支持禁用状态
#if 0
    // 禁用状态禁用鼠标消息
    if (!this->GetEnabled()) return true;
#endif
    // 坐标转换
    D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(
        this->world, D2D1::Point2F(arg.ptx,  arg.pty)
    );
    // ------------------------------- 鼠标左键
    auto on_lbutton_down = [pt4self, this]() noexcept {
        // 双击
        if (m_hlpDbClick.Click(long(pt4self.x), long(pt4self.y))) {
            const bool a = m_uClickX < m_dataMap.map_width;
            const bool b = m_uClickY < m_dataMap.map_height;
            if (a && b) {
                auto index = m_uClickY * m_dataMap.map_width + m_uClickX;
                auto& cell = m_pMapCells[index];
                cell = !cell;
                // TODO: 针对性优化
                this->reset_sprites();
                this->InvalidateThis();
            }
        }
        // 单击
        else {
            auto x = uint32_t(pt4self.x) / m_dataMap.cell_width;
            auto y = uint32_t(pt4self.y) / m_dataMap.cell_height;
            if (m_uClickX != x || m_uClickY != y) {
                m_uClickX = x; m_uClickY = y;
                this->InvalidateThis();
            }
        }
    };
    // ------------------------------- 事件处理
    switch (arg.event)
    {
    case LongUI::MouseEvent::Event_MouseWheelV:
        // 按住Ctrl键缩放
        if (UIInput.IsKbPressed(UIInput.KB_CONTROL)) {
            float z = arg.wheel.delta * 0.5f + 1.f;
            auto zx = this->parent->GetZoomX();
            this->ZoomMapTo(zx * z, 0.5f);
            return true;
        }
        break;
    case LongUI::MouseEvent::Event_MouseWheelH:
        break;
    case LongUI::MouseEvent::Event_DragEnter:
        break;
    case LongUI::MouseEvent::Event_DragOver:
        break;
    case LongUI::MouseEvent::Event_DragLeave:
        break;
    case LongUI::MouseEvent::Event_Drop:
        break;
    case LongUI::MouseEvent::Event_MouseEnter:
        break;
    case LongUI::MouseEvent::Event_MouseLeave:
        break;
    case LongUI::MouseEvent::Event_MouseHover:
        break;
    case LongUI::MouseEvent::Event_MouseMove:
        break;
    case LongUI::MouseEvent::Event_LButtonDown:
        on_lbutton_down();
        break;
    case LongUI::MouseEvent::Event_LButtonUp:
        break;
    case LongUI::MouseEvent::Event_RButtonDown:
        break;
    case LongUI::MouseEvent::Event_RButtonUp:
        break;
    case LongUI::MouseEvent::Event_MButtonDown:
        break;
    case LongUI::MouseEvent::Event_MButtonUp:
        break;
    default:
        break;
    }
    return false;
}

// 缩放空间大小
void PathFD::UIMapControl::ZoomMapTo(float zoom, float time) noexcept {
    auto con = this->parent;
    auto zs = con->GetZoomX();
    auto len = zoom - zs;
    // 添加控制缩放的时间胶囊
    UIManager.AddTimeCapsule([con, zs, len](float x) noexcept {
        x = LongUI::EasingFunction(LongUI::AnimationType::Type_QuarticEaseOut, x);
        auto zoomx = zs + len * x;
        con->SetZoom(zoomx, zoomx);
        // 不要终止时间胶囊刷新
        return false;
    }, this, time);
}

/// <summary>
/// 执行寻路
/// </summary>
/// <param name="algorithm">The algorithm.</param>
/// <param name="info">The info.</param>
/// <returns></returns>
void PathFD::UIMapControl::Execute(IFDAlgorithm* algorithm, LongUI::CUIString& info) noexcept {
    assert(algorithm && "bad argument");
    PathFD::Finder finder;
    finder.data = m_pMapCells;
    finder.width = m_dataMap.map_width;
    finder.height = m_dataMap.map_height;
    finder.startx = int16_t(m_dataMap.char_x);
    finder.starty = int16_t(m_dataMap.char_y);
    finder.goalx = int16_t(m_uGoalX);
    finder.goaly = int16_t(m_uGoalY);
    LongUI::CUITimeMeterH meter;
    meter.Start();
    auto* path = algorithm->Execute(finder);
    auto time = meter.Delta_ms<double>();
    info.Format(L"%ls: %.3f 毫秒", path ? L"成功" : L"失败", time);
    // 旧的路径有效
    if (m_pPath) {
        //assert(!"UNFINISHED");
        std::free(m_pPath);
    }
    // 新的路径有效
    if ((m_pPath = path)) {
        uint32_t count = path->len;
        uint32_t index = 0;
        assert(m_pMapSpriteBatch);
        auto mapw = m_dataMap.map_width;
        // 添加控制路径显示的时间胶囊
        UIManager.AddTimeCapsule([this, path, mapw, index](float x) mutable noexcept {
            constexpr auto at = LongUI::AnimationType::Type_QuarticEaseOut;
            x = LongUI::EasingFunction(at, x);
            if (x != 1.f) {
                // 计算
                uint32_t end = uint32_t(x * float(path->len));
                D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::Orange);
                bool invalidate = false;
                // 遍历方块
                for (auto i = index; i <= end; ++i) {
                    auto realindex = uint32_t(path->pt[i].x) + uint32_t(path->pt[i].y) * mapw;
                    m_pMapSpriteBatch->SetSprites(
                        realindex, 1,
                        nullptr,
                        nullptr,
                        &color,
                        nullptr,
                        0, 0, 0, 0
                    );
                    invalidate = true;
                }
                // 推进索引
                index = end + 1;
                // 需要刷新
                if (invalidate) {
                    this->InvalidateThis();
                }
            }
            // 不要中断调用
            return false;
        }, &m_pMapSpriteBatch, 1.f + float(count) / 300.f);
    }
}

/// <summary>
/// U刷新控件
/// </summary>
/// <returns></returns>
void PathFD::UIMapControl::Update() noexcept {
    // 父类刷新
    Super::Update();
    // 地图有效
    if (!m_dataMap.map_width) return;
    // 接受输入
    m_char.Input(PathFD::InputCheck());
    // 刷新角色
    if (m_char.Update()) {
        this->InvalidateThis();
    }
}

/// <summary>
/// 创建本控件
/// </summary>
/// <param name="type">The type.</param>
/// <param name="node">The node.</param>
/// <returns></returns>
auto PathFD::UIMapControl::CreateControl(
    LongUI::CreateEventType type, pugi::xml_node node) noexcept -> UIControl * {
    UIMapControl* pControl = nullptr;
    switch (type)
    {
    case LongUI::Type_Initialize:
        break;
    case LongUI::Type_Recreate:
        break;
    case LongUI::Type_Uninitialize:
        break;
    case_LongUI__Type_CreateControl:
        LongUI__CreateWidthCET(UIMapControl, pControl, type, node);
    }
    return pControl;
}



#ifdef LongUIDebugEvent
// UI文本: 调试信息
bool PathFD::UIMapControl::debug_do_event(const LongUI::DebugEventInformation& info) const noexcept {
    switch (info.infomation)
    {
    case LongUI::DebugInformation::Information_GetClassName:
        info.str = L"UIMapControl";
        return true;
    case LongUI::DebugInformation::Information_GetFullClassName:
        info.str = L"::PathFD::UIMapControl";
        return true;
    case LongUI::DebugInformation::Information_CanbeCasted:
        // 类型转换
        return *info.iid == LongUI::GetIID<::PathFD::UIMapControl>()
            || Super::debug_do_event(info);
    default:
        break;
    }
    return false;
}
#endif

// pathfd 路径
namespace PathFD {
    /// <summary>
    /// 创建地图控件
    /// </summary>
    /// <returns></returns>
    auto CreateMapControl(LongUI::CreateEventType cet, pugi::xml_node node)
        noexcept -> LongUI::UIControl * {
        return UIMapControl::CreateControl(cet, node);
    }
}