#pragma once

#include <cstdint>

// pathfd 命名空间
namespace PathFD {
    // 角色方位
    enum CharacterDirection : uint32_t {
        Direction_Nil = uint32_t(-1),
        Direction_S = 0,    // 南方
        Direction_W,        // 西方
        Direction_E,        // 东方
        Direction_N,        // 北方
        Direction_SW,       // 西南
        Direction_SE,       // 东南
        Direction_NW,       // 西北
        Direction_NE,       // 东北
        DIRECTION_SIZE,     // 数量大小
    };
    // 方向
    struct DT { int32_t x, y; };
    // 方向偏移
    extern const DT DIRECTION_OFFSET[DIRECTION_SIZE];
    // 获取每帧间隔时间
    auto GetDeltaTime() noexcept -> float;
    // 申请小块内存
    auto AllocSmall(size_t length) noexcept -> void*;
    // 申请小块内存
    void FreeSmall(void* address) noexcept;
    // 按键检查
    auto InputCheck() noexcept ->CharacterDirection;
}