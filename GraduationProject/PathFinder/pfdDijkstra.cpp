#include "pfdAlgorithm.h"
#include "pdfImpl.h"

#include <algorithm>
#include <cassert>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include <new>

/*

Dijkstra 算法:
  在tile地图上, Dijkstra其实就是广度优先算法

*/

// pathfd 命名空间
namespace PathFD {
    // impl
    namespace impl {
        // 步进操作类
        template<typename T> struct dijk_step_op {
            // 设置OPEN表
            template<typename Y>inline void set_list(Y& l) {
                list = &l;
            }
            // 设置OPEN表
            inline void set_open_list_begin(size_t obegin) {
                openbegin = obegin;
            }
            // 路径已经找到
            template<typename Y> inline void found(const Y& list) {
                parent.SetFinished();
            }
            // 是否继续寻找
            inline bool go_on() {
                return !parent.IsExit();
            }
            // 寻找路径失败
            inline void failed() {
                parent.SetFinished();
            }
            // 提示继续执行
            inline void signal() {
                impl::signal(ev);
            }
            // 等待下一步骤
            inline void wait() {
                impl::wait(ev);
            }
            // 为表操作加锁
            inline void lock() {
                impl::lock(mx);
            }
            // 为表操作解锁
            inline void unlock() {
                impl::unlock(mx);
            }
            // 构造本类函数
            dijk_step_op(T& parent) : parent(parent) {}
            // 构造本类函数
            ~dijk_step_op() { impl::destroy(ev); impl::destroy(mx);}
            // 事件
            event               ev = impl::create_event();
            // 访问锁
            mutex               mx = impl::create_mutex();
            // 父对象
            T&                  parent;
            // OPEN表
            typename T::List*   list = nullptr;
            // OPEN表位置
            size_t              openbegin = 0;
        };
        // 操作类
        struct dijk_op {
            // 设置OPEN表
            template<typename Y> inline void set_list(Y& list) {  }
            // 设置OPEN表
            inline void set_open_list_begin(size_t obegin) { }
            // 获取一个节点
            template<typename Y> inline void get_node(const Y& node) { }
            // 路径已经找到
            template<typename Y> inline auto found(const Y& list) {
                return impl::path_found(list);
            }
            // 是否继续寻找
            inline bool go_on() { return true; }
            // 寻找路径失败
            inline auto failed() ->Path* { return nullptr; }
            // 提示继续执行
            inline void signal() { }
            // 等待下一步骤
            inline void wait() { }
            // 为表操作加锁
            inline void lock() { }
            // 为表操作解锁
            inline void unlock() { }
        };
    }
    // Dijkstra算法
    class CFDDijkstra final : public IFDAlgorithm {
    public:
        // 节点
        struct NODE {
            // 坐标
            int16_t         x, y;
            // 父节点坐标相加
            int8_t          px, py;
            // 步数
            int16_t         step;
        };
        // 列表
        using List = std::vector<CFDDijkstra::NODE>;
        // 操作
        using StepOp = impl::dijk_step_op<CFDDijkstra>;
    public:
        // 构造函数
        CFDDijkstra() noexcept;
        // 释放对象
        void Dispose() noexcept override { delete this; };
        // 执行算法, 返回路径(成功的话), 需要调用者调用std::free释放
        auto Execute(const PathFD::Finder& fd) noexcept->Path* override;
        // 可视化步进
        void BeginStep(const PathFD::Finder& fd) noexcept override;
        // 可视化步进
        bool NextStep(void* cells, void* num, bool refresh) noexcept override;
        // 结束可视化步进
        void EndStep() noexcept override;
    public:
        // 退出
        bool IsExit() const { return m_bExit; }
        // 完成
        void SetFinished() { m_bFinished = true; }
    private:
        // 析构函数
        ~CFDDijkstra() noexcept;
    private:
        // 操作数据
        StepOp                  m_opStep;
        // 线程数据
        PathFD::Finder          m_fdData;
        // 执行线程
        std::thread             m_thdStep;
        // 退出信号
        std::atomic_bool        m_bExit = false;
        // 退出信号
        std::atomic_bool        m_bFinished = false;
        // 8方向
        bool                    m_unused[2];
    };
}


/// <summary>
/// 创建A*算法
/// </summary>
/// <returns>A*算法接口</returns>
auto PathFD::CreateDijkstraAlgorithm() noexcept -> IFDAlgorithm* {
    return new(std::nothrow) PathFD::CFDDijkstra();
}


/// <summary>
/// <see cref="CFDDijkstra"/> 类构造函数
/// </summary>
PathFD::CFDDijkstra::CFDDijkstra() noexcept : m_opStep(*this) {

}


