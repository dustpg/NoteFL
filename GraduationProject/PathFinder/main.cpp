#include "pfdConfig.h"
#include "pfdWndView.h"

// Xml Layout
constexpr char* const LAY_XML =
u8R"(<?xml version="1.0" encoding="utf-8"?>
<Window titlename="Path Finder">
    <HorizontalLayout>
        <Single weight="3" margin="4,4,4,4" borderwidth="1" bgbrush="1">
            <ScrollBarA wheelstep="128" marginal="bottom"/>
            <ScrollBarA wheelstep="128" marginal="right"/>
            <PathMap size="32, 32" name="mapPathFD" mapicon="6" steptime="0.1"
                charbitmap="4" mapbitmap="5"/>
        </Single>
        <VerticalLayout templatesize="256, 0" bgbrush="2" hostposterity="true">
            <Null weight="0.1"/>
            <HorizontalLayout >
                <Text weight="0.6" text="地图宽度:" textrenderer="outline" textcontext="2.33"/>
                <Edit bgbrush="3" templateid="1" text="32" name="edtMapWidth"/>
            </HorizontalLayout>
            <HorizontalLayout >
                <Text weight="0.6" text="地图高度:" textrenderer="outline" textcontext="2.33"/>
                <Edit bgbrush="3" templateid="1" text="32" name="edtMapHeight" />
            </HorizontalLayout>
            <HorizontalLayout>
                <Text weight="0.6" text="生成算法:" textrenderer="outline" textcontext="2.33"/>
                <ComboBox drawdownarrow="true" textoffsetx="4" name="cbbAlgGen" align="left" templateid="3" >
                    <List linetemplate="Text">
                        <ScrollBarA marginal="right"/>
                        <ListLine><Text text="地牢"/></ListLine>
                        <ListLine><Text text="迷宫"/></ListLine>
                    </List>
                </ComboBox>
            </HorizontalLayout>
            <HorizontalLayout>
                <Button templateid="3" text="生成地图" name="btnMapGene"/>
                <Button templateid="3" text="载入地图" name="btnMapLoad"/>
                <Button templateid="3" text="保存地图" name="btnMapSave"/>
            </HorizontalLayout>
            <HorizontalLayout>
                <Button templateid="3" text="重置缩放" name="btnMapRezm"/>
                <Button templateid="3" text="清除地图" name="btnMapCler"/>
            </HorizontalLayout>
            <HorizontalLayout>
                <RadioButton text="通行方格" checked="true"/>
                <RadioButton text="地图视图"/>
            </HorizontalLayout>
            <HorizontalLayout>
                <RadioButton text="四方向" checked="true"/>
                <RadioButton name="rdoDirection8" text="八方向"/>
            </HorizontalLayout>
            <HorizontalLayout >
                <Text weight="0.6" text="步进间隔:" />
                <Edit bgbrush="3" templateid="4" text="0.1" name="edtMapStep"/>
            </HorizontalLayout>
            <HorizontalLayout>
                <Text weight="0.6" text="寻路算法:"/>
                <ComboBox drawdownarrow="true" textoffsetx="4" name="cbbAlgPath" align="left" templateid="3" >
                    <List linetemplate="Text">
                        <ScrollBarA marginal="right"/>
                    </List>
                </ComboBox>
            </HorizontalLayout>
            <HorizontalLayout>
                <Button templateid="3" text="开始寻路" name="btnFinderStart"/>
                <Button templateid="3" text="开始演示" name="btnFinderShow"/>
            </HorizontalLayout>
            <HorizontalLayout>
                <Button templateid="3" text="步进演示" name="btnFinderStep"/>
                <Button templateid="3" text="暂停/恢复" name="btnFinderPaRe"/>
            </HorizontalLayout>
            <Text text="----" name="txtDisplay"/>
        </VerticalLayout>
    </HorizontalLayout>
</Window>
)";

// Res Xml
constexpr char* const RES_XML =
u8R"(<?xml version="1.0" encoding="utf-8"?>
<Res>
    <!-- Bitmap区域 -->
    <Bitmap>
        <!-- You can use other name not limited in 'Item' -->
        <Item desc="按钮1"    res="res/btn.png"/>
        <Item desc="背景"     res="res/darksouls.jpg"/>
        <Item desc="背景3A"   res="res/darksouls3-4.jpg"/>
        <Item desc="角色图"   res="res/Actor4.png"/>
        <Item desc="外部地图" res="res/Outside_A4.png"/>
        <Item desc="待用图片" res="res/icons.png"/>
    </Bitmap>
    <!-- Brush区域 -->
    <Brush>
        <!-- You can use other name not limited in 'Item' -->
        <Item desc="背景" type="bitmap" bitmap="2"/>
        <Item desc="背景" type="bitmap" bitmap="3"/>
        <Item desc="背景" type="solid" color="1,1,1,0.2"/>
    </Brush>
    <!-- Meta区域Zone -->
    <Meta>
        <Item desc="按钮1无效图元" bitmap="1" rect="0,  0, 96, 24" rule="button"/>
        <Item desc="按钮1通常图元" bitmap="1" rect="0, 72, 96, 96" rule="button"/>
        <Item desc="按钮1悬浮图元" bitmap="1" rect="0, 24, 96, 48" rule="button"/>
        <Item desc="按钮1按下图元" bitmap="1" rect="0, 48, 96, 72" rule="button"/>
    </Meta>
</Res>
)";

// Template Xml
constexpr char* const TMP_XML =
u8R"(<?xml version="1.0" encoding="utf-8"?>
<Template>
    <!-- 编号 1 -->
    <Control desc="数字输入" margin="4,4,4,4" borderwidth="1" 
        textnumber="true" textmax="1024" textmin="2"/>
    <!-- 编号 2 -->
    <Control desc="类系统钮" margin="4,4,4,4" borderwidth="1"/>
    <!-- 编号 3 -->
    <Control desc="btn.png 按钮" margin="4,4,4,4" metagroup="1,2,3,4"/>
    <!-- 编号 4 -->
    <Control desc="一般输入" margin="4,4,4,4" borderwidth="1" 
        textmax="1024" textmin="2"/>
</Template>
)";

// Entry for App
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    // every windows desktop app should do this
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // use OleInitialize to init ole and com
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // config
        PathFD::CFDConfig config;
        // resource
        config.resource = RES_XML;
        // tmeplate
        config.tmplt = TMP_XML;
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize(&config))) {
            // my style
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // create main window, return nullptr for some error
            UIManager.CreateUIWindow<PathFD::CFDWndView>(LAY_XML)->ShowWindow(nCmdShow);
            // run this app
            UIManager.Run();
            // my style
            UIManager << DL_Hint << L"Battle Control Terminated!" << LongUI::endl;
        }
        // cleanup longui
        UIManager.Uninitialize();
    }
    // cleanup ole and com
    ::OleUninitialize();
    // exit
    return EXIT_SUCCESS;
}



/*
            // 添加
            auto insert2 = [&](const CFDAStar::NODE& node) {
                // 加锁
                op.lock();
                // 比最后的都大?
                if (open.empty() || node.fx >= open.back().fx) {
                    // 添加到最后
                    open.push_back(node);
                    // 解锁
                    op.unlock();
                    return;
                }
                // 添加节点
                for (auto itr = open.begin(); itr != open.end(); ++itr) {
                    if (node.fx < itr->fx) {
                        open.insert(itr, node);
                        // 解锁
                        op.unlock();
                        return;
                    }
                }
                // 不可能
                assert(!"Impossible ");
            };
            // 移动
            auto moveto = [&](int16_t xplus, int16_t yplus) {
                CFDAStar::NODE tmp; 
                tmp.x = node.x + xplus; 
                tmp.y = node.y + yplus; 
                // 可以通行 并且没有遍历过
                if (check_pass(tmp.x, tmp.y) && !check_visited(tmp.x, tmp.y)) {
                    // 标记
                    mark_visited(tmp.x, tmp.y);
                    // 记录父节点位置
                    tmp.parent = &node;
                    // 计算g(n)
                    tmp.gn = node.gn + 1;
                    // f(n) = g(n) + h(n)
                    tmp.fx = tmp.gn + hn(tmp.x, tmp.y);
                    // 添加
                    insert2(tmp);
                }
            };
*/