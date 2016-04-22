#pragma once

#include <luibase.h>
#include <luiconf.h>
#include <Control/UIControl.h>
#include <Platonly/luiPoHlper.h>
#include "pfdCharacter.h"

// 文件对话框
struct IFileDialog;

// pathfd 命名空间
namespace PathFD {
    // 路径
    struct Path;
    // 算法
    struct IFDAlgorithm;
    // 迷宫生产算法
    using DungeonGeneration = bool(*)(uint8_t* data, uint32_t w, uint32_t h, uint32_t []);
    // 默认迷宫生产算法
    bool DefaultGeneration(uint8_t* data, uint32_t w, uint32_t h, uint32_t []) noexcept;
    // 地图控件
    class UIMapControl final : public LongUI::UIControl {
        // 点击
        enum ClickinType : uint16_t {
            Type_Cell = 0,      // 单元格
            Type_Charactar,     // 角色格
            Type_Start,         // 起点格
            Type_Goal,          // 终点格
        };
        // 父类
        using Super = UIControl;
        // 清除控件
        virtual void cleanup() noexcept override;
        // 双击帮助器
        using DoubleClickHelper = LongUI::Helper::DoubleClick;
    public:
        // 选择框宽度
        static constexpr float  CEEL_SELECT_WIDTH = 4.f;
        // 常量
        enum : uint32_t {
            // 地图最大宽度
            MAX_WIDTH = 1024,
            // 地图最大高度
            MAX_HEIGHT = 1024,
            // 地图单位宽度初始化值
            CELL_WIDTH_INIT = 32,
            // 地图单位高度初始化值
            CELL_HEIGHT_INIT = 32,
            // 路径权重节点宽度
            WEIGHT_NODE_BITMAP_WH = 1024,
            // 数字源X偏移(像素)
            DIGNUMNER_SRCX_OFFSET = 0,
            // 数字源Y偏移(像素)
            DIGNUMNER_SRCY_OFFSET = 32 * 2,
            // 数字宽度(像素)
            DIGNUMNER_WIDTH = 4,
            // 数字高度(像素)
            DIGNUMNER_HEIGHT = 8,
            // 数字位数
            NUMCOUNT = 4,
            // 数字表宽度(像素)
            NUMBERTABLE_WIDTH = 1024,
            // 数字表高度(像素)
            NUMBERTABLE_HEIGHT = 1024,
        };
    public:
        // 渲染 
        virtual void Render() const noexcept override;
        // 刷新
        virtual void Update() noexcept override;
        // 事件处理
        virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
        // 鼠标事件
        virtual bool DoMouseEvent(const LongUI::MouseEventArgument& arg) noexcept override;
        // 重建
        virtual auto Recreate() noexcept ->HRESULT override;
    public:
        // 载入地图
        void LoadMap() noexcept;
        // 载入地图
        void LoadMap(const wchar_t* filename) noexcept;
        // 保存地图
        void SaveMap() noexcept;
        // 保存地图
        void SaveMap(const wchar_t* filename) noexcept;
        // 地图
        void GenerateMap(uint32_t width, uint32_t height) noexcept;
        // 重置地图单位大小
        void ResizeCellSize(uint32_t width, uint32_t height) noexcept;
        // 缩放地图
        void ZoomMapTo(float zoom, float time) noexcept;
        // 执行寻路
        void Execute(IFDAlgorithm* algorithm, LongUI::CUIString& info) noexcept;
        // 开始演示
        void BeginShow(IFDAlgorithm*&& algorithm) noexcept;
        // 执行下一步
        void ExeNextStep() noexcept;
        // 暂停恢复
        void PauseResume() noexcept;
        // 设置步进间隔时间
        void SetStepDeltaTime(float time) noexcept { m_fAlgorithmStepTimeAll = time; }
    public:
        // 构造函数
        UIMapControl(LongUI::UIContainer* cp) noexcept;
        // 创建控件
        static auto CreateControl(LongUI::CreateEventType type, pugi::xml_node) noexcept ->UIControl*;
    protected:
        // 析构函数
        ~UIMapControl() noexcept;
        // 渲染链: 渲染背景
        void render_chain_background() const noexcept;
        // 渲染链: 渲染主体
        void render_chain_main() const noexcept;
        // 渲染链: 渲染背景
        void render_chain_foreground() const noexcept;
        // 删除前调用
        void before_deleted() noexcept { Super::before_deleted(); }
        // 初始化
        void initialize(pugi::xml_node node) noexcept;
        // 保证数据有效
        bool require_mapdata(int32_t width, uint32_t height) noexcept;
        // 重建设备资源
        void release_resource() noexcept;
        // 重置地图
        void reset_map() noexcept;
        // 重置精灵
        void reset_sprites() noexcept;
        // 获取角色数据
        auto&get_char_data() noexcept { return *reinterpret_cast<CharData*>(m_bufCharData); }
        // 执行下一步
        void exe_next_step() noexcept;
#ifdef LongUIDebugEvent
    protected:
        // debug infomation
        virtual bool debug_do_event(const LongUI::DebugEventInformation&) const noexcept override;
#endif
    private:
        // 当前算法
        IFDAlgorithm*           m_pAlgorithm = nullptr;
        // 文件打开对话框
        IFileDialog*            m_pFileOpenDialog = nullptr;
        // 文件保存对话框
        IFileDialog*            m_pFileSaveDialog = nullptr;
        // 地图分界线笔刷
        ID2D1BitmapBrush*       m_pCellBoundaryBrush = nullptr;
        // 地图精灵集
        ID2D1SpriteBatch*       m_pMapSpriteBatch = nullptr;
        // 路径显示精灵集
        ID2D1SpriteBatch*       m_pPathDisplay = nullptr;
        // 地图自动瓦片地图缓存
        ID2D1Bitmap1*           m_pAutoTileCache = nullptr;
        // 地图皮肤
        ID2D1Bitmap1*           m_pMapSkin = nullptr;
        // 地图相关图标
        ID2D1Bitmap1*           m_pMapIcon = nullptr;
        // 数字表位图
        ID2D1Bitmap1*           m_pNumnberTable = nullptr;
        // 地图数据
        uint8_t*                m_pMapCells = nullptr;
        // 路径信息
        PathFD::Path*           m_pPath = nullptr;
        // 迷宫生成算法
        DungeonGeneration       m_fnGeneration = PathFD::DefaultGeneration;
        // 角色 显示
        CFDCharacter            m_char;
        // 地图数据
        PathFD::MapData         m_dataMap;
        // 双击帮助器
        DoubleClickHelper       m_hlpDbClick;
        // 地图当前精灵数量
        uint32_t                m_uMapSpriteCount = 0;
        // 路径当前精灵数量
        //uint32_t                m_uPathSpriteCount = 0;
        // 选中点x位置
        uint32_t                m_uClickX = uint32_t(-1);
        // 选中点x位置
        uint32_t                m_uClickY = uint32_t(-1);
        // 终点x位置
        uint32_t                m_uGoalX = 0;
        // 终点x位置
        uint32_t                m_uGoalY = 0;
        // 算法 总共间隔时间
        float                   m_fAlgorithmStepTimeAll = 0.3f;
        // 算法 当前间隔时间
        float                   m_fAlgorithmStepTimeNow = 0.f;
        // 鼠标点击类型
        ClickinType             m_typeClicked = Type_Cell;
        // 角色位图
        uint16_t                m_uCharBitmap = 0;
        // 地图位图
        uint16_t                m_uMapBitmap = 0;
        // 图标位图id
        uint16_t                m_idMapIcon = 0;
        // 未使用
        uint16_t                m_unused_u16_map[2];
        // 角色数据缓存 :目前只需要4个动作
        char                    m_bufCharData[sizeof(CharData) + sizeof(CharData::action[0]) * 4];
    };
}

#ifdef LongUIDebugEvent
// longui namespace
namespace LongUI {
    // 重载?特例化 GetIID
    template<> LongUIInline const IID& GetIID<PathFD::UIMapControl>() {
        // {F46C5DE3-D761-4E9A-9D1A-4A301BB45CBE}
        static const GUID IID_PathFD_UIMapControl = {
            0xf46c5de3, 0xd761, 0x4e9a, { 0x9d, 0x1a, 0x4a, 0x30, 0x1b, 0xb4, 0x5c, 0xbe }
        };
        return IID_PathFD_UIMapControl;
    }
}
#endif
