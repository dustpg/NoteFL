#pragma once

#include <luibase.h>
#include <luiconf.h>
#include <Control/UIViewport.h>
#include "pfdAlgorithm.h"

// pathfd 命名空间
namespace PathFD {
    // 地图控件
    class UIMapControl;
    // 程序窗口视图
    class CFDWndView final : public LongUI::UIViewport {
        // 父类申明
        using Super = LongUI::UIViewport;
        // 友元申明
        friend class Super;
        // 清除控件
        virtual void cleanup() noexcept override;
    public:
        // 构造函数
        CFDWndView(LongUI::XUIBaseWindow* window) : Super(window) { }
    public:
        // 事件处理
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
    protected:
        // 删前处理
        void before_deleted() noexcept { Super::before_deleted(); }
        // 初始化
        void init_wndview() noexcept;
        // 添加算法
        void add_algorithm() noexcept;
        // 创建算法
        static auto create_algorithm(uint32_t id) noexcept -> IFDAlgorithm*;
    private:
        // 不允许复制构造
        CFDWndView(const CFDWndView&) = delete;
        // 析构函数
        ~CFDWndView() = default;
    private:
        // 本地图控件
        UIMapControl*           m_pMapControl = nullptr;
        // 算法选择框
        UIControl*              m_pCcbAlgorithm = nullptr;
    };
}