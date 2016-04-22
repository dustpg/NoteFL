#include "pfdAlgorithm.h"
#include "pdfImpl.h"

#include <algorithm>
#include <cassert>
#include <atomic>
#include <thread>
#include <memory>
#include <list>
#include <new>

/*

A*算法流程：

首先将起始结点S放入OPEN表，CLOSE表置空，算法开始时：
1、如果OPEN表不为空，从表头取一个结点n，如果为空算法失败。

2、n是目标解吗？是，找到一个解（继续寻找，或终止算法）。

3、将n的所有后继结点展开，就是从n可以直接关联的结点（子结点），
如果不在CLOSE表中，就将它们放入OPEN表，并把S放入CLOSE表，
同时计算每一个后继结点的估价值f(n)，将OPEN表按f(x)排序，
最小的放在表头，重复算法，回到1。
*/

// pathfd 命名空间
namespace PathFD {
    // impl
    namespace impl {
        // 步进操作类
        template<typename T> struct astar_step_op {
            // 设置OPEN表
            template<typename Y>inline void set_open_list(Y& list) {
                openlist = &list;
            }
            // 设置CLOSE表
            template<typename Y>inline void set_close_list(Y& list) {
                closelist = &list;
            }
            // 获取一个节点
            template<typename Y> inline void get_node(const Y& node) {
                int bk = 9;
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
            astar_step_op(T& parent) : parent(parent) {}
            // 构造本类函数
            ~astar_step_op() { impl::destroy(ev); impl::destroy(mx);}
            // 事件
            event               ev = impl::create_event();
            // 访问锁
            mutex               mx = impl::create_mutex();
            // 父对象
            T&                  parent;
            // OPEN表
            typename T::List*   openlist = nullptr;
            // CLOSE表
            typename T::List*   closelist = nullptr;
        };
    }
    // 自定义分配器
    template<typename T> class Allocator {
    public : 
        //    typedefs
        typedef T value_type;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;

    public : 
        //    convert an allocator<T> to allocator<U>
        template<typename U>
        struct rebind {
            typedef Allocator<U> other;
        };

    public : 
        inline explicit Allocator() {}
        inline ~Allocator() {}
        inline explicit Allocator(Allocator const&) {}
        template<typename U>
        inline explicit Allocator(Allocator<U> const&) {}

        //    address
        inline pointer address(reference r) { return &r; }
        inline const_pointer address(const_reference r) { return &r; }

        //    memory allocation
        inline pointer allocate(size_type cnt, 
            typename std::allocator<void>::const_pointer = 0) { 
            void* ptr = PathFD::AllocSmall(cnt * sizeof(T));
            if (!ptr) throw(std::bad_alloc());
            return reinterpret_cast<pointer>(ptr); 
        }
        inline void deallocate(pointer p, size_type) {
            PathFD::FreeSmall(p);
        }

        //    size
        inline size_type max_size() const { 
            return std::numeric_limits<size_type>::max() / sizeof(T);
        }

        //    construction/destruction
        inline void construct(pointer p, const T& t) { new(p) T(t); }
        inline void destroy(pointer p) { p->~T(); }

        inline bool operator==(Allocator const&) { return true; }
        inline bool operator!=(Allocator const& a) { return !operator==(a); }

    };
    // A*算法
    class CFDAStar final : public IFDAlgorithm {
    public:
        // 节点
        struct NODE {
            // 坐标
            int16_t         x, y;
            // 节点深度(gn)
            int16_t         gn;
            // 节点价值(fx)
            int16_t         fx;
            // 父节点
            const NODE*     parent;
        };
        // 列表
        using List = std::list<CFDAStar::NODE, Allocator<CFDAStar::NODE>>;
        // 操作
        using StepOp = impl::astar_step_op<CFDAStar>;
    public:
        // 构造函数
        CFDAStar() noexcept;
        // 释放对象
        void Dispose() noexcept override { delete this; };
        // 执行算法, 返回路径(成功的话), 需要调用者调用std::free释放
        auto Execute(const PathFD::Finder& fd) noexcept->Path* override;
        // 可视化步进
        void BeginStep(const PathFD::Finder& fd) noexcept override;
        // 可视化步进
        bool NextStep(void* cells) noexcept override;
        // 结束可视化步进
        void EndStep() noexcept override;
    public:
        // 退出
        bool IsExit() const { return m_bExit; }
        // 完成
        void SetFinished() { m_bFinished = true; }
    private:
        // 析构函数
        ~CFDAStar() noexcept;
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
    };
}


/// <summary>
/// 创建A*算法
/// </summary>
/// <returns>A*算法接口</returns>
auto PathFD::CreateAStarAlgorithm() noexcept -> IFDAlgorithm* {
    return new(std::nothrow) PathFD::CFDAStar;
}


/// <summary>
/// <see cref="CFDAStar"/> 类构造函数
/// </summary>
PathFD::CFDAStar::CFDAStar() noexcept : m_opStep(*this) {

}


// pathfd::impl 命名空间
namespace PathFD { namespace impl {
    // 找到路径
    auto path_found(PathFD::CFDAStar::List& close_list) noexcept {
        assert(!close_list.empty());
        size_t size = size_t(close_list.front().gn);
#ifdef _DEBUG
        size_t debug_count = 0;
        {
            const auto* itr = &close_list.front();
            while (itr->parent) {
                ++debug_count;
                itr = itr->parent;
            }
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
                // 遍历检查
                const auto* itr = &close_list.front();
                while (itr->parent) {
                    --pp;
                    pp->x = itr->x;
                    pp->y = itr->y;
                    itr = itr->parent;
                }
            }
            return path;
        }
    }
    // 寻找路径
    auto a_star_find(const PathFD::Finder& fd) -> PathFD::Path* {
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
        // 估值函数 f(n)=g(n)+h(n)
        auto hn = [=](int16_t x, int16_t y) noexcept -> int16_t {
#if 0
            auto xxx = std::abs(x - gx);
            auto yyy = std::abs(y - gy);
            auto maxone = std::max(xxx, yyy);
            auto minone = std::min(xxx, yyy);
            return minone * 3 + (maxone - minone) * 2;
#else
            return std::abs(x - gx) + std::abs(y - gy);
#endif
        };
        // 检查通行
        auto cp_ptr = fd.data; int16_t cp_width = fd.width; int16_t cp_height = fd.height;
        auto check_pass = [cp_ptr, cp_width, cp_height](int16_t x, int16_t y) noexcept {
            if (x >= 0 && x < cp_width && y >= 0 && y < cp_height) {
                return !!cp_ptr[x + y * cp_width];
            }
            return false;
        };
        // 起点终点数据
        CFDAStar::NODE start, end; 
        start.x = sx; start.y = sy;
        start.gn = 0;
        start.fx = hn(start.x, start.y);
        start.parent = nullptr;
        end.x = gx; end.y = gy;
        // 起点加入OPEN表
        CFDAStar::List open, close; open.push_back(start);
        mark_visited(start.x, start.y);
        // 为空算法失败
        while (!open.empty()) {
            // 从表头取一个结点 添加到CLOSE表
            close.push_front(open.front());
            // 弹出
            open.pop_front();
            // 获取
            const auto& node = close.front();
            // 目标解
            if (node.x == end.x && node.y == end.y) {
                return impl::path_found(close);
            }
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
                    // 比最后的都大?
                    if (open.empty() || tmp.fx >= open.back().fx) {
                        // 添加到最后
                        open.push_back(tmp);
                        return;
                    }
                    // 添加节点
                    for (auto itr = open.begin(); itr != open.end(); ++itr) {
                        if (tmp.fx < itr->fx) {
                            open.insert(itr, tmp);
                            return;
                        }
                    }
                    // 不可能
                    assert(!"Impossible ");
                }
            };
            // 南
            moveto( 0,+1);
            // 西
            moveto(-1, 0);
            // 东
            moveto(+1, 0);
            // 北
            moveto( 0,-1);
        }
        return nullptr;
    }
    // 寻找路径ex
    auto a_star_find_ex(CFDAStar::StepOp& op, const PathFD::Finder& fd) {
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
        // 估值函数 f(n)=g(n)+h(n)
        auto hn = [=](int16_t x, int16_t y) noexcept -> int16_t {
#if 0
            auto xxx = std::abs(x - gx);
            auto yyy = std::abs(y - gy);
            auto maxone = std::max(xxx, yyy);
            auto minone = std::min(xxx, yyy);
            return minone * 3 + (maxone - minone) * 2;
#else
            return std::abs(x - gx) + std::abs(y - gy);
#endif
        };
        // 检查通行
        auto cp_ptr = fd.data; int16_t cp_width = fd.width; int16_t cp_height = fd.height;
        auto check_pass = [cp_ptr, cp_width, cp_height](int16_t x, int16_t y) noexcept {
            if (x >= 0 && x < cp_width && y >= 0 && y < cp_height) {
                return !!cp_ptr[x + y * cp_width];
            }
            return false;
        };
        // 起点终点数据
        CFDAStar::NODE start, end; 
        start.x = sx; start.y = sy;
        start.gn = 0;
        start.fx = hn(start.x, start.y);
        start.parent = nullptr;
        end.x = gx; end.y = gy;
        // 起点加入OPEN表
        CFDAStar::List open, close; open.push_back(start);
        mark_visited(start.x, start.y);
        // 为操作设置表数据
        op.set_open_list(open); op.set_close_list(close);
        // 为空算法失败
        while (!open.empty() && op.go_on()) {
            // 加锁
            op.lock();
            // 从表头取一个结点 添加到CLOSE表
            close.push_front(open.front());
            // 事件处理
            op.get_node(close.front());
            // 弹出
            open.pop_front();
            // 获取
            const auto& node = close.front();
            // 解锁
            op.unlock();
            // 目标解
            if (node.x == end.x && node.y == end.y) {
                op.found(close);
                return;
            }
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
                    // 加锁
                    op.lock();
                    // 比最后的都大?
                    if (open.empty() || tmp.fx >= open.back().fx) {
                        // 添加到最后
                        open.push_back(tmp);
                        // 解锁
                        op.unlock();
                        return;
                    }
                    // 添加节点
                    for (auto itr = open.begin(); itr != open.end(); ++itr) {
                        if (tmp.fx < itr->fx) {
                            open.insert(itr, tmp);
                            // 解锁
                            op.unlock();
                            return;
                        }
                    }
                    // 不可能
                    assert(!"Impossible ");
                }
            };
            // 南
            moveto( 0,+1);
            // 西
            moveto(-1, 0);
            // 东
            moveto(+1, 0);
            // 北
            moveto( 0,-1);
            // 等待一步
            op.wait();
        }
        return op.failed();
    }
}}


// 执行算法
auto PathFD::CFDAStar::Execute(const PathFD::Finder& fd) noexcept -> PathFD::Path* {
    // 正式处理
    try { return impl::a_star_find(fd);  }
    // 出现异常
    catch (...) { return nullptr; }
}

// 可视化步进
void PathFD::CFDAStar::BeginStep(const PathFD::Finder& fd) noexcept {
    assert(m_thdStep.joinable() == false);
    std::this_thread::yield();
    m_fdData = fd;
    try {
        auto& data = m_fdData;
        auto& op = m_opStep;
        m_thdStep.std::thread::~thread();
        m_thdStep.std::thread::thread([&op, &data]() {
            impl::a_star_find_ex(op, data);
        });
    }
    catch (...) { m_opStep.failed(); }
}
// 可视化步进
bool PathFD::CFDAStar::NextStep(void* cells) noexcept {
    assert(cells && "bad pointer");
    // 结束就提前返回
    if (m_bFinished) return true;
    //auto count = m_fdData.width * m_fdData.height;
    impl::color red;
    red.r = 1.f; red.g = 0.f; red.b = 0.f; red.a = 1.f;
    impl::color green;
    green.r = 0.f; green.g = 1.f; green.b = 0.f; green.a = 1.f;
    impl::color white;
    white.r = white.g = white.b = white.a = 1.4f;
    // 读取数据
    m_opStep.lock();
    {
        assert(m_opStep.openlist && m_opStep.closelist);
        // 为OPEN表添加红色
        for (const auto& node : (*m_opStep.openlist)) {
            uint32_t index = node.x + node.y * m_fdData.width;
            impl::set_cell_color(cells, index, red);
        }
        // 为CLOSE表添加绿色
        for (const auto& node : (*m_opStep.closelist)) {
            uint32_t index = node.x + node.y * m_fdData.width;
            impl::set_cell_color(cells, index, green);
        }
        // 为OPEN表头添加蓝色
        if (!m_opStep.openlist->empty()) {
            const auto& node = m_opStep.openlist->front();
            uint32_t index = node.x + node.y * m_fdData.width;
            impl::set_cell_color(cells, index, white);
        }
    }
    // 解数据访问锁
    m_opStep.unlock();
    // 提示下一步
    m_opStep.signal();
    return false;
}

// 结束可视化步进
void PathFD::CFDAStar::EndStep() noexcept {
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
PathFD::CFDAStar::~CFDAStar() noexcept {
    m_opStep.signal();
    m_bExit = true;
    if (m_thdStep.joinable()) m_thdStep.join();
}
