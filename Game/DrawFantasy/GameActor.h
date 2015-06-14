#pragma once

// 游戏地形
class GameTerrain {
public:
    // 线
    struct Line {
        // 线头
        D2D1_POINT_2F       begin;
        // 线尾
        D2D1_POINT_2F       end;
    };
    // 设置
    auto Set(size_t size, Line* lines) noexcept { m_cLineSize = size; m_pTerrain = lines; }
    // 碰撞检测
    auto HitTest(const D2D1_RECT_F& rect) const noexcept ->uint32_t;
    // f(x)
    auto Fx(size_t no, float x) const noexcept->float;
private:
    // 数量
    size_t          m_cLineSize = 0;
    // 地形
    Line*           m_pTerrain = nullptr;
};

// 动作信息
struct AnimationInfo {
    // 开始索引
    uint32_t    start;
    // 结束索引
    uint32_t    end;
    // 是否循环
    uint32_t    loop;
    // 间隔时间
    float       time;
};



// 游戏按钮
class GameActor {
public:
    // 速度增加值
    static constexpr float SPPED_UP = 1.f;
    // 速度衰减值
    static constexpr float SPPED_ATTENUATION = 2.f;
    // 跑动阈值
    static constexpr float RUN_THRESHOLD = 0.5f;
    // 最高速度
    static constexpr float RUN_BASIC_LENGTH = 150.f;
    // 动作类型
    enum ActionType : uint8_t {
        Type_Standby,       // 待机
        Type_Walk,          // 行走
        Type_Run,           // 跑动
        Type_Attack,        // 攻击
        Type_Block,         // 格挡
        Type_Die,           // 死亡
        Type_Spell,         // 咒语
        Type_Jump,          // 跳跃
        //---
        Type_UNK
    };
public:
    // 构造函数
    GameActor(const GameTerrain& terrain) noexcept;
    // 移动构造函数
    GameActor(GameActor&&)noexcept;
    // 复制构造函数
    GameActor(const GameActor&) =delete;
    // 析构函数
    ~GameActor() { ::SafeRelease(m_pSprite); }
    // ->
    auto operator ->() noexcept { assert(m_pSprite); return m_pSprite; }
    // 新的动作
    auto SetAction(ActionType type) noexcept { m_bActed = true; this->set_action(type); }
    // 刷新
    void Update(float delta) noexcept;
    // 设置基本位置
    auto SetPos(float x, float y) noexcept { m_x = x; m_y = y; }
    // 设置面向
    auto SetFaceLeft() noexcept { m_pSprite->zoom_x = -1.f; }
    // 获取面向
    auto GetFaced() const noexcept { return m_faceRight; }
    // 获取速度
    auto GetSpeedX() const noexcept { return m_fSpeedX; }
    // 获取速度
    auto GetSpeedY() const noexcept { return m_fSpeedY; }
private:
    // 新的动作
    void set_action(ActionType type) noexcept;
public:
    // 待机
    auto Standby() noexcept { return this->SetAction(GameActor::Type_Standby); }
    // 往右跑
    void RunRight() noexcept;
    // 往左跑
    void RunLeft() noexcept;
    // 跳跃
    void Jump() noexcept;
    // 攻击
    void Attack() noexcept;
    // 死亡
    auto IsDead() noexcept { return m_bDead; }
    // 死亡
    void Die() noexcept;
private:
    // 精灵
    Sprite*                 m_pSprite = Sprite::New();
    // 地形
    const GameTerrain&      m_terrain;
    // 当前动作
    ActionType              m_nowAction = Type_UNK;
    // 输入动作
    bool                    m_bActed = false;
    // 死亡
    bool                    m_bDead = false;
    // 空中
    bool                    m_bUnlanded = false;
    // 当前信息
    AnimationInfo           m_info;
    // 当前索引
    uint32_t                m_idNowIndex = 0;
    // 当前X速度
    float                   m_fSpeedX = 0.f;
    // 当前Y速度
    float                   m_fSpeedY = 0.f;
    // 最高速度
    float                   m_fMAxSpeed = 1.f;
    // 面向右边
    float                   m_faceRight = 1.f;
    // 时间
    float                   m_fDeltaTime = 0.f;
    // 动作时间消耗
    float                   m_fActionTimeRemain = 0.f;
    // X坐标
    float                   m_x = 0.f;
    // Y坐标
    float                   m_y = 0.f;
};


// 数据
struct ActorData {
    // 选择技能
    uint32_t        skill_id;
    // 旧的选择技能
    uint32_t        old_id;
    // 血量
    uint32_t        hp;
    // 技能点
    uint32_t        sp;
};



// 自动动画
class AutoAnimation : public AnimationInfo {
public:
    // 构造函数
    AutoAnimation() noexcept;
    // 析构函数
    ~AutoAnimation() noexcept { ::SafeRelease(m_pSprite); }
    // 播放
    void Play() noexcept;
    // 播放结束?
    auto IsEnd() const noexcept { return m_played > 0u; }
    // 刷新
    void Update(float delta) noexcept;
    // ->
    auto operator ->() noexcept { assert(m_pSprite); return m_pSprite; }
private:
    // 精灵
    Sprite*                 m_pSprite = Sprite::New();
    // 当前索引
    uint32_t                m_idNowIndex = 0;
    // 播放
    uint32_t                m_played = 0;
    // 剩余时间
    float                   m_fTimeRemain = 0.f;
public:
    // 技能编号
    uint32_t                id = 0;
    // 数量
    uint32_t                size = 1;
    // 单位
    float                   unit = 1.f;
};


// 敌人
class GameEnemy {
    // 敌人数据
    using EnemyList = std::list<GameActor>;
public:
    // 构造函数
    GameEnemy(const GameTerrain&, GameActor&) noexcept;
    // 添加敌人
    auto AddEnemy() noexcept ->GameActor*;
    // 刷新
    void Update(float delta) noexcept;
    // 遍历
    template<typename Lam>
    auto for_each(Lam lam) { for (auto& ememy : m_list) {if (lam(ememy)) break; } }
private:
    // 地形
    const GameTerrain&  m_terrain;
    // 主角
    GameActor&          m_player;
    // 敌人列表
    EnemyList           m_list;
};