#pragma once

// pathfd::impl 命名空间
namespace PathFD { namespace impl {
#ifdef _DEBUG
    // 调试数输出
    void outputdebug(const wchar_t* a) noexcept;
#endif
    // 互斥锁
    struct mutex_impl; using mutex = mutex_impl*;
    // 创建互斥锁
    auto create_mutex() noexcept->mutex;
    // 摧毁互斥锁
    void destroy(mutex& mx) noexcept;
    // 上互斥锁
    void lock(mutex mx) noexcept;
    // 下互斥锁
    void unlock(mutex mx) noexcept;
    // 事件
    struct event_impl; using event = event_impl*;
    // 颜色
    struct color { float r, g, b, a; };
    // 设定指定单元格附加颜色
    void set_cell_color(void* sprite, uint32_t index, const color& c) noexcept;
    // 设置数字显示
    void set_node_display(void* num, void* display) noexcept;
    // 创建事件
    auto create_event() noexcept->event;
    // 激活事件
    void signal(event ev) noexcept;
    // 等待事件
    void wait(event ev) noexcept;
    // 摧毁事件
    void destroy(event& ev) noexcept;
}}