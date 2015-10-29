// 高性能计时器
#pragma once

// the timer - high
class HTimer {
public:
    // QueryPerformanceCounter
    static inline auto QueryPerformanceCounter(LARGE_INTEGER* ll) noexcept {
        auto old = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
        auto r = ::QueryPerformanceCounter(ll);
        ::SetThreadAffinityMask(::GetCurrentThread(), old);
        return r;
    }
    // refresh the frequency
    auto inline RefreshFrequency() noexcept { ::QueryPerformanceFrequency(&m_cpuFrequency); }
    // start timer
    auto inline Start() noexcept { HTimer::QueryPerformanceCounter(&m_cpuCounterStart); }
    // move end var to start var
    auto inline MovStartEnd() noexcept { m_cpuCounterStart = m_cpuCounterEnd; }
    // delta time in sec.
    template<typename T> auto inline Delta_s() noexcept {
        HTimer::QueryPerformanceCounter(&m_cpuCounterEnd);
        return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart) / static_cast<T>(m_cpuFrequency.QuadPart);
    }
    // delta time in ms.
    template<typename T> auto inline Delta_ms() noexcept {
        HTimer::QueryPerformanceCounter(&m_cpuCounterEnd);
        return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e3) / static_cast<T>(m_cpuFrequency.QuadPart);
    }
    // delta time in micro sec.
    template<typename T> auto inline Delta_mcs() noexcept {
        HTimer::QueryPerformanceCounter(&m_cpuCounterEnd);
        return static_cast<T>(m_cpuCounterEnd.QuadPart - m_cpuCounterStart.QuadPart)*static_cast<T>(1e6) / static_cast<T>(m_cpuFrequency.QuadPart);
    }
private:
    // CPU freq
    LARGE_INTEGER            m_cpuFrequency;
    // CPU start counter
    LARGE_INTEGER            m_cpuCounterStart;
    // CPU end counter
    LARGE_INTEGER            m_cpuCounterEnd;
public:
    // ctor
    HTimer() noexcept { m_cpuCounterStart.QuadPart = 0; m_cpuCounterEnd.QuadPart = 0; RefreshFrequency(); }
};

#include <Mmsystem.h>
// the timer : medium
class MTimer {
public:
    // refresh the frequency
    auto inline RefreshFrequency() noexcept { }
    // start timer
    auto inline Start() noexcept { m_dwStart = ::timeGetTime(); }
    // move end var to start var
    auto inline MovStartEnd() noexcept { m_dwStart = m_dwNow; }
    // delta time in sec.
    template<typename T> auto inline Delta_s() noexcept {
        m_dwNow = ::timeGetTime();
        return static_cast<T>(m_dwNow - m_dwStart) * static_cast<T>(0.001);
    }
    // delta time in ms.
    template<typename T> auto inline Delta_ms() noexcept {
        m_dwNow = ::timeGetTime();
        return static_cast<T>(m_dwNow - m_dwStart);
    }
    // delta time in micro sec.
    template<typename T> auto inline Delta_mcs() noexcept {
        m_dwNow = ::timeGetTime();
        return static_cast<T>(m_dwNow - m_dwStart) * static_cast<T>(1000);
    }
private:
    // start time
    DWORD                   m_dwStart = 0;
    // now time
    DWORD                   m_dwNow = 0;
public:
    // ctor
    MTimer() noexcept { this->Start(); }
};