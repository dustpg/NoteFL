#pragma once

#include <cstdint>
#include "pfdUtil.h"

#ifndef PATHFD_NOVTABLE
#ifdef _MSC_VER
#define PATHFD_NOVTABLE __declspec(novtable)
#else
#define PATHFD_NOVTABLE
#endif
#endif
#pragma warning(disable: 4200)

// pathfd 命名空间
namespace PathFD {
    // FD结构
    struct Finder {
        // 迷宫数据
        const uint8_t*  data;
        // 迷宫宽度
        int16_t         width;
        // 迷宫高度
        int16_t         height;
        // 起点位置X
        int16_t         startx;
        // 起点位置Y
        int16_t         starty;
        // 终点位置X
        int16_t         goalx;
        // 终点位置Y
        int16_t         goaly;
        // 是否是8方向
        bool            dir8;
        // 未使用
        bool            unused[3];
    };
    // 路径点
    struct PathPoint { int16_t x, y; };
    // 路径
    struct Path { uint32_t len; PathPoint pt[0]; };
    // 接口
    struct PATHFD_NOVTABLE IFDInterface {
        // 释放对象
        virtual void Dispose() noexcept = 0;
    };
    // 寻路算法
    struct PATHFD_NOVTABLE IFDAlgorithm : IFDInterface {
        // 执行算法, 返回路径(成功的话), 需要调用者调用std::free释放
        virtual auto Execute(const PathFD::Finder& fd) noexcept->Path* = 0;
        // 可视化步进开始
        virtual void BeginStep(const PathFD::Finder& fd) noexcept = 0;
        // 可视化步进, 写入可视化数据(保证访问安全), 返回true表示结束
        virtual bool NextStep(void* cells, void* num, bool refresh) noexcept = 0;
        // 结束可视化步进
        virtual void EndStep() noexcept = 0;
    };
    // 创建A*算法
    auto CreateAStarAlgorithm() noexcept ->IFDAlgorithm*;
    // 创建Greedy BFS算法
    auto CreateGreedyBFSAlgorithm() noexcept ->IFDAlgorithm*;
    // 创建Dijkstra算法
    auto CreateDijkstraAlgorithm() noexcept ->IFDAlgorithm*;
}