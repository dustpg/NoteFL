#pragma once

#include <cstdint>
#include <d2d1_3.h>
#include "pfdUtil.h"

#pragma warning(disable: 4200)

// pathfd 命名空间
namespace PathFD {
    // 地图数据
    struct MapData {
        // 地图数据
        const uint8_t*          map_data;
        // 地图宽度(单元格)
        uint32_t                map_width;
        // 地图高度(单元格)
        uint32_t                map_height;
        // 角色X坐标(单元格)
        uint32_t                char_x;
        // 角色Y坐标(单元格)
        uint32_t                char_y;
        // 单元格宽度(像素)
        uint32_t                cell_width;
        // 单元格高度(像素)
        uint32_t                cell_height;
    };
    // 角色数据
    struct CharData {
        // 角色宽度(像素)
        uint32_t                width;
        // 角色高度(像素)
        uint32_t                height;
        // 源X偏移(像素)
        uint32_t                src_offsetx;
        // 源Y偏移(像素)
        uint32_t                src_offsety;
        // 目标X偏移(像素)
        uint32_t                des_offsetx;
        // 目标Y偏移(像素)
        uint32_t                des_offsety;
        // 角色朝向
        CharacterDirection      direction;
        // 动作数量
        uint32_t                acount;
        // 动作消耗时间
        float                   atime;
        // 移动速度
        float                   speed;
        // 动作数据
        uint32_t                action[0];
    };
    // 角色显示
    class CFDCharacter final {
    public:
        // 构造函数
        CFDCharacter() noexcept;
        // 析构函数
        ~CFDCharacter() noexcept { this->release_data(); }
        // 刷新, 若需要重新渲染着返回true
        bool Update() noexcept;
        // 输入
        void Input(CharacterDirection d) noexcept;
        // 渲染
        void Render() const noexcept;
        // 重置地图数据
        void ResetMap(const MapData& data) noexcept;
        // 重置角色图像
        void ResetChar(ID2D1Bitmap1* bitmap, const CharData& data) noexcept;
        // 重置渲染目标呈现器
        void ResetRenderTarget(ID2D1DeviceContext2* d2ddc) noexcept;
        // 执行动作
        void BeginAction();
        // 结束动作
        void EndAction();
    public:
        // 移动中
        bool IsMoving() const noexcept { return m_dtMoving != PathFD::Direction_Nil; }
        // 获取X像素坐标
        auto GetPxX() const noexcept { return; }
        // 获取X像素坐标
        auto GetPxY() const noexcept { return;}
    private:
        // 释放数据
        void release_data() noexcept;
        // 刷新位置
        void refresh_position() noexcept;
    private:
        // 渲染目标呈现器
        ID2D1DeviceContext2*    m_pRenderTarget = nullptr;
        // 所使用位图
        ID2D1Bitmap1*           m_pBitmap = nullptr;
        // 地图数据
        MapData                 m_map;
        // 角色数据
        CharData*               m_pCharData = nullptr;
        // 动作索引
        uint32_t                m_ixAction = 0;
        // 动作事件
        float                   m_fActionTime = 0.f;
        // 移动方位
        CharacterDirection      m_dtMoving = Direction_Nil;
        // 移动偏移率 范围[0, 1]
        float                   m_fMoveOffset = 0.f;
        // x 像素位置
        float                   m_fPosX = 0.f;
        // y 橡素位置
        float                   m_fPosY = 0.f;
        // 未使用
        //uint32_t                m_unused_u32 = 0;
        // 角色插值算法
        uint16_t                m_uCharInter = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
        // 刷新
        bool                    m_bNeedRefresh = false;
        // 动作中
        bool                    m_bInAction = false;
    };
}