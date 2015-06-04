// 对使用DirectInput 对键鼠进行检测

#pragma once

#define DIRECTINPUT_VERSION  0x0800
#include <dinput.h>

// 游戏键鼠输入类 本类不允许随意创建，特别是动态
class GameKeyboardMouseInput {
public:
    // 键盘缓冲区大小
    static constexpr DWORD  KEYBOARD_BUFFER_SIZE = 256;
public:
    // 初始化
    HRESULT Init(const HINSTANCE hInst, const HWND hwnd) noexcept;
public:
    // 鼠标X
    auto x()noexcept { return m_pt.x; }
    // 鼠标X
    auto y()noexcept { return m_pt.y; }
    // Mouse X puls
    auto mx()noexcept { return m_lX; }
    // Mouse Y puls
    auto my() noexcept { return m_lY; }
    // Mouse Z
    auto mz()noexcept { return m_lMouseDelta; }
    // 刷新设备
    void Update() noexcept;
    // 是否使用键盘按了键
    auto KPress(DWORD key) noexcept {
        return *(m_cKeyboardBuffer + m_dwKeyboardBufferOffsetNew + key);
    }
    // 是否使用键盘敲击了键
    auto KTrigger(DWORD key) noexcept {
        return (*(m_cKeyboardBuffer + m_dwKeyboardBufferOffsetOld + key) && !*(m_cKeyboardBuffer + m_dwKeyboardBufferOffsetNew + key));
    }
    // 是否刚按下键
    auto KKeyDown(DWORD key) noexcept {
        return (*(m_cKeyboardBuffer + m_dwKeyboardBufferOffsetNew + key) && !*(m_cKeyboardBuffer + m_dwKeyboardBufferOffsetOld + key));
    }
    // 是否使用鼠标按了键
    auto MPress(DWORD btn) noexcept {
        return !!reinterpret_cast<uint8_t*>(m_cMouseBuffer + 1)[btn];
    }
    // 是否使用鼠标敲击了键
    auto MTrigger(DWORD btn) noexcept {
        return reinterpret_cast<uint8_t*>(m_cMouseBuffer + 0)[btn]
            && !reinterpret_cast<uint8_t*>(m_cMouseBuffer + 1)[btn];
    }
    // 是否刚按下键
    auto MKeyDown(DWORD btn) noexcept {
        return !reinterpret_cast<uint8_t*>(m_cMouseBuffer + 0)[btn]
            && reinterpret_cast<uint8_t*>(m_cMouseBuffer + 1)[btn];
    }
private:
    // 构造函数
    GameKeyboardMouseInput() noexcept;
    // 析构函数
    ~GameKeyboardMouseInput() noexcept;
    // DirectInput对象指针
    IDirectInput8*          m_pDirectInputObject = nullptr;
    // DirectInput设备指针(键盘)
    IDirectInputDevice8*    m_pDirectInputDeviceKeyboard = nullptr;
    // DirectInput设备指针(鼠标)
    IDirectInputDevice8*    m_pDirectInputDeviceMouse = nullptr;
    // 窗口句柄 用于校正鼠标位置
    HWND                    m_hwnd = nullptr;
    // 键盘状态缓冲 2倍 用于比较是否触发(Trigger)某键
    bool                    m_cKeyboardBuffer[KEYBOARD_BUFFER_SIZE * 2];
    // 键盘状态缓冲偏移值 指向旧值
    DWORD                   m_dwKeyboardBufferOffsetOld = 0;
    // 键盘状态缓冲偏移值 指向新值
    DWORD                   m_dwKeyboardBufferOffsetNew = GameKeyboardMouseInput::KEYBOARD_BUFFER_SIZE;
    // 鼠标状态缓冲 2倍 用于比较是否触发(Trigger)某键
    DIMOUSESTATE            m_cMouseBuffer[2];
    // 鼠标
    POINT                   m_pt;
    // 鼠标当前X坐标
    LONG                    m_lX = 0;
    // 鼠标当前Y坐标
    LONG                    m_lY = 0;
    // 鼠标滑轮
    LONG                    m_lMouseDelta = 0;
public:
    // GameKeyboardMouseInput 一号实例 欲支持多人请多添加静态变量
    static GameKeyboardMouseInput  s_inputPlayer1;
};

#define KMInput (GameKeyboardMouseInput::s_inputPlayer1)

