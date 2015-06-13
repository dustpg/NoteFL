#include "stdafx.h"
#include "included.h"


// HelloScene: 运行
void HelloScene::Run() GameExitThrow {
#ifdef _DEBUG
    // 载入场景
    m_app.LoadScene<TitleScene>();
    return;
#endif
    float time = 2.0f;
    auto sprite(Sprite::New());
    if (!sprite) return;
    sprite->LoadNewBitmap(L"resource/logo.png");
    //
    SpriteStatus status1, status2;
    {
        // 设置为中心
        status1.x = sprite->GetWidth() * 0.5f;
        status1.y = sprite->GetHeight() * 0.5f;
        status1.ox = status1.x;
        status1.oy = status1.y;
        status1.zoom_x = 0.0f;
        status1.zoom_y = 0.0f;
        status1.opacity = 0.f;
        status1.rotation = 90.f;
        // 设置为中心
        status2.x = sprite->GetWidth() * 0.5f;
        status2.y = sprite->GetHeight() * 0.5f;
        status2.ox = status2.x;
        status2.oy = status2.y;
        status2.zoom_x = 1.f;
        status2.zoom_y = 1.f;
        status2.opacity = 1.f;
        status2.rotation = 0.f;
    }
    static_cast<SpriteStatus&>(*sprite) = status1;
    status1.rotation = -90.f;
    sprite->atype = AnimationType::Type_CubicEaseIn;
    sprite->ChangeTo(status2, 0.5f);
    bool loop = true;
    // 事件
    auto exit_loop = [](void* obj, void* loop) ->void { *(bool*)(loop) = false; };
    sprite->SetNewMoveEnd(exit_loop, &loop);
    // 渐入
    while (loop) Sprite::UpdateFrame(1);
    // 主要显示
    while (time > 0.f)  time -= Sprite::UpdateFrame(1);
    // 渐出
    sprite->atype = AnimationType::Type_CubicEaseOut;
    loop = true;
    sprite->ChangeTo(status1, 0.5f);
    while (loop) Sprite::UpdateFrame(1);
    // 清理
    ::SafeRelease(sprite);
    // 载入场景
    m_app.LoadScene<TitleScene>();
}


// ----------- BaseScene2 -------------
// BaseScene2: 运行
void BaseScene2::Run() GameExitThrow {
    this->start();
    this->post_start();
    while (m_app.Scene() == this) this->update();
    //while (true) this->update();
    this->pre_terminate();
    this->terminate();
}


// --------- TitleScene --------------
// TitleScene: 析构函数
TitleScene::~TitleScene() noexcept {
    ::SafeRelease(m_pBackground);
    ::SafeRelease(m_pBackBGM);
    ::SafeRelease(m_pBackSE);
    ::SafeRelease(m_pBackWndZoom);
}


constexpr float OPINTION_POS_BASE_X = -10.f;
constexpr float OPINTION_POS_BASE_Y = 400.f;

constexpr float OPINTION_POS_XPLUS = 80.f;
constexpr float OPINTION_POS_YPLUS = 80.f;

constexpr float OPINTION_POS_END_X = 10.f;
constexpr float OPINTION_POS_END_Y = 40.f;
// 常量
static const D2D1_RECT_F OPINTION_POS[] = {
    {  0.f, 256.f,  40.f, 296.f },
    {  0.f, 296.f,  40.f, 336.f },
    {  0.f, 336.f,  40.f, 376.f },
    
    { 40.f, 256.f,  80.f, 296.f },
    { 40.f, 296.f,  80.f, 336.f },
    { 40.f, 336.f,  80.f, 376.f },


    { 80.f, 256.f, 100.f, 296.f },
    { 80.f, 296.f, 100.f, 336.f },
    { 80.f, 336.f, 100.f, 376.f },

    {
        -480.f + OPINTION_POS_BASE_X, OPINTION_POS_BASE_Y,
        OPINTION_POS_END_X, OPINTION_POS_END_Y
    },
    {
        -480.f + OPINTION_POS_BASE_X + 440.f, OPINTION_POS_BASE_Y,
        OPINTION_POS_END_X + 440.f, OPINTION_POS_END_Y
    },
    {
        -480.f + OPINTION_POS_BASE_X + 40.f, OPINTION_POS_BASE_Y,
        OPINTION_POS_END_X + 40.f, OPINTION_POS_END_Y
    },

    {
        -480.f + OPINTION_POS_BASE_X, OPINTION_POS_BASE_Y + OPINTION_POS_YPLUS,
        OPINTION_POS_END_X + OPINTION_POS_XPLUS, OPINTION_POS_END_Y + OPINTION_POS_YPLUS
    },
    {
        -480.f + OPINTION_POS_BASE_X + 440.f, OPINTION_POS_BASE_Y + OPINTION_POS_YPLUS,
        OPINTION_POS_END_X + OPINTION_POS_XPLUS + 440.f, OPINTION_POS_END_Y + OPINTION_POS_YPLUS
    },
    {
        -480.f + OPINTION_POS_BASE_X + 40.f, OPINTION_POS_BASE_Y + OPINTION_POS_YPLUS,
        OPINTION_POS_END_X + OPINTION_POS_XPLUS + 40.f, OPINTION_POS_END_Y + OPINTION_POS_YPLUS
    },

    {
        -480.f + OPINTION_POS_BASE_X, OPINTION_POS_BASE_Y + OPINTION_POS_YPLUS*2.f,
        OPINTION_POS_END_X + OPINTION_POS_XPLUS*2.f, OPINTION_POS_END_Y + OPINTION_POS_YPLUS*2.f
    },
    {
        -480.f + OPINTION_POS_BASE_X + 440.f, OPINTION_POS_BASE_Y + OPINTION_POS_YPLUS*2.f,
        OPINTION_POS_END_X + OPINTION_POS_XPLUS*2.f + 440.f, OPINTION_POS_END_Y + OPINTION_POS_YPLUS*2.f
    },
    {
        -480.f + OPINTION_POS_BASE_X + 40.f, OPINTION_POS_BASE_Y + OPINTION_POS_YPLUS*2.f,
        OPINTION_POS_END_X + OPINTION_POS_XPLUS*2.f + 40.f, OPINTION_POS_END_Y + OPINTION_POS_YPLUS*2.f
    },
};

