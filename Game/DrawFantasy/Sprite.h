#pragma once



// 精灵状态
struct SpriteStatus {
    // 初始化
    void Init() noexcept;
    // X 坐标
    float               x;
    // Y 坐标
    float               y;
    // 原点X 坐标
    float               ox;
    // 原点Y 坐标
    float               oy;
    // X 轴放大率
    float               zoom_x;
    // Y 轴放大率
    float               zoom_y;
    // 不透明度
    float               opacity;
    // 旋转度数
    float               rotation;
};


// 游戏精灵
class Sprite : public SpriteStatus {
public:
    // 删除器
    static void Deleter(Sprite* sprite)noexcept { assert(sprite && "bad"); sprite->Release(); }
    // 渲染器
    static ImageRenderer* s_pImageRenderer;
    // 创建新的精灵
    static Sprite* New() noexcept;
    // 刷新帧
    static auto UpdateFrame(UINT syn) throw(GameExitExpt&) ->float;
    // 刷新帧
    static auto UpdateFrameNoExcept(UINT syn) noexcept ->float;
public:
    // 插值
    using InterpolationMode = D2D1_INTERPOLATION_MODE;
    // 构造函数
    Sprite() noexcept;
    // 移动构造函数
    Sprite(Sprite&&);
    // 构造函数
    Sprite(const Sprite&) = delete;
    // 析构函数
    ~Sprite() noexcept { ::SafeRelease(m_pBitmap); }
    // 释放
    void Release() noexcept;
    // 新的位图
    void SetNewBitmap(ID2D1Bitmap1* bitmap) noexcept;
    // 载入新的位图
    void LoadNewBitmap(const wchar_t*) noexcept;
    // 新的回调
    auto SetNewMoveEnd(GameCallBack call, void* data) noexcept { m_pMoveEndData = data; m_pMoveEnd = call;};
    // 刷新
    void Updata(float delta) noexcept;
    // 获取目标
    auto GetTarget() ->SpriteStatus& { return m_tarStatus; }
    // 获取位图宽度
    auto GetBitmapWidth() const noexcept { return m_fWidth; }
    // 获取位图高度
    auto GetBitmapHeight() const noexcept { return m_fHeight; }
    // 获取实际宽度
    auto GetWidth() const noexcept { return src_rect.right - src_rect.left; }
    // 获取实际高度
    auto GetHeight() const noexcept { return src_rect.bottom - src_rect.top; }
    // 改变
    auto ChangeTo(const SpriteStatus& target, float time) noexcept { return this->change_to(target, time); }
    // 渲染
    void Render(ID2D1DeviceContext* dc, const D2D1_MATRIX_3X2_F& old) const noexcept {
        if (!m_pBitmap) return;
        D2D1_MATRIX_3X2_F matrix = m_matrix * old;
        dc->SetTransform(&matrix);
        dc->DrawBitmap(
            m_pBitmap, nullptr, this->opacity,
            this->imode, &this->src_rect
            );
    }
    // 移动到
    auto MoveTo(float _x, float _y, float time) noexcept {
        SpriteStatus target;
        target.x = _x; target.y = _y;
        target.ox = this->ox;
        target.oy = this->oy;
        target.zoom_x = this->zoom_x;
        target.zoom_y = this->zoom_y;
        target.opacity = this->opacity;
        target.rotation = this->rotation;
        this->ChangeTo(target, time);
    }
    // 转换坐标
    auto TransformPoint(D2D1_POINT_2F & pt) noexcept {
#if 1
        // x = (bn-dm)/(bc-ad)
        // y = (an-cm)/(ad-bc)
        // a : m_matrix._11
        // b : m_matrix._21
        // c : m_matrix._12
        // d : m_matrix._22
        register auto bc_ad = m_matrix._21 * m_matrix._12 
            - m_matrix._11 * m_matrix._22;
        register auto m = pt.x - m_matrix._31;
        register auto n = pt.y - m_matrix._32;
        pt.x = (m_matrix._21*n - m_matrix._22 * m) / bc_ad;
        pt.y = (m_matrix._12*m - m_matrix._11 * n) / bc_ad;
#else
        register auto tx = m_matrix._11 * pt.x + m_matrix._21 * pt.y + m_matrix._31;
        register auto ty = m_matrix._12 * pt.x + m_matrix._22 * pt.y + m_matrix._32;
        pt.x = tx;  pt.y = ty;
#endif
    }
private:
    // 改变
    void change_to(const SpriteStatus& target, float time) noexcept;
    // 创建转换矩阵
    void make_matrix() noexcept;
public:
    // 动画时间
    float               duration = 0.2f;
    // Z坐标
    int32_t             z = 0;
    // 插值方法
    InterpolationMode   imode = D2D1_INTERPOLATION_MODE_LINEAR;
    // 动画类型
    AnimationType       atype = AnimationType::Type_LinearInterpolation;
    // 源矩形
    D2D1_RECT_F         src_rect = D2D1::RectF();
    // 用户数据
    void*               user_data = nullptr;
private:
    // 转换矩阵
    D2D1_MATRIX_3X2_F   m_matrix;
    // 旧Z坐标
    int32_t             m_iOldZ = 0;
    // 所花时间
    float               m_fTime = 0.0f;
    // 宽度
    float               m_fWidth = 0.f;
    // 宽度
    float               m_fHeight = 0.f;
    // 位图
    ID2D1Bitmap1*       m_pBitmap = nullptr;
    // 移动回调
    GameCallBack        m_pMoveEnd = nullptr;
    // 移动回调数据
    void*               m_pMoveEndData = nullptr;
    // 旧状态
    SpriteStatus        m_oldStatus;
    // 目标状态
    SpriteStatus        m_tarStatus;
};