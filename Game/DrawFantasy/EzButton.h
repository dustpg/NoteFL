#pragma once


// 按钮状态
enum ButtonStatus : uint32_t {
    Status_Disabled = 0,    // 禁用状态
    Status_Normal,          // 通常状态
    Status_Hover,           // 鼠标盘旋
    Status_Pushed,          // 点击状态
    Status_Count,           // 状态数量
};


// 简易按钮
class EzButton {
public:
    // 构造函数
    EzButton() noexcept;
    // 构造函数
    ~EzButton() noexcept { ::SafeRelease(m_pBitmap); }
    // 鼠标左键点击时
    void OnMouseLUp(float x, float y) noexcept;
    // 鼠标左键按下时
    void OnMouseLDown(float x, float y) noexcept;
    // 鼠标移动时候
    void OnMouseMove(float x, float y) noexcept;
    // 初始化状态
    auto Init(ButtonStatus s)noexcept { m_oldStatus = m_tarStatus = s; }
    // 刷新
    void Render(ID2D1DeviceContext*, float delta) noexcept;
    // 设置点击回调
    auto SetOnClickCall(void* g, GameCallBack call)noexcept { m_clickData = g, m_onClick = call; }
    // 设置新的位图
    auto SetBitmap(ID2D1Bitmap1* bmp) noexcept { ::SafeRelease(m_pBitmap); m_pBitmap = ::SafeAcquire(bmp); }
public:
    // 时间位置
    float                   time = 0.0f;
    // 动画时间
    float                   duration = 0.20f;
    // 基本Alpha
    float                   alpha_base = 1.f;
private:
    // 当前值
    float                   m_fValue = 1.f;
    // 上一帧X坐标
    float                   m_x = -1.f;
    // 上一帧Y坐标
    float                   m_y = -1.f;
    // 回调数据
    void*                   m_clickData = nullptr;
    // 回调
    GameCallBack            m_onClick = nullptr;
    // 位图
    ID2D1Bitmap1*           m_pBitmap = nullptr;
    // 当前状态
    ButtonStatus            m_oldStatus = Status_Normal;
    // 目标
    ButtonStatus            m_tarStatus = Status_Normal;
    // 值
public:
    // 目标位置
    D2D1_RECT_F             des_rect;
    // 源位置
    D2D1_RECT_F             src_rects[Status_Count];
};

// 游戏按钮
class GameButton {
public:
    // 默认构造函数
    GameButton(Sprite* s = nullptr) noexcept;
    // 复制构造函数
    GameButton(const GameButton&) =delete;
    // 移动构造函数
    GameButton(GameButton&&) noexcept;
    // 析构函数
    ~GameButton()  { ::SafeRelease(m_pSprite); }
    // ->
    auto operator ->() noexcept { return m_pSprite; }
    // 设置
    void SetSprite(Sprite* s) noexcept { ::SafeRelease(m_pSprite); m_pSprite = s; }
    // 刷新
    bool Update() noexcept;
    // 有效
    operator bool()const noexcept { return m_pSprite != nullptr; }
private:
    // 精灵
    Sprite*                 m_pSprite = nullptr;
public:
    // 源位置
    D2D1_RECT_F             src_rects[Status_Count];
private:
    // 点击
    bool                    m_bClickIn = false;
    // 保留
    bool                    m_bUnsed[sizeof(void*) - 1];
};

// 游戏按钮EX
class GameButtonEx : public GameButton {
public:
    // 默认构造函数
    GameButtonEx(Sprite* s = nullptr) noexcept : GameButton(s) {
        start.Init(); end.Init();
    };
    // 初始状态
    SpriteStatus            start;
    // 末尾状态
    SpriteStatus            end;
};