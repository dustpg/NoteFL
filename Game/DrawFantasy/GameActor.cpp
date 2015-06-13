#include "stdafx.h"
#include "included.h"


// 动画信息
static AnimationInfo ANIMATION_INFO[] = {
    { 48, 52,  true, 0.20f },
    { 32, 40,  true, 0.20f },
    {  4, 12,  true, 0.10f },
    { 12, 16, false, 0.20f },
    { 16, 18, false, 0.20f },
    { 18, 24, false, 0.20f },
    { 24, 28, false, 0.20f },
    { 40, 48, false, 0.15f },
};

// GameActor 构造函数
GameActor::GameActor(const GameTerrain& terrain) noexcept :m_terrain(terrain) {
    ::memset(&m_info, 0, sizeof(m_info));
    m_pSprite->src_rect = { 0.f, 0.f, 64.f, 64.f };
    m_pSprite->ox = 32.f;
    m_pSprite->oy = 64.f;
    //
    this->SetAction(Type_Standby);
}

// 设定动作
void GameActor::set_action(ActionType type) noexcept {
    if (m_nowAction != type) {
        m_nowAction = type;
        m_info = ANIMATION_INFO[type];
        m_idNowIndex = m_info.start;
        m_fActionTimeRemain = m_info.time;
    }
}

// GameActor 刷新
void GameActor::Update(float delta) noexcept {
    // 更新
    m_fDeltaTime = delta;
    m_fActionTimeRemain -= delta;
    if (m_fActionTimeRemain < 0.f) {
        m_fActionTimeRemain += m_info.time;
        if (++m_idNowIndex == m_info.end) {
            if (m_info.loop) {
                m_idNowIndex = m_info.start;
            }
            else {
                m_idNowIndex = m_idNowIndex - 1;
            }
        }
    }
    // 检查待机动画
    if (!m_bActed && !m_bUnlanded) {
        if (m_fSpeedX > 0.f) {
            m_fSpeedX -= SPPED_ATTENUATION * m_fDeltaTime;
            if (m_fSpeedX <= 0.f) {
                m_fSpeedX = 0.f;
                this->Standby();
            }
        }
    }
    m_bActed = false;
    // 检查速度
    float real_length = m_fSpeedX * m_faceRight * RUN_BASIC_LENGTH * m_fDeltaTime;
    m_x += real_length;
    m_y += m_fSpeedY * m_fDeltaTime;
    // 检查动作
    if (!m_bUnlanded) {
        if (m_fSpeedX > RUN_THRESHOLD) {
            this->set_action(GameActor::Type_Run);
        }
        else if (m_fSpeedX > 0.f) {
            this->set_action(GameActor::Type_Walk);
        }
        else {
            this->set_action(GameActor::Type_Standby);
        }
    }
    // 设置
    float x = float(m_idNowIndex % 8 * 64);
    float y = float(m_idNowIndex / 8 * 64);
    m_pSprite->src_rect = {
        x, y, x + 64.f, y + 64.f
    };
    // 修改
    m_pSprite->x = m_x;
    m_pSprite->y = m_y;
    if (real_length > 0.f) {
        m_pSprite->zoom_x = 1.f;
    }
    else if (real_length < 0.f) {
        m_pSprite->zoom_x = -1.f;
    }
    //检查
    {
        constexpr float BASIC_SIZE = 8.f;
        D2D1_RECT_F body;
        body.left = m_x - BASIC_SIZE;
        body.top = m_y - BASIC_SIZE * 2.f;
        body.right = m_x + BASIC_SIZE;
        body.bottom = m_y;
        auto ht = m_terrain.HitTest(body);
        if (!ht) {
            m_bUnlanded = true;
            m_fSpeedY += m_fDeltaTime * 500.f;
        }
        else {
            m_bUnlanded = false;
            m_fSpeedY = 0.f;
            // 地形限制
            m_y = m_terrain.Fx(ht - 1, m_x);
            //::_cwprintf(L"ID:[%d] f(%d) = %d\n", ht - 1, int(m_x), int(m_y));
        }
        if (m_y > 700.f) {
            m_bDead = true;
        }
    }
}

// 往右跑
void GameActor::RunRight() noexcept {
    m_bActed = true;
    if (m_bUnlanded) return;
    m_faceRight = 1.f; 
    m_fSpeedX += SPPED_UP * m_fDeltaTime;
    m_fSpeedX = std::min(m_fMAxSpeed, m_fSpeedX);
}

