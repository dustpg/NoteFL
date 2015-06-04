#pragma once

// 用途:  包含待修改头文件 以免相互包含

// 游戏用互斥锁
using GameMutex = std::recursive_mutex;

// 在矩形内
static inline auto InRect(float x, float y, const D2D1_RECT_F& rect) {
    return x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom;
}
// 标题动画时间
static constexpr float CAPTION_ANIMATION_TIME = 0.20f;
// VC++ 异常规范
#ifdef _MSC_VER
#pragma warning (disable:4290)
#endif
struct GameExitExpt { };
// 退出异常
#define GameExitThrow throw(GameExitExpt&)
// GameExitThrow noexcept
// 提前声明
class ImageRenderer;
class ThisApp;
// 游戏回调
using GameCallBack = void(*)(void* game_object, void* call_data);
// 基本
#include "Util.h"
#include "UITimer.h"
#include "Sprite.h"
// 精灵
#include <list> 
using SpriteList = std::list<Sprite>;
// 剩余
#include "EzButton.h"
#include "GameScene.h"

#include "GameInput.h"
#include "ImageRenderer.h"
#include "ThisApp.h"


// 数组长度
#define lengthof(a) (sizeof(a)/sizeof(*(a)))
// 更新一帧
#define UpdateOneFrame (Sprite::s_pImageRenderer)->OnRender(1)