#pragma once

#include <LongUI.h>

// pathfd 命名空间
namespace PathFD {
    // 创建地图控件
    auto CreateMapControl(LongUI::CreateEventType, pugi::xml_node) noexcept->LongUI::UIControl*;
    // 程序配置信息
    class CFDConfig final : public LongUI::CUIDefaultConfigure {
        // 父类
        using Super = CUIDefaultConfigure;
    public:
        // template string
        const char*     tmplt = nullptr;
        // 构造函数
        CFDConfig() : Super(UIManager) { }
        // 字体地域名称
        auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept ->void override {
            std::wcscpy(name, L"en-us");
        };
        // 返回flag
        auto GetConfigureFlag() noexcept ->ConfigureFlag override { 
            return Flag_OutputDebugString /*| Flag_RenderByCPU /*| Flag_DbgOutputFontFamily*/;
        }
        // 获取模板字符串
        auto GetTemplateString() noexcept ->const char* override { return tmplt; }
        // 注册控件
        auto RegisterSome() noexcept ->void override {
            m_manager.RegisterControlClass(CreateMapControl, "PathMap");
        };
        // 选择显卡
        auto ChooseAdapter(const DXGI_ADAPTER_DESC1 adapters[], const size_t length) noexcept -> size_t override {
            // Intel 测试
            for (size_t i = 0; i < length; ++i) {
                if (!std::wcsncmp(L"Intel", adapters[i].Description, 5))
                    return i;
            }
            // 核显卡
            for (size_t i = 0; i < length; ++i) {
                if (!std::wcsncmp(L"NVIDIA", adapters[i].Description, 6))
                    return i;
            }
            return length;
        }
    };
}