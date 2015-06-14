#include "stdafx.h"
#include "included.h"




// GameKeyboardMouseInput构造函数
GameKeyboardMouseInput::GameKeyboardMouseInput() noexcept {
    // 清空缓存区
    ZeroMemory(m_cKeyboardBuffer, sizeof(m_cKeyboardBuffer));
    ZeroMemory(m_cMouseBuffer, sizeof(m_cMouseBuffer));
}



// GameKeyboardMouseInput初始化 函数
HRESULT GameKeyboardMouseInput::Init(const HINSTANCE hInst, const HWND hwnd) noexcept {
    m_hwnd = hwnd;
    // 已经存在的话就不需要了
    if (m_pDirectInputObject)
        return S_FALSE;
    HRESULT hr = S_OK;
    // 创建DInput对象
    if SUCCEEDED(hr) {
        hr = ::DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDirectInputObject, 0);
    }
    // 创建DInput鼠标设备
    if SUCCEEDED(hr) {
        hr = m_pDirectInputObject->CreateDevice(GUID_SysMouse, &m_pDirectInputDeviceMouse, 0);
    }
    // 设置数据格式 :鼠标
    if SUCCEEDED(hr) {
        hr = m_pDirectInputDeviceMouse->SetDataFormat(&c_dfDIMouse);
    }
    // 设置协作等级 不独占
    if SUCCEEDED(hr) {
        hr = m_pDirectInputDeviceMouse->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    }
    // 创建DInput键盘设备
    if SUCCEEDED(hr) {
        hr = m_pDirectInputObject->CreateDevice(GUID_SysKeyboard, &m_pDirectInputDeviceKeyboard, 0);
    }
    // 设置数据格式 :键盘
    if SUCCEEDED(hr) {
        hr = m_pDirectInputDeviceKeyboard->SetDataFormat(&c_dfDIKeyboard);
    }
    // 设置协作等级 不独占
    if SUCCEEDED(hr) {
        hr = m_pDirectInputDeviceKeyboard->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
    }
    // 获得鼠标输入设备 通知操作系统已经准备完毕
    if SUCCEEDED(hr) {
        hr = m_pDirectInputDeviceMouse->Acquire();
    }
    // 获得键盘输入设备 通知操作系统已经准备完毕
    if SUCCEEDED(hr) {
        hr = m_pDirectInputDeviceKeyboard->Acquire();
    }
    // 刷新一次
    if (SUCCEEDED(hr)) {
        this->Update();
    }
    return hr;
}

// 鼠标更新
void GameKeyboardMouseInput::Update() noexcept {
    auto zoom = Sprite::s_pImageRenderer->WindowScale();
    // 更新鼠标状态 还有应对设备丢失情况
    if (m_pDirectInputDeviceMouse) {
        ::memcpy(m_cMouseBuffer, m_cMouseBuffer + 1, sizeof(DIMOUSESTATE));
        if (DIERR_INPUTLOST == m_pDirectInputDeviceMouse->GetDeviceState(
            sizeof(DIMOUSESTATE), m_cMouseBuffer + 1))
            m_pDirectInputDeviceMouse->Acquire();
        m_lMouseDelta = m_cMouseBuffer[1].lZ;
        // 获取鼠标
        ::GetCursorPos(&m_pt); ::ScreenToClient(m_hwnd, &m_pt);
        m_pt.x = LONG(float(m_pt.x) / float(zoom));
        m_pt.y = LONG(float(m_pt.y) / float(zoom));
        m_lX = m_cMouseBuffer[1].lX;
        m_lY = m_cMouseBuffer[1].lY;
    }
    // 更新键盘状态 还有应对设备丢失情况
    if (m_pDirectInputDeviceKeyboard) {
        if (DIERR_INPUTLOST == m_pDirectInputDeviceKeyboard->GetDeviceState(
            GameKeyboardMouseInput::KEYBOARD_BUFFER_SIZE, m_cKeyboardBuffer + m_dwKeyboardBufferOffsetOld))
            m_pDirectInputDeviceKeyboard->Acquire();
        m_dwKeyboardBufferOffsetOld = m_dwKeyboardBufferOffsetOld ? 0 : GameKeyboardMouseInput::KEYBOARD_BUFFER_SIZE;
        m_dwKeyboardBufferOffsetNew = m_dwKeyboardBufferOffsetNew ? 0 : GameKeyboardMouseInput::KEYBOARD_BUFFER_SIZE;
    }
}


// GameKeyboardMouseInput析构函数
GameKeyboardMouseInput::~GameKeyboardMouseInput() noexcept {
    // 放弃鼠标设备
    if (m_pDirectInputDeviceMouse)
        m_pDirectInputDeviceMouse->Unacquire();
    ::SafeRelease(m_pDirectInputDeviceMouse);

    // 放弃键盘设备
    if (m_pDirectInputDeviceKeyboard)
        m_pDirectInputDeviceKeyboard->Unacquire();
    ::SafeRelease(m_pDirectInputDeviceKeyboard);

    ::SafeRelease(m_pDirectInputObject);
}



