#include "stdafx.h"
#include "included.h"


// HelloScene: 运行
void HelloScene::Run() GameExitThrow {
#ifdef _DEBUG
    ID2D1Bitmap1* bitmap = nullptr;
    m_app.LoadBitmapFromFile(L"resource/All.png", &bitmap);
    ::SafeRelease(bitmap);
    m_app.LoadBitmapFromFile(L"resource/background1.png", &bitmap);
    ::SafeRelease(bitmap);
    // 载入场景
    m_app.LoadScene<GameScene>();
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
    // 继续游戏
    if (m_buttons[Button_Con].Update()) {
        // 载入场景
        m_app.LoadScene<GameScene>();
    }
    // 游戏选项
    if (m_buttons[Button_Opt].Update()) {
        // 选项菜单有效？
        if (m_opintion[Button_BGMA]->x > 0.f) {
            m_buttons[Button_Ply]->ChangeTo(m_buttons[Button_Ply].end, 0.4f);
            m_buttons[Button_New]->ChangeTo(m_buttons[Button_New].end, 0.45f);
            m_buttons[Button_Con]->ChangeTo(m_buttons[Button_Con].end, 0.5f);
            // 
            for (auto& btn : m_opintion) {
                btn->ChangeTo(btn.start, 0.3f);
            }
        }
        // 选项菜单无效
        else {
            m_buttons[Button_Ply]->ChangeTo(m_buttons[Button_Ply].start, 0.4f);
            m_buttons[Button_New]->ChangeTo(m_buttons[Button_New].start, 0.45f);
            m_buttons[Button_Con]->ChangeTo(m_buttons[Button_Con].start, 0.5f);
            //
            for (auto& btn : m_opintion) {
                if ((&btn - m_opintion) % 3 != 2) {
                    btn->ChangeTo(btn.end, 0.3f);
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
        m_opintion[Button_WndC]->x = x;
        m_opintion[Button_WndC]->y = y;
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



// 结束前
void TitleScene::pre_terminate() noexcept {
    // 动画
    for (auto& btn : m_buttons) btn->ChangeTo(btn.start, 1.f);
    m_pBackground->MoveTo(-800.f, 0.f, 0.5f);
    Super::pre_terminate();
    // 检查
    while (m_pBackground->IsMoving()) {
        try {
            for (auto& btn : m_buttons) btn.Update();
            Super::update();
        }
        catch (...) {
            break;
        }
    }
}

// ----------------------------------

// GameScene 析构函数
GameScene::~GameScene() noexcept {
    for (auto& sprite : m_apSprites) {
        ::SafeRelease(sprite);
    }
}



// 地形
GameTerrain::Line GAME_LINES[] = {
    {   0.f, 530.f, 300.f, 540.f },
    { 300.f, 530.f, 480.f, 480.f },
    { 560.f, 460.f, 800.f, 555.f },
};

constexpr float HUD_YPOS1 = 600.f;
constexpr float HUD_YPOS2 = 32.f;
constexpr float HUD_YPOS3 = 34.f;
constexpr float HUD_YPOS4 = 16.f;

constexpr float HUD_XPOS_SL = 128.f;
constexpr float HUD_XPOS_SP = HUD_XPOS_SL + 16.f;
constexpr float HUD_XPOS_HP = 80.f;
constexpr float HUD_XPOS_SK = 0.f;


//
constexpr float HUD_MENU_UNIT = 176.f;
// 阿斯顿 
static const D2D1_RECT_F GAME_HUD_SRC_RECTS[] = {
    { 0.f, 0.f, 0.f, 0.f },
    { 512.f, 269.f, 640.f, 296.f },
    { 512.f, 308.f, 512.f, 332.f },
    { 0.f, 0.f, 0.f, 0.f },
    { 512.f, 336.f, 512.f+32.f, 368.f },
    { 1024.f - HUD_MENU_UNIT * 2.f, 1024.f - HUD_MENU_UNIT * 2.f,1024.f, 1024.f },
    { 
        1024.f - HUD_MENU_UNIT * 4.f, 1024.f - HUD_MENU_UNIT * 2.f, 
        1024.f - HUD_MENU_UNIT * 2.f, 1024.f 
    },
};


// GameScene 开始
void GameScene::start() noexcept {
    {
        static_assert(lengthof(GAME_HUD_SRC_RECTS) == SPRITE_SIZE, "ck");
        ID2D1Bitmap1* bitmap = nullptr;
        m_app.LoadBitmapFromFile(L"resource/All.png", &bitmap);
        auto srect = GAME_HUD_SRC_RECTS;
        for (auto& sprite : m_apSprites) {
            sprite = Sprite::New();
            sprite->atype = AnimationType::Type_CubicEaseIn;
            if (&sprite == m_apSprites) {
                sprite->LoadNewBitmap(L"resource/background1.png");
            }
            else {
                sprite->SetNewBitmap(bitmap);
                sprite->src_rect = *srect;
            }
            ++srect;
        }
        ::SafeRelease(bitmap);
        m_player->LoadNewBitmap(L"resource/player.png");
    }
    // 设置
    m_terrain.Set(lengthof(GAME_LINES), GAME_LINES);
    m_apSprites[Sprite_Background]->y = -800.f;
    m_apSprites[Sprite_Background]->MoveTo(0.f, 0.f, 0.2333f);
    // 载入
    m_apSprites[Sprite_SkillSlot]->z = 299;
    m_apSprites[Sprite_SkillSlot]->x = HUD_XPOS_SL;
    m_apSprites[Sprite_SkillSlot]->y = HUD_YPOS1;
    m_apSprites[Sprite_SkillSlot]->MoveTo(m_apSprites[Sprite_SkillSlot]->x, HUD_YPOS3, 0.5f);

    m_apSprites[Sprite_SkillPoint]->z = 300;
    m_apSprites[Sprite_SkillPoint]->x = HUD_XPOS_SP;
    m_apSprites[Sprite_SkillPoint]->y = HUD_YPOS1;
    m_apSprites[Sprite_SkillPoint]->MoveTo(m_apSprites[Sprite_SkillPoint]->x, HUD_YPOS3, 0.5f);


    m_apSprites[Sprite_Health]->z = 300;
    m_apSprites[Sprite_Health]->x = HUD_XPOS_HP;
    m_apSprites[Sprite_Health]->y = HUD_YPOS1;
    m_apSprites[Sprite_Health]->MoveTo(m_apSprites[Sprite_Health]->x, HUD_YPOS2, 0.5f);

    m_apSprites[Sprite_Skill]->z = 300;
    m_apSprites[Sprite_Skill]->x = -HUD_XPOS_HP;
    m_apSprites[Sprite_Skill]->MoveTo(HUD_XPOS_SK, HUD_YPOS4, 0.5f);

    m_apSprites[Sprite_MenuA]->z = 500;
    m_apSprites[Sprite_MenuA]->opacity = 0.f;
    m_apSprites[Sprite_MenuA]->ox = HUD_MENU_UNIT;
    m_apSprites[Sprite_MenuA]->oy = HUD_MENU_UNIT;
    m_apSprites[Sprite_MenuB]->z = 499;
    m_apSprites[Sprite_MenuB]->opacity = 0.f;
    m_apSprites[Sprite_MenuB]->ox = HUD_MENU_UNIT;
    m_apSprites[Sprite_MenuB]->oy = HUD_MENU_UNIT;

    m_player.SetPos(50.f, 400.f);
    m_player->z = 200;

    m_skillAnimation->z = 210;

}



// GameScene 刷新
void GameScene::update() GameExitThrow {
    // 鼠标左键
    if (KMInput.MPress(DIMOFS_BUTTON0)) {
        if (KMInput.x() > m_player->x) {
            m_player.RunRight();
        }
        else {
            m_player.RunLeft();
        }
    }
    // 空格
    if (KMInput.KKeyDown(DIK_SPACE)) {
        m_player.Jump();
    }
    // 刷新玩家
    this->updata_player();
    // 刷新菜单
    this->updata_menu();
    // 父类
    Super::update();
}

// GameTerrain 获取Y坐标
auto GameTerrain::Fx(size_t no, float x) const noexcept -> float {
    const register auto& line = m_pTerrain[no];
    auto k = (line.end.y - line.begin.y) / (line.end.x - line.begin.x);
    return (x - line.begin.x) * k + line.begin.y;
}

// GameScene 构造函数
GameScene::GameScene(ThisApp& g) noexcept: Super(g), m_player(m_terrain){
    // 初始化
    m_playerData.skill_id = 0;
    m_playerData.old_id = SKILL_SIZE;
    m_playerData.hp = 3;
    m_playerData.sp = 0;
    //
    m_playerData.skill_id = Skill_Heal;
    // 清理
    ::memset(&data, 0, sizeof(data));
};


// 技能坐标
static const D2D1_RECT_F SKILL_ID[]{
    { 0.f, 515.f, 70.f, 585.f },
    { 0.f, 585.f, 70.f, 655.f },
    { 0.f, 445.f, 70.f, 515.f },
    { 0.f, 655.f, 70.f, 725.f },
};


// GameScene 刷新玩家
void GameScene::updata_player() noexcept {
    // 释放技能
    if (KMInput.MTrigger(DIMOFS_BUTTON2)) {
        m_playerData.sp = 4;
        // 消费技能点
        if (m_playerData.sp > m_playerData.skill_id) {
            m_playerData.sp -= m_playerData.skill_id;
            --m_playerData.sp;
            this->set_skill_animation();
        }
        // 不够技能点
        else {

        }
    }
    // 刷新技能动画
    m_skillAnimation.Update(m_fDeltaTime);
    // 刷新技能
    this->updata_skill();
    // 死亡?
    if (!m_playerData.hp || m_player.Dead()) {
        ::MessageBoxW(nullptr, L"你死了", L"提示", MB_OK);
        // 载入场景
        m_app.LoadScene<HelloScene>();
    }
    // 技能
    m_apSprites[Sprite_Skill]->src_rect = SKILL_ID[m_playerData.skill_id];
    // 技能点
    constexpr float SPSTEP = 24.f;
    m_apSprites[Sprite_SkillPoint]->src_rect.right =
        m_apSprites[Sprite_SkillPoint]->src_rect.left + float(m_playerData.sp) * SPSTEP;
    // 刷新
    m_player.Update(m_fDeltaTime);
}


// 数据
static uint32_t MENU_DATA[8] = { 0, 2, 3, 2, 0, 1, 3, 1 };

// GameScene 刷新菜单
void GameScene::updata_menu() noexcept {
    constexpr float MENU_ANIMATION_TIME = 0.233f;
    constexpr float TIME_ZOOM = 0.1f;
    // 鼠标右键
    if (KMInput.MKeyDown(DIMOFS_BUTTON1)) {
        // TIME SKIP!
        m_app.SetTimeScale(TIME_ZOOM);
        m_app.SetBackgroundBlur(450, 5.f);
        // 菜单
        SpriteStatus ss;
        ss.x = float(KMInput.x()); ss.y = float(KMInput.y());
        ss.ox = ss.oy = HUD_MENU_UNIT;
        ss.zoom_x = ss.zoom_y = 1.f;
        ss.opacity = 0.6f; ss.rotation = 0.f;
        //ss.opacity = 0.8f;
        m_apSprites[Sprite_MenuA]->zoom_x = 0.f;
        m_apSprites[Sprite_MenuA]->zoom_y = 0.f;
        m_apSprites[Sprite_MenuA]->x = ss.x;
        m_apSprites[Sprite_MenuA]->y = ss.y;
        m_apSprites[Sprite_MenuB]->zoom_x = 0.f;
        m_apSprites[Sprite_MenuB]->zoom_y = 0.f;
        m_apSprites[Sprite_MenuB]->x = ss.x;
        m_apSprites[Sprite_MenuB]->y = ss.y;
        //
        m_apSprites[Sprite_MenuA]->ChangeTo(ss, MENU_ANIMATION_TIME * TIME_ZOOM);
    }
    // 鼠标右键
    if (KMInput.MTrigger(DIMOFS_BUTTON1)) {
        m_app.SetTimeScale(1.0f);
        m_app.SetBackgroundBlur(450, 0.f);
        // 菜单
        SpriteStatus ss;
        ss.x = m_apSprites[Sprite_MenuA]->x; 
        ss.y = m_apSprites[Sprite_MenuA]->y;
        ss.ox = ss.oy = HUD_MENU_UNIT;
        ss.zoom_x = ss.zoom_y = 0.f;
        ss.opacity = 0.0f; ss.rotation = 0.f;
        m_apSprites[Sprite_MenuA]->ChangeTo(ss, MENU_ANIMATION_TIME);
        m_apSprites[Sprite_MenuB]->ChangeTo(ss, MENU_ANIMATION_TIME);
        //
        m_playerData.skill_id = m_playerData.old_id;
    }
    // 菜单有效
    if (KMInput.MPress(DIMOFS_BUTTON1)) {
        // 获取技能编号
        auto get_skill_id = [](float x, float y) noexcept -> uint32_t {
            // 大于?
            if (x*x + y*y > HUD_MENU_UNIT*HUD_MENU_UNIT) {
                return SKILL_SIZE;
            }
            // 编码
            uint32_t id = ((x > 0.f) << 2) | ((y > 0.f) << 1) | (std::abs(x) > std::abs(y));
            return MENU_DATA[id];
        };
        float x = float(KMInput.x()) - m_apSprites[Sprite_MenuA]->x;
        float y = float(KMInput.y()) - m_apSprites[Sprite_MenuA]->y;
        auto id = get_skill_id(x, y);
        // 检查
        if (id == SKILL_SIZE) id = m_playerData.skill_id;
        // 不同
        if (id != m_playerData.old_id) {
            m_playerData.old_id = id;
            SpriteStatus ss; ss.Init();
            ss.ox = ss.oy = HUD_MENU_UNIT;
            ss.x = m_apSprites[Sprite_MenuA]->x;
            ss.y = m_apSprites[Sprite_MenuA]->y;
            ss.opacity = 0.8f;
            float angles[4] = { 270.f, 0.f, 180.f, 90.f };
            ss.rotation = angles[id];
            m_apSprites[Sprite_MenuB]->ChangeTo(ss, MENU_ANIMATION_TIME * TIME_ZOOM);
        }
    }
}

// 设置技能动画
void GameScene::set_skill_animation() noexcept {
    constexpr float GOD_TIME = 3.f;
    // 释放技能
    m_skillAnimation.id = m_playerData.skill_id + 1;
    m_skillAnimation->z = 210;
    switch (Skill(m_playerData.skill_id))
    {
    case GameScene::Skill_FireBall:
        m_skillAnimation->x = m_player->x;
        m_skillAnimation->y = m_player->y - 32.f;
        m_skillAnimation->LoadNewBitmap(L"resource/fire.png");
        m_skillAnimation.size = 5;
        m_skillAnimation.unit = 192.f;
        m_skillAnimation.start = 0;
        m_skillAnimation.end = 10;
        m_skillAnimation.loop = false;
        m_skillAnimation.time = 0.2f;
        // 主角位置
        this->data.fire_direction = m_player.GetFaced();
        this->data.fire_x = m_player.GetSpeedX();
        this->data.fire_y = m_player.GetSpeedY();
        break;
    case GameScene::Skill_Heal:
        m_skillAnimation->x = m_player->x;
        m_skillAnimation->y = m_player->y - 32.f;
        m_skillAnimation->LoadNewBitmap(L"resource/healgod.png");
        m_skillAnimation.size = 5;
        m_skillAnimation.unit = 192.f;
        m_skillAnimation.start = 1;
        m_skillAnimation.end = 8;
        m_skillAnimation.loop = false;
        m_skillAnimation.time = 0.1f;
        break;
    case GameScene::Skill_AoE:
        m_skillAnimation->x = float(KMInput.x());
        m_skillAnimation->y = float(KMInput.y());
        m_skillAnimation->LoadNewBitmap(L"resource/aoe.png");
        m_skillAnimation.size = 5;
        m_skillAnimation.unit = 192.f;
        m_skillAnimation.start = 0;
        m_skillAnimation.end = 8;
        m_skillAnimation.loop = false;
        m_skillAnimation.time = 0.1f;
        break;
    case GameScene::Skill_God:
        m_skillAnimation->x = m_player->x;
        m_skillAnimation->y = m_player->y - 32.f;
        m_skillAnimation->z = 150;
        m_skillAnimation->LoadNewBitmap(L"resource/healgod.png");
        m_skillAnimation.size = 5;
        m_skillAnimation.unit = 192.f;
        m_skillAnimation.start = 20;
        m_skillAnimation.end = 25;
        m_skillAnimation.loop = true;
        m_skillAnimation.time = 0.1f;
        this->data.god_time = GOD_TIME;
        break;
    }

    m_skillAnimation.Play();
}

// 刷新技能
void GameScene::updata_skill() noexcept {
    if (!m_skillAnimation.id) return;
    constexpr float FIREBALL_SPPED = 300.f;
    switch (Skill(m_skillAnimation.id - 1))
    {
    case GameScene::Skill_FireBall:
        m_skillAnimation->x += this->data.fire_direction * m_fDeltaTime * 
            (FIREBALL_SPPED + this->data.fire_x);
        m_skillAnimation->y += this->data.fire_y * m_fDeltaTime;
        if (m_skillAnimation->x > 900.f || m_skillAnimation->x < -100.f) {
            m_skillAnimation.id = 0;
            m_skillAnimation->SetNewBitmap(nullptr);
        }
        break;
    case GameScene::Skill_Heal:
        m_skillAnimation->x = m_player->x;
        m_skillAnimation->y = m_player->y - 32.f;
        if (m_skillAnimation.IsEnd()) {
            m_skillAnimation.id = 0;
            m_skillAnimation->SetNewBitmap(nullptr);
        }
        break;
    case GameScene::Skill_AoE:
        if (m_skillAnimation.IsEnd()) {
            m_skillAnimation.id = 0;
            m_skillAnimation->SetNewBitmap(nullptr);
        }
        break;
    case GameScene::Skill_God:
        m_skillAnimation->x = m_player->x;
        m_skillAnimation->y = m_player->y - 32.f;
        this->data.god_time -= m_fDeltaTime;
        if (this->data.god_time < 0.f) {
            m_skillAnimation.id = 0;
            m_skillAnimation->SetNewBitmap(nullptr);
        }
        break;
    }
}