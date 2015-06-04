#pragma once


// 场景阶段
enum class SceneStage : uint32_t {
    Stage_In = 0,   // 入
    Stage_Main,     // 主
    Stage_Out,      // 出
    Stage_Unknown,  // NO
};


// 游戏场景
class DECLSPEC_NOVTABLE BaseScene {
public:
    // 构造函数
    BaseScene(ThisApp& a) noexcept : m_app(a) {  };
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
class DECLSPEC_NOVTABLE BaseScene2 : public BaseScene {
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
    virtual inline void update() GameExitThrow { Sprite::UpdateFrame(1); };
    // 结束前
    virtual inline void pre_terminate() noexcept {};
    // 结束
    virtual inline void terminate() noexcept {};
private:
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
    // 父类声明
    using Super = BaseScene2;
public:
    // 构造函数
    TitleScene(ThisApp& g) noexcept : Super(g) {};
    // 析构函数
    ~TitleScene() override;
protected:
    // 开始
    void start() noexcept ;
    // 刷新
    void update() GameExitThrow;
protected:
    // 背景图片
    Sprite*             m_pBackground = nullptr;
    // 游戏按钮
    GameButton          m_buttons[BUTTON_SIZE];
};
