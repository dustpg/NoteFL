#include "stdafx.h"
#include "included.h"



// 创建新的精灵
Sprite* Sprite::New() noexcept {
    try {
        return s_pImageRenderer->NewSprite();
    }
    catch (...) {
        return nullptr;
    }
}

// 刷新帧
auto Sprite::UpdateFrame(UINT syn) throw(GameExitExpt &)-> float {
    // 退出
    if (s_pImageRenderer->m_app.IsExit()) {
        throw GameExitExpt();
    }
    // 解锁
    s_pImageRenderer->m_app.Unlock();
    // 渲染
    auto delta = s_pImageRenderer->OnRender(syn);
    // 刷新输入
    KMInput.Update();
    // 加锁
    s_pImageRenderer->m_app.Lock();
    return delta;
}

auto Sprite::UpdateFrameNoExcept(UINT syn) noexcept -> float {
    if (s_pImageRenderer->m_app.IsExit()) return -0.01f;
    // 解锁
    s_pImageRenderer->m_app.Unlock();
    // 渲染
    auto delta = s_pImageRenderer->OnRender(syn);
    // 刷新输入
    KMInput.Update();
    // 加锁
    s_pImageRenderer->m_app.Lock();
    return delta;
}


// 初始化
void SpriteStatus::Init() noexcept {
    // X 坐标
    this->x = 0.f;
    // Y 坐标
    this->y = 0.f;
    // 原点X 坐标
    this->ox = 0.f;
    // 原点Y 坐标
    this->oy = 0.f;
    // X 轴放大率
    this->zoom_x = 1.f;
    // Y 轴放大率
    this->zoom_y = 1.f;
    // 不透明度
    this->opacity = 1.f;
    // 旋转度数
    this->rotation = 0.f;
}

// Sprite: 构造函数
Sprite::Sprite() noexcept {
    this->Init();
}

// Sprite: 移动构造
Sprite::Sprite(Sprite&& sprite) {
    ::memcpy(this, &sprite, sizeof(sprite));
    sprite.m_pBitmap = nullptr;
}

// Sprite: 释放
void Sprite::Release() noexcept {
    s_pImageRenderer->ReleaseSprite(this);
}

// 新的位图
void Sprite::SetNewBitmap(ID2D1Bitmap1 * bitmap) noexcept {
    ::SafeRelease(m_pBitmap);
    m_pBitmap = ::SafeAcquire(bitmap);
    if (bitmap) {
        auto size = bitmap->GetSize();
        m_fWidth = size.width;
        m_fHeight = size.height;
        this->src_rect = { 0.f, 0.f,size.width, size.height };
    }
}

// 载入新的位图
void Sprite::LoadNewBitmap(const wchar_t* path) noexcept {
    ID2D1Bitmap1 * bitmap = nullptr;
    s_pImageRenderer->LoadBitmapFromFile(path, &bitmap);
    this->SetNewBitmap(bitmap);
    ::SafeRelease(bitmap);
}

// Sprite: 刷新
void Sprite::Updata(float delta) noexcept {
    // 检查动画
    if (m_fTime > 0.f) {
        m_fTime -= delta;
        if (m_fTime <= 0.f) {
            m_fTime = 0.f;
            // 事件
            if(m_pMoveEnd) m_pMoveEnd(this, m_pMoveEndData);
        }
        // 计算值
        register auto value = ::EasingFunction(this->atype, m_fTime / this->duration);
        // 更新
#define UpdataValue(m) this->m = value * (m_oldStatus.m - m_tarStatus.m) + m_tarStatus.m;
        UpdataValue(x);
        UpdataValue(y);
        UpdataValue(ox);
        UpdataValue(oy);
        UpdataValue(zoom_x);
        UpdataValue(zoom_y);
        UpdataValue(opacity);
        UpdataValue(rotation);
#undef  UpdataValue
    }
    // 检查矩阵
    {
        this->make_matrix();
    }
    // 检查Z坐标
    if (this->z != m_iOldZ) {
        this->z = m_iOldZ;
        s_pImageRenderer->NeedSort();
    }
}


// 改变
void Sprite::change_to(const SpriteStatus& target, float time) noexcept {
    m_tarStatus = target;
    // 保存状态
    m_oldStatus.x = this->x;
    m_oldStatus.y = this->y;
    m_oldStatus.ox = this->ox;
    m_oldStatus.oy = this->oy;
    m_oldStatus.zoom_x = this->zoom_x;
    m_oldStatus.zoom_y = this->zoom_y;
    m_oldStatus.opacity = this->opacity;
    m_oldStatus.rotation = this->rotation;
    // 开始动画
    m_fTime = this->duration = time;
}

// Sprite: 创建转换矩阵
void Sprite::make_matrix() noexcept {
    register D2D1_POINT_2F center = { this->ox , this->oy };
    m_matrix = D2D1::Matrix3x2F::Scale(this->zoom_x, this->zoom_y, center)
        * D2D1::Matrix3x2F::Rotation(rotation, center)
        * D2D1::Matrix3x2F::Translation(this->x - this->ox, this->y - this->oy);

}