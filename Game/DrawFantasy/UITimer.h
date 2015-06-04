#pragma once


// the timer of ui in frame
class UITimer {
public:
    // refresh the frequency
    auto RefreshFrequency() noexcept { ::QueryPerformanceFrequency(&m_cpuFrequency); }
    // start timer
    auto Start() noexcept { ::QueryPerformanceCounter(&m_cpuCounterStart); }
    // move end var to start var
    auto  MovStartEnd() noexcept { m_cpuCounterStart = m_cpuCounterEnd; }
    // delta time in sec.
    template<typename T>
    auto Delta_s() {
        ::QueryPerformanceCounter(&m_cpuCounterEnd);
        return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart) / static_cast<T>(m_cpuFrequency.QuadPart);
    }
    // delta time in ms.
    template<typename T>
    auto Delta_ms() {
        ::QueryPerformanceCounter(&m_cpuCounterEnd);
        return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e3) / static_cast<T>(m_cpuFrequency.QuadPart);
    }
    // delta time in micro sec.
    template<typename T>
    auto Delta_mcs() {
        ::QueryPerformanceCounter(&m_cpuCounterEnd);
        return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e6) / static_cast<T>(m_cpuFrequency.QuadPart);
    }
private:
    // CPU 当前频率
    LARGE_INTEGER            m_cpuFrequency;
    // CPU 开始计时时间
    LARGE_INTEGER            m_cpuCounterStart;
    // CPU 结束计时时间
    LARGE_INTEGER            m_cpuCounterEnd;
public:
    // 构造函数
    UITimer() { m_cpuCounterStart.QuadPart = 0; m_cpuCounterEnd.QuadPart = 0; RefreshFrequency(); }
};