// pathfd::impl 命名空间
namespace PathFD { namespace impl {
    // 找到路径
    auto path_found(const PathFD::CFDDijkstra::List& list) noexcept {
        assert(!list.empty());
        // 包括起始点
        size_t size = size_t(list.back().step) + 1;
        assert(size);
        auto noop = list.crend();
        // 查找结点
        auto find_node = [noop](decltype(noop)& itr) noexcept -> decltype(noop)& {
            const auto& node = *itr;
            decltype(node.x) parentx = node.x + node.px;
            decltype(node.y) parenty = node.y + node.py;
            ++itr;
            while (itr != noop) {
                if (itr->x == parentx && itr->y == parenty) {
                    break;
                }
                ++itr;
            }
            return itr;
        };
#ifdef _DEBUG
        size_t debug_count = 0;
        {
            auto itr = list.crbegin();
            do { ++debug_count; } while (find_node(itr) != noop);
        }
        assert(debug_count == size);
#endif
        {
            // 申请空间
            auto* path = reinterpret_cast<PathFD::Path*>(std::malloc(
                sizeof(PathFD::Path) + sizeof(PathFD::Path::pt[0]) * size
            ));
            // 有效
            if (path) {
                path->len = uint32_t(size);
                auto pp = path->pt + size;
                auto itr = list.crbegin();
                do {
                    --pp;
                    pp->x = itr->x;
                    pp->y = itr->y;
                } while (find_node(itr) != noop);
            }
            return path;
        }
    }
    // 寻找路径ex
    template<typename OP>
    auto dijk_find_ex(OP& op, const PathFD::Finder& fd) {
        // 加锁
        op.lock();
        // 8方向
        auto direction8 = fd.dir8;
        // 起点终点数据
        const int16_t sx = fd.startx;
        const int16_t sy = fd.starty;
        const int16_t gx = fd.goalx;
        const int16_t gy = fd.goaly;
        // 遍历过数据
        auto visited = std::make_unique<uint8_t[]>(fd.width * fd.height);
        std::memset(visited.get(), 0, sizeof(uint8_t) * fd.width * fd.height);
        // 标记需要数据
        auto mk_ptr = visited.get(); int16_t mk_width = fd.width;
        // 标记遍历
        auto mark_visited = [mk_ptr, mk_width](int16_t x, int16_t y) noexcept {
            mk_ptr[x + y * mk_width] = true;
        };
        // 检查标记
        auto check_visited = [mk_ptr, mk_width](int16_t x, int16_t y) noexcept {
            return mk_ptr[x + y * mk_width];
        };
        // 检查通行
        auto cp_ptr = fd.data; int16_t cp_width = fd.width; int16_t cp_height = fd.height;
        auto check_pass = [cp_ptr, cp_width, cp_height](int16_t x, int16_t y) noexcept {
            if (x >= 0 && x < cp_width && y >= 0 && y < cp_height) {
                return !!cp_ptr[x + y * cp_width];
            }
            return false;
        };
        // 起点数据
        CFDDijkstra::NODE start; 
        start.x = sx; start.y = sy;
        start.px = 0; start.py = 0; start.step = 0;
        // 终点数据
        struct { decltype(start.x) x, y; } end;
        end.x = gx; end.y = gy;
        // 起点加入OPEN表
        CFDDijkstra::List list;
        mark_visited(start.x, start.y);
        constexpr size_t reserved = 1024 * 64;
        list.reserve(reserved);
        list.push_back(start);
        // OPEN起始位置
        size_t open_begin_index = 0;
        // OPEN终点位置
        size_t open_end_index = 1;
        // 为操作设置表数据
        op.set_list(list);
        // 解锁
        op.unlock();
        // 为空算法失败
        while (!list.empty() && op.go_on()) {
            // 从表取一个结点
            for (auto i = open_begin_index; i != open_end_index; ++i) {
                // 从表取一个结点, 迭代器可能失效
                const auto node = list[i];
                // 目标解
                if (node.x == end.x && node.y == end.y) {
                    // 加锁
                    impl::auto_locker<OP> locker(op);
                    // 重置大小
                    list.resize(i + 1);
                    // 解
                    return op.found(list);
                }
                // 移动
                auto moveto = [&](int8_t xplus, int8_t yplus) {
                    CFDDijkstra::NODE tmp; 
                    tmp.x = node.x + xplus; tmp.y = node.y + yplus; 
                    // 可以通行 并且没有遍历过
                    if (check_pass(tmp.x, tmp.y) && !check_visited(tmp.x, tmp.y)) {
                        // 标记
                        mark_visited(tmp.x, tmp.y);
                        // 记录父节点位置
                        tmp.px = -xplus; tmp.py = -yplus;
                        // 步数
                        tmp.step = node.step + 1;
                        // 加锁
                        impl::auto_locker<OP> locker(op);
                        // 添加到最后
                        list.push_back(tmp);
                    }
                };
                // 南东西北
                moveto(0, +1); moveto(-1, 0); moveto(+1, 0); moveto(0, -1);
                // 8方向
                if (direction8) { moveto(-1, +1); moveto(+1, +1); moveto(-1, -1); moveto(+1, -1); }
            }
            // 写入数据
            open_begin_index = open_end_index;
            open_end_index = list.size();
            op.set_open_list_begin(open_begin_index);
            // 等待一步
            op.wait();
        }
        // 加锁
        impl::auto_locker<OP> locker(op);
        // 失败
        return op.failed();
    }
}}


// 执行算法
auto PathFD::CFDDijkstra::Execute(const PathFD::Finder& fd) noexcept -> PathFD::Path* {
    // 正式处理
    try { impl::dijk_op op; return impl::dijk_find_ex(op, fd); }
    // 出现异常
    catch (...) { return nullptr; }
}

// 可视化步进
void PathFD::CFDDijkstra::BeginStep(const PathFD::Finder& fd) noexcept {
    assert(m_thdStep.joinable() == false);
    std::this_thread::yield();
    m_fdData = fd;
    try {
        auto& data = m_fdData;
        auto& op = m_opStep;
        m_thdStep.std::thread::~thread();
        m_thdStep.std::thread::thread([&op, &data]() {
            impl::dijk_find_ex(op, data);
        });
    }
    catch (...) { m_opStep.failed(); }
}
// 可视化步进
bool PathFD::CFDDijkstra::NextStep(void* cells, void* num, bool refresh) noexcept {
    assert(cells && "bad pointer");
    //auto count = m_fdData.width * m_fdData.height;
    if (refresh) {
        impl::color red;
        red.r = 1.f; red.g = 0.f; red.b = 0.f; red.a = 1.f;
        impl::color green;
        green.r = 0.f; green.g = 1.f; green.b = 0.f; green.a = 1.f;
        // 读取数据
        m_opStep.lock();
        // 已经结束
        if (m_bFinished) {
            m_opStep.unlock();
            return true;
        }
        assert(m_opStep.list);
        static uint32_t s_index;
        s_index = 0;
        // 显示节点数据
        auto nodedisplay = [num](const NODE& node) noexcept {
            constexpr uint32_t COUNT = 1;
            char buffer[sizeof(NodeDisplay) + sizeof(uint32_t) * COUNT];
            auto& numdis = reinterpret_cast<NodeDisplay&>(*buffer);
            // 设置
            numdis.x = node.x;
            numdis.y = node.y;
            numdis.i = s_index++;
            numdis.argc = COUNT;
            // 节点
            numdis.d = PathFD::CaculateDirection(node.px, node.py);
            // gn = 
            numdis.argv[0] = node.step;
            // 显示数字
            impl::set_node_display(num, &numdis);
        };
        auto& list = *m_opStep.list;
        auto openbegin = list.begin() + m_opStep.openbegin;
        // 为CLOSE表添加绿色
        for (auto itr = list.begin(); itr != openbegin; ++itr) {
            const auto& node = *itr;
            uint32_t index = node.x + node.y * m_fdData.width;
            impl::set_cell_color(cells, index, green);
            nodedisplay(node);
        }
        // 为OPEN表添加红色
        for (auto itr = openbegin; itr != list.end(); ++itr) {
            const auto& node = *itr;
            uint32_t index = node.x + node.y * m_fdData.width;
            impl::set_cell_color(cells, index, red);
            nodedisplay(node);
        }
        // 解数据访问锁
        m_opStep.unlock();
    }
    // 提示下一步
    m_opStep.signal();
    return false;
}

// 结束可视化步进
void PathFD::CFDDijkstra::EndStep() noexcept {
    assert(m_thdStep.joinable());
    m_bExit = true;
    m_opStep.signal();
    try { 
        m_thdStep.join();
        m_thdStep.std::thread::~thread();
        m_thdStep.std::thread::thread();
    }
    catch (...) { }
}

// 析构函数
PathFD::CFDDijkstra::~CFDDijkstra() noexcept {
    m_opStep.signal();
    m_bExit = true;
    if (m_thdStep.joinable()) m_thdStep.join();
}
