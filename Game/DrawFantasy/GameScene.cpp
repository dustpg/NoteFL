#include "stdafx.h"
#include "included.h"


// HelloScene: 运行
void HelloScene::Run() GameExitThrow {
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
TitleScene::~TitleScene() {
    ::SafeRelease(m_pBackground);
}


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
    // 坐标
    D2D1_RECT_F rect_x = { 0.f,0.f, BUTTON_WIDTH, BUTTON_HEIGHT };
    // 动画
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
        }
        rect_x.left += BUTTON_WIDTH;
        rect_x.right += BUTTON_WIDTH;
        ybase += BUTTON_YPLUS;
        atime += BUTTON_ANIMATION;
    }
    ::SafeRelease(bitmap);
    m_app.ShowCaption();
}


// TitleScene: 刷新
void TitleScene::update() GameExitThrow {
    for (auto& btn : m_buttons)  btn.Update();
    Super::update();
}
