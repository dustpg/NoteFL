#pragma once


// 场景阶段
enum class SceneStage : uint32_t {
    Stage_In = 0,   // 入
    Stage_Main,     // 主
    Stage_Out,      // 出
    Stage_Unknown,  // NO
};


// 游戏场景
class BaseScene {
public:
    // 构造函数
    BaseScene(ThisApp& a) noexcept;
public:
    // 运行
    virtual void Run() GameExitThrow = 0;
    // 设备重置
    virtual void DeviceReset() noexcept {};
    // 点击Close直接退出?
    virtual bool IsExitDirectly() noexcept { return true; };
    // 析构函数
    virtual ~BaseScene() noexcept = default;
protected:
    // 游戏
    ThisApp&                m_app;
    // MRuby 状态(虚拟机)
    mrb_state*              m_pMRuby;
};


// 开始场景
class HelloScene final : public BaseScene {
    // 父类声明
    using Super = BaseScene;
public:
    // 构造函数
    HelloScene(ThisApp& g) noexcept : Super(g) {};
public: // 实现BaseScene接口
    // 运行
    virtual void Run() GameExitThrow override;
    // 析构函数
    ~HelloScene() override = default;
protected:
};

// 基本场景 lv2
class BaseScene2 : public BaseScene {
    // 父类声明
    using Super = BaseScene;
public:
    // 构造函数
    BaseScene2(ThisApp& g) noexcept : Super(g) {};
    // 运行
    void Run()  GameExitThrow override final;
protected:
    // 开始
    virtual inline void start() noexcept {};
    // 开始后
    virtual inline void post_start() noexcept {};
    // 刷新
    virtual inline void update() GameExitThrow { m_fDeltaTime = Sprite::UpdateFrame(1); };
    // 结束前
    virtual inline void pre_terminate() noexcept {};
    // 结束
    virtual inline void terminate() noexcept {};
protected:
    // 间隔时间
    float               m_fDeltaTime = 0.f;
};


// 初始化动画时间
static constexpr float TITLE_ANIMATION_TIME = 0.5f;
// 标题场景
class TitleScene final : public BaseScene2 {
    // 标题按钮
    enum TitleButton {
        Button_Ply,     // 开始游戏
        Button_New,     // x
        Button_Con,     // 
        Button_Opt,     // 
        BUTTON_SIZE,    // 总数量
    };
    // 选项按钮
    enum OpintionButton {
        Button_BGMA,
        Button_BGMB,
        Button_BGMC,
        Button_SEA,
        Button_SEB,
        Button_SEC,
        Button_WndA,
        Button_WndB,
        Button_WndC,
        OPINTION_SIZE,    // 总数量
    };
    // 父类声明
    using Super = BaseScene2;
    // 片段
    using Clip = WrapAL::AudioSourceClip;
public:
    // 构造函数
    TitleScene(ThisApp& g) noexcept : Super(g) {};
    // 析构函数
    ~TitleScene() noexcept;
protected:
    // 开始
    void start() noexcept override;
    // 刷新
    void update() GameExitThrow override;
    // 结束前
    void pre_terminate() noexcept override;
protected:
    // 背景图片
    Sprite*             m_pBackground = nullptr;
    // 背景: BGM
    Sprite*             m_pBackBGM = nullptr;
    // 背景: SE
    Sprite*             m_pBackSE = nullptr;
    // 背景: 窗口缩放
    Sprite*             m_pBackWndZoom = nullptr;
    // 音乐
    Clip                m_clip = 0;
    // 游戏按钮
    GameButtonEx        m_buttons[BUTTON_SIZE];
    // 选项区
    GameButtonEx        m_opintion[OPINTION_SIZE];
};


// 游戏场景
class GameScene final : public BaseScene2 {
    // 片段
    using Clip = WrapAL::AudioSourceClip;
    // 父类声明
    using Super = BaseScene2;
    // 精灵
    enum : uint32_t {
        // 背景图
        Sprite_Background = 0,
        // 技能槽
        Sprite_SkillSlot,
        // 技能点
        Sprite_SkillPoint,
        // 选择技能
        Sprite_Skill,
        // HP值
        Sprite_Health,
        // 菜单A
        Sprite_MenuA,
        // 菜单B
        Sprite_MenuB,
        // 大小
        SPRITE_SIZE,
    };
    // 技能编号
    enum Skill : uint32_t {
        Skill_FireBall = 0,
        Skill_Heal,
        Skill_AoE,
        Skill_God,
        SKILL_SIZE
    };
public:
    // 构造函数
    GameScene(ThisApp& g) noexcept;
    // 析构函数
    ~GameScene() noexcept;
protected:
    // 开始
    void start() noexcept override;
    // 刷新
    void update() GameExitThrow override;
private:
    // 检查玩家
    void updata_player() noexcept;
    // 检查玩家
    void updata_menu() noexcept;
    // 检查玩家
    void updata_skill() noexcept;
    // 设置技能动画
    void set_skill_animation()noexcept;
private:
    // 背景图片
    Sprite*             m_apSprites[SPRITE_SIZE];
    // 音乐
    Clip                m_clip = 0;
    // 技能动画
    AutoAnimation       m_skillAnimation;
    // 玩家
    GameActor           m_player;
    // 数据
    ActorData           m_playerData;
    // 地形
    GameTerrain         m_terrain;
    // 游戏敌人
    GameEnemy           m_enemy;
    // 技能数据
    union {
        // 火球
        struct  {
            // 方向
            float       fire_direction;
            // 速度X
            float       fire_x;
            // 速度X
            float       fire_y;
        };
        // 无敌
        struct {
            // 剩余时间
            float       god_time;
        };
    }                   data;
};