#include "pfdCharacter.h"
#include <algorithm>
#include <cassert>
#include <memory>
#undef min
#undef max

// pathfd 命名空间
namespace PathFD {
    // 安全释放
    template<class T> void SafeRelease(T*& iii) noexcept {
        if (iii) {
            iii->Release();
            iii = nullptr;
        }
    }
    // 安全引用
    template<class T> inline auto SafeAcquire(T* pInterfaceToRelease) {
        if (pInterfaceToRelease) {
            pInterfaceToRelease->AddRef();
        }
        return pInterfaceToRelease;
    }
}

/// <summary>
/// <see cref="CFDCharacter"/> 类 构造函数
/// </summary>
PathFD::CFDCharacter::CFDCharacter() noexcept {

}


/// <summary>
/// 释放数据
/// </summary>
/// <returns></returns>
void PathFD::CFDCharacter::release_data() noexcept {
    PathFD::SafeRelease(m_pBitmap);
    PathFD::SafeRelease(m_pRenderTarget);
    if (m_pCharData) {
        PathFD::FreeSmall(m_pCharData);
        m_pCharData = nullptr;
    }
}


/// <summary>
/// 重置地图
/// </summary>
/// <param name="data">The data.</param>
/// <returns></returns>
void PathFD::CFDCharacter::ResetMap(const MapData& data) noexcept {
    // 检查参数
    assert(data.map_data && "bad map data");
    assert(data.map_width && "bad map width");
    assert(data.map_height && "bad map height");
    assert(data.cell_width && "bad cell width");
    assert(data.cell_height && "bad cell height");
    // 赋值
    m_map = data;
    // 刷新
    this->refresh_position();
}


/// <summary>
/// 重置角色图像
/// </summary>
/// <param name="bitmap">The bitmap.</param>
/// <returns></returns>
void PathFD::CFDCharacter::ResetChar(ID2D1Bitmap1* bitmap, const CharData& data) noexcept {
    // 检查参数
    assert(data.atime > 0.f && "bad action time");
    assert(data.speed > 0.f && "bad speed");
    assert(data.acount && "bad action count");
    assert(data.width && "bad character width");
    assert(data.height && "bad character height");
    assert(bitmap && "bad bitmap");
    // 释放老数据
    PathFD::SafeRelease(m_pBitmap);
    if (m_pCharData) PathFD::FreeSmall(m_pCharData);
    // 复制数据
    m_pBitmap = PathFD::SafeAcquire(bitmap);
    const size_t len = sizeof(CharData) + sizeof(CharData::action[0]) * data.acount;
    m_pCharData = reinterpret_cast<CharData*>(PathFD::AllocSmall(len));
    assert(m_pCharData && "OOM for just 'len' byte");
    std::memcpy(m_pCharData, &data, len);
}

/// <summary>
/// 重置渲染目标呈现器
/// </summary>
/// <param name="d2ddc">The D2DDC.</param>
/// <returns></returns>
void PathFD::CFDCharacter::ResetRenderTarget(ID2D1DeviceContext2* d2ddc) noexcept {
    PathFD::SafeRelease(m_pRenderTarget);
    m_pRenderTarget = PathFD::SafeAcquire(d2ddc);
}

/// <summary>
/// 渲染角色
/// </summary>
/// <returns></returns>
void PathFD::CFDCharacter::Render() const noexcept {
    if (!m_pCharData) return;
    const auto& chardata = *m_pCharData;
    // 角色宽高
    const float charw = float(chardata.width);
    const float charh = float(chardata.height);
    // 计算目标矩形
    D2D1_RECT_F des;
    {
        des.left = m_fPosX;
        des.top = m_fPosY;
        des.right = des.left + charw;
        des.bottom = des.top + charh;
    }
    // 计算源矩形
    D2D1_RECT_F src;
    {
        // X坐标是靠"源X偏移"和"动作"计算 
        src.left = float(
            chardata.src_offsetx +
            chardata.width * chardata.action[m_ixAction]
            );
        // Y坐标是靠"源X偏移"和"朝向"计算 
        src.top = float(
            chardata.src_offsety +
            chardata.width * chardata.direction
            );
        // 宽高就是角色宽高
        src.right = src.left + charw;
        src.bottom = src.top + charh;
    }
    // 刻画图像
    m_pRenderTarget->DrawBitmap(
        m_pBitmap,
        &des,
        1.f,
        D2D1_INTERPOLATION_MODE(m_uCharInter),
        &src,
        nullptr
    );
}

/// <summary>
/// Refresh_positions this instance.
/// </summary>
/// <returns></returns>
void PathFD::CFDCharacter::refresh_position() noexcept {
    const auto& chardata = *m_pCharData;
    DT dt = { 0,0 };
    // 移动中
    if (this->IsMoving()) dt = DIRECTION_OFFSET[m_dtMoving];
    // TODO: XY坐标还有单元格和角色大小比较, 这里默认一致
    float x = float(dt.x) * m_fMoveOffset + float(m_map.char_x);
    float y = float(dt.y) * m_fMoveOffset + float(m_map.char_y);
    // X坐标是靠"目标X偏移","X位置"和"移动偏移量"计算
    x = float(chardata.des_offsetx) + x * float(m_map.cell_width);
    // Y坐标是靠"目标Y偏移","Y位置"和"移动偏移量"计算
    y = float(chardata.des_offsety) + y * float(m_map.cell_height);
    // 需要刷新
    m_bNeedRefresh |= (int(x) != int(m_fPosX)) | (int(y) != int(m_fPosY));
    // 更新数据
    m_fPosX = x; m_fPosY = y;
}

/// <summary>
/// 刷新本对象
/// </summary>
/// <returns></returns>
bool PathFD::CFDCharacter::Update() noexcept {
    const auto& chardata = *m_pCharData;
    // 移动计算
    if (this->IsMoving()) {
        m_fMoveOffset += PathFD::GetDeltaTime() * chardata.speed;
        this->refresh_position();
        if (m_fMoveOffset >= 1.f) {
            m_fMoveOffset = 0.f;
            auto dt = DIRECTION_OFFSET[m_dtMoving];
            m_dtMoving = Direction_Nil;
            m_map.char_x += dt.x;
            m_map.char_y += dt.y;
            m_bNeedRefresh = true;
            this->EndAction();
        }
    }
    // 更新动作
    if (m_bInAction) {
        m_fActionTime += PathFD::GetDeltaTime();
        // 索引计算
        auto findex = m_fActionTime / chardata.atime * float(chardata.acount);
        uint32_t index = uint32_t(findex);
        if (m_fActionTime > chardata.atime) {
            m_fActionTime = 0.f;
            index = chardata.acount - 1;
        }
        // 设置新的索引
        if (index != m_ixAction) {
            m_ixAction = index;
            m_bNeedRefresh = true;
        }
    }
    {
        bool bk = m_bNeedRefresh;
        m_bNeedRefresh = false;
        return bk;
    }
}

/// <summary>
/// 接受输入
/// </summary>
/// <param name="d">The d.</param>
/// <returns></returns>
void PathFD::CFDCharacter::Input(CharacterDirection d) noexcept {
    // 移动中
    if (this->IsMoving()) return;
    // 输入有效
    if (d != PathFD::Direction_Nil) {
        if (m_pCharData->direction != d) {
            m_pCharData->direction = d;
            m_bNeedRefresh = true;
        }
        assert(d < DIRECTION_SIZE);
        // 检查是否可以通过
        auto dt = DIRECTION_OFFSET[d];
        uint32_t x = uint32_t(dt.x + int32_t(m_map.char_x));
        uint32_t y = uint32_t(dt.y + int32_t(m_map.char_y));
        if (x < m_map.map_width && y < m_map.map_height) {
            uint32_t index = x + y * m_map.map_width;
            // 允许通行
            if (m_map.map_data[index]) {
                m_dtMoving = d;
                this->BeginAction();
            }
        }
    }
}

/// <summary>
/// 执行动作
/// </summary>
void PathFD::CFDCharacter::BeginAction() {
    m_bInAction = true;
    //m_fActionTime = 0.f;
    //m_ixAction = 0;
}

/// <summary>
/// 结束动作
/// </summary>
void PathFD::CFDCharacter::EndAction() {
    m_bInAction = false;
    //m_fActionTime = 0.f;
    //m_ixAction = 0;
}