// TitleScene: 刷新
void TitleScene::start() noexcept {
    m_pBackground = Sprite::New();
    if (m_pBackground) {
        m_pBackground->atype = AnimationType::Type_CubicEaseIn;
        m_pBackground->LoadNewBitmap(L"resource/title.png");
        // 保存目标状态
        SpriteStatus s = *m_pBackground;
        m_pBackground->x = -m_pBackground->GetWidth() * 0.5f;
        m_pBackground->opacity = 0.f;
        m_pBackground->ChangeTo(s, 0.5f);
    }
    // 按钮宽度
    constexpr float BUTTON_WIDTH = 215.f;
    constexpr float BUTTON_HEIGHT = 62.f;
    constexpr float BUTTON_XBASE = 900.f;
    constexpr float BUTTON_XPLUS = 100.f;
    constexpr float BUTTON_YBASE = 1000.f;
    constexpr float BUTTON_YPLUS = 100.f;
    constexpr float BUTTON_XEND = 500.f;
    constexpr float BUTTON_ANIMATION = 0.10f;
    // 图标
    ID2D1Bitmap1* bitmap = nullptr;
    m_app.LoadBitmapFromFile(L"resource/All.png", &bitmap);
    // 滑动槽
    if ((m_pBackBGM = Sprite::New())) {
        m_pBackBGM->SetNewBitmap(bitmap);
        m_pBackBGM->src_rect = { 100.f, 256.f, 500.f, 296.f };
    }
    if ((m_pBackSE = Sprite::New())) {
        m_pBackSE->SetNewBitmap(bitmap);
        m_pBackSE->src_rect = { 100.f, 296.f, 500.f, 336.f };
    }
    if ((m_pBackWndZoom = Sprite::New())) {
        m_pBackWndZoom->SetNewBitmap(bitmap);
        m_pBackWndZoom->src_rect = { 100.f, 336.f, 500.f, 376.f };
    }
    // 坐标
    D2D1_RECT_F rect_x = { 0.f,0.f, BUTTON_WIDTH, BUTTON_HEIGHT };
    // 动画
    {
        float ybase = BUTTON_YPLUS;
        float atime = 0.25f + BUTTON_ANIMATION;
        for (auto& btn : m_buttons) {
            auto rect_y = rect_x;
            btn.SetSprite(Sprite::New());
            if (btn) {
                // 基本
                btn->z = 100;
                btn->atype = AnimationType::Type_BackEaseIn;
                btn->SetNewBitmap(bitmap);
                btn.src_rects[Status_Normal] = rect_y;
                rect_y.top += BUTTON_HEIGHT;
                rect_y.bottom += BUTTON_HEIGHT;
                btn.src_rects[Status_Hover] = rect_y;
                rect_y.top += BUTTON_HEIGHT;
                rect_y.bottom += BUTTON_HEIGHT;
                btn.src_rects[Status_Pushed] = rect_y;
                // 动画
                btn->x = BUTTON_XBASE;
                btn->y = ybase;
                btn->rotation = 90.f;
                btn->MoveTo(BUTTON_XEND, ybase, atime);
                btn->GetTarget().rotation = 15.f;
                // 修改
                btn.start = btn->GetOld();
                btn.end = btn->GetTarget();
            }
            rect_x.left += BUTTON_WIDTH;
            rect_x.right += BUTTON_WIDTH;
            ybase += BUTTON_YPLUS;
            atime += BUTTON_ANIMATION;
        }
    }
    // 选项
    {
        float y_base = 0.f;
        auto now = 0u;
        for (auto& btn : m_opintion) {
            btn.SetSprite(Sprite::New());
            if (btn) {
                // 基本
                btn->z = 100;
                btn->atype = AnimationType::Type_CubicEaseIn;
                btn->SetNewBitmap(bitmap);
                ::memcpy(
                    btn.src_rects + Status_Normal, OPINTION_POS + now*3, 3 * sizeof(D2D1_RECT_F)
                    );
                //
                auto index = &btn - m_opintion;
                btn.start.x = OPINTION_POS[index + 9].left;
                btn.start.y = OPINTION_POS[index + 9].top;
                btn.end.x = OPINTION_POS[index + 9].right;
                btn.end.y = OPINTION_POS[index + 9].bottom;
                btn->Set(btn.start);
                btn->y = y_base;
            }
            now += 1; if (now == 3) now = 0;
            y_base += 60.f;
        }
    }
    ::SafeRelease(bitmap);
    m_app.ShowCaption();
}


// TitleScene: 刷新
void TitleScene::update() GameExitThrow {
    // 开始游戏
    if (m_buttons[Button_Ply].Update()) {

    }
    // xxx
    if (m_buttons[Button_New].Update()) {

    }
    // 开始游戏
    if (m_buttons[Button_Con].Update()) {

    }
    // 游戏选项
    if (m_buttons[Button_Opt].Update()) {
        // 选项菜单有效？
        if (m_opintion[Button_BGMA]->x > 0.f) {
            m_buttons[Button_Ply]->ChangeTo(m_buttons[Button_Ply].end, 0.5f);
            m_buttons[Button_New]->ChangeTo(m_buttons[Button_New].end, 0.5f);
            m_buttons[Button_Con]->ChangeTo(m_buttons[Button_Con].end, 0.5f);
            // 
            for (auto& btn : m_opintion) {
                btn->ChangeTo(btn.start, 0.5f);
            }
        }
        // 选项菜单无效
        else {
            m_buttons[Button_Ply]->ChangeTo(m_buttons[Button_Ply].start, 0.5f);
            m_buttons[Button_New]->ChangeTo(m_buttons[Button_New].start, 0.5f);
            m_buttons[Button_Con]->ChangeTo(m_buttons[Button_Con].start, 0.5f);
            //
            for (auto& btn : m_opintion) {
                if ((&btn - m_opintion) % 3 != 2) {
                    btn->ChangeTo(btn.end, 0.5f);
                }
            }
        }
    }
    // 刷新 位置
    // BGM
    auto bgm = AudioEngine.Master_Volume();
    auto new_bgm = bgm;
    auto se = AudioEngine.Master_Volume();
    auto new_se= se;

    // BGM: A
    if (m_opintion[Button_BGMA].Update()) {
        new_bgm -= 0.1f;
    }
    // BGM: B
    if (m_opintion[Button_BGMB].Update()) {
        new_bgm += 0.1f;
    }
    // BGM: C
    if (m_opintion[Button_BGMC].Update()) {
    }
    // SE: A
    if (m_opintion[Button_SEA].Update()) {
        new_se -= 0.1f;
    }
    // SE: B
    if (m_opintion[Button_SEB].Update()) {
        new_se += 0.1f;
    }
    // SE: C
    if (m_opintion[Button_SEC].Update()) {

    }
    // Wnd: A
    if (m_opintion[Button_WndA].Update()) {

    }
    // Wnd: B
    if (m_opintion[Button_WndB].Update()) {

    }
    // Wnd: C
    if (m_opintion[Button_WndC].Update()) {

    }
    {
        float x = m_opintion[Button_BGMA]->x + 40.f;
        float y = m_opintion[Button_BGMA]->y;
        m_pBackBGM->x = x;
        m_pBackBGM->y = y;
        //
        m_opintion[Button_BGMC]->x = x + bgm * 190.f;
        m_opintion[Button_BGMC]->y = y;
    }
    // SE
    {
        float x = m_opintion[Button_SEA]->x + 40.f;
        float y = m_opintion[Button_SEA]->y;
        m_pBackSE->x = x;
        m_pBackSE->y = y;
        //
        m_opintion[Button_SEC]->x = x + se * 190.f;
        m_opintion[Button_SEC]->y = y;
    }
    // Wnd
    {
        float x = m_opintion[Button_WndA]->x + 40.f;
        float y = m_opintion[Button_WndA]->y;
        m_pBackWndZoom->x = x;
        m_pBackWndZoom->y = y;
    }
    // 修改
    if (new_bgm != bgm) {
        AudioEngine.Master_Volume(new_bgm);
    }
    if (new_se != se) {
        AudioEngine.Master_Volume(new_se);
    }
    // 父类
    Super::update();
}

