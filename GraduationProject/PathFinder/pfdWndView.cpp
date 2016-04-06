#include "pfdWndView.h"
#include "pfdUIMap.h"
#include <Core/luiWindow.h>
#include <Core/luiManager.h>
#include  "pfdAlgorithm.h"


/// <summary>
/// 清理控件
/// </summary>
/// <returns></returns>
void PathFD::CFDWndView::cleanup() noexcept {
    // 删前调用
    this->before_deleted();
    // 释放空间
    delete this;
}


/// <summary>
/// 事件处理
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns></returns>
bool PathFD::CFDWndView::DoEvent(const LongUI::EventArgument& arg) noexcept {
    switch (arg.event)
    {
    case LongUI::Event::Event_TreeBuildingFinished:
        this->init_wndview();
        __fallthrough;
    default:
        return Super::DoEvent(arg);
    }
}

// 引用函数
using LongUI::longui_cast;

// 初始化控件
void PathFD::CFDWndView::init_wndview() noexcept {
    LongUI::UIControl* ctrl = nullptr;
#ifdef _DEBUG
    UIManager << DL_Log << L"called" << LongUI::endl;
#endif
    // 地图控件
    ctrl = m_pWindow->FindControl("mapPathFD");
    m_pMapControl = longui_cast<UIMapControl*>(ctrl);
    // 地图宽度
    auto wc = m_pWindow->FindControl("edtMapWidth");
    // 地图高度
    auto wh = m_pWindow->FindControl("edtMapHeight");
    // 生成地图
    ctrl = m_pWindow->FindControl("btnMapGene");
    {
        auto map = m_pMapControl;
        ctrl->AddEventCall([wc, map, wh](LongUI::UIControl*) noexcept {
            assert(wc && wh && "bad action");
            auto w = LongUI::AtoI(wc->GetText());
            auto h = LongUI::AtoI(wh->GetText());
            map->GenerateMap(uint32_t(w), uint32_t(h));
            return true;
        }, LongUI::SubEvent::Event_ItemClicked);
    }
    // 保存地图
    ctrl = m_pWindow->FindControl("btnMapSave");
    {
        auto map = m_pMapControl;
        ctrl->AddEventCall([map](LongUI::UIControl*) noexcept {
            map->SaveMap();
            return true;
        }, LongUI::SubEvent::Event_ItemClicked);
    }
    // 载入地图
    ctrl = m_pWindow->FindControl("btnMapLoad");
    {
        auto map = m_pMapControl;
        ctrl->AddEventCall([map](LongUI::UIControl*) noexcept {
            map->LoadMap();
            return true;
        }, LongUI::SubEvent::Event_ItemClicked);
    }
    // 重置缩放
    ctrl = m_pWindow->FindControl("btnMapRezm");
    {
        auto map = m_pMapControl;
        ctrl->AddEventCall([map](LongUI::UIControl*) noexcept {
            map->ZoomMapTo(1.f, 0.5f);
            return true;
        }, LongUI::SubEvent::Event_ItemClicked);
    }
    // 开始寻路
    ctrl =  m_pWindow->FindControl("btnFinderStart");
    {
        auto display = m_pWindow->FindControl("txtDisplay");
        auto map = m_pMapControl;
        ctrl->AddEventCall([map, display](LongUI::UIControl*) noexcept {
            IFDAlgorithm* algorithm = nullptr;
            algorithm = PathFD::CreateAStarAlgorithm();
            if (algorithm) {
                LongUI::CUIString str;
                map->Execute(algorithm, str);
                display->SetText(str);
                algorithm->Dispose();
            }
            return true;
        }, LongUI::SubEvent::Event_ItemClicked);
    }
}

// pathfd 命名空间
namespace PathFD {
    // 小块内存申请
    auto AllocSmall(size_t length) noexcept -> void * {
        assert(length < 256);
        return LongUI::SmallAlloc(length);
    }
    // 小块内存释放
    void FreeSmall(void* address) noexcept {
        return LongUI::SmallFree(address);
    }
    // 获取间隔时间
    auto GetDeltaTime() noexcept -> float {
        return UIManager.GetDeltaTime();
    }
    // 按键检查
    auto InputCheck() noexcept -> CharacterDirection {
        // 下方向键检查
        if (UIInput.IsKbPressed(UIInput.KB_DOWN)) {
            return Direction_S;
        }
        // 左方向键检查
        if (UIInput.IsKbPressed(UIInput.KB_LEFT)) {
            return Direction_W;
        }
        // 右方向键检查
        if (UIInput.IsKbPressed(UIInput.KB_RIGHT)) {
            return Direction_E;
        }
        // 上方向键检查
        if (UIInput.IsKbPressed(UIInput.KB_UP)) {
            return Direction_N;
        }
        // 无
        return Direction_Nil;
    }
    // 方向偏移
    extern const DT DIRECTION_OFFSET[DIRECTION_SIZE] = {
        { 0, 1}, {-1, 0}, { 1, 0}, { 0,-1},
        {-1, 1}, { 1, 1}, {-1,-1}, { 1,-1},
    };
#ifdef _DEBUG
    // 调试输出
    void outputdebug(const wchar_t* a) {
        UIManager << DL_Log << a << LongUI::endl;
    }
#endif
}