// 往左跑
void GameActor::RunLeft() noexcept {
    m_bActed = true;
    if (m_bUnlanded) return;
    m_faceRight = -1.f;
    m_fSpeedX += SPPED_UP * m_fDeltaTime;
    m_fSpeedX = std::min(m_fMAxSpeed, m_fSpeedX);
}


// 跳跃
void GameActor::Jump() noexcept {
    m_bActed = true;
    if (m_bUnlanded) return;
    m_bUnlanded = true;
    m_fSpeedY = -400.f;
    this->set_action(GameActor::Type_Jump);
}

// 碰撞检测
auto GameTerrain::HitTest(const D2D1_RECT_F & rect) const noexcept ->uint32_t {
    // 直线相交
    auto IsTwoLineIntersect = [](const Line& line1, const Line& line2) ->bool {
        float q = (line1.begin.y - line2.begin.y) * (line2.end.x - line2.begin.x) - (line1.begin.x - line2.begin.x) * (line2.end.y - line2.begin.y);
        float d = (line1.end.x - line1.begin.x) * (line2.end.y - line2.begin.y) - (line1.end.y - line1.begin.y) * (line2.end.x - line2.begin.x);
        if (d == 0.f) return false;
        float r = q / d;
        q = (line1.begin.y - line2.begin.y) * (line1.end.x - line1.begin.x) - (line1.begin.x - line2.begin.x) * (line1.end.y - line1.begin.y);
        float s = q / d;
        if (r < 0.f || r > 1.f || s < 0.f || s > 1.f) return false;
        return true;
    };
    // 检查
    for (auto i = 0u;i < m_cLineSize; ++i) {
        register const auto& line1 = m_pTerrain[i]; Line lines[4];
        auto width = rect.right - rect.left, height = rect.bottom - rect.top;
        // 0
        lines[0].begin = { rect.left, rect.top };
        lines[0].end = { lines[0].begin.x + width,  lines[0].begin.y };
        // 1
        lines[1].begin = { rect.left, rect.top + height };
        lines[1].end = { lines[1].begin.x + width,  lines[1].begin.y };
        // 2
        lines[2].begin = { rect.left, rect.top  };
        lines[2].end = { lines[2].begin.x,  lines[2].begin.y + height };
        // 3
        lines[3].begin = { rect.left + width, rect.top  };
        lines[3].end = { lines[3].begin.x,  lines[3].begin.y + height };
        // 检查
        if (IsTwoLineIntersect(line1, lines[0]) ||
            IsTwoLineIntersect(line1, lines[1]) ||
            IsTwoLineIntersect(line1, lines[2]) ||
            IsTwoLineIntersect(line1, lines[3])) {
            return i + 1;
        }
    }
    return 0;
}

// AutoAnimation 构造函数
AutoAnimation::AutoAnimation() noexcept {
    ::memset(this, 0, sizeof(AnimationInfo));
}


void AutoAnimation::Play() noexcept {
    m_played = 0;
    m_idNowIndex = this->start; 
    m_fTimeRemain = this->time;
    m_pSprite->ox = this->unit * 0.5f;
    m_pSprite->oy = this->unit * 0.5f;
}


// AutoAnimation 刷新
void AutoAnimation::Update(float delta) noexcept {
    m_fTimeRemain -= delta;
    if (m_fTimeRemain < 0.f) {
        m_fTimeRemain += this->time;
        if (++m_idNowIndex == this->end) {
            if (this->loop) {
                m_idNowIndex = this->start;
            }
            else {
                m_idNowIndex = m_idNowIndex - 1;
                m_played = 1;
            }
        }
    }
    // 设置
    float x = float(m_idNowIndex % this->size) * unit;
    float y = float(m_idNowIndex / this->size) * unit;
    m_pSprite->src_rect = { x, y, x + unit, y + unit };
}


// GameEnemy 构造函数
GameEnemy::GameEnemy(const GameTerrain& terrain) noexcept: m_terrain(terrain) {

}

// 添加敌人
auto GameEnemy::AddEnemy() noexcept -> GameActor* {
    try {
        m_list.push_back(GameActor(m_terrain));
        auto& back = m_list.back();
        back->LoadNewBitmap(L"resource/enemy.png");
        return &back;
    }
    catch (...) {
        return nullptr;
    }
}

// 刷新
void GameEnemy::Update(float delta) noexcept {
    // 遍历刷新
    for (auto& unit : m_list) {
        unit.Update(delta);
    }
}