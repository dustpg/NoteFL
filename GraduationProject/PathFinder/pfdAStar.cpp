#include "pfdAlgorithm.h"
#include "pdfImpl.h"

#include <algorithm>
#include <cassert>
#include <atomic>
#include <thread>
#include <memory>
#include <vector>
#include <queue>
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
            // 表
            using List = typename T::List;
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
                nodex = node.x;
                nodey = node.y;
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
            const List*         openlist = nullptr;
            // CLOSE表
            const List*         closelist = nullptr;
            // 选择节点X
            uint32_t            nodex = 0;
            // 选择节点Y
            uint32_t            nodey = 0;
        };
        // 操作类
        struct astar_op {
            // 设置OPEN表
            template<typename Y> inline void set_open_list(Y& list) {  }
            // 设置CLOSE表
            template<typename Y> inline void set_close_list(Y& list) { }
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
    // A*算法
    class CFDAStar final : public IFDAlgorithm {
    public:
        // 节点
        struct alignas(void*) NODE {
            // 节点深度(gn)
            uint16_t        gn;
            // 节点价值(fn)
            uint16_t        fn;
            // 坐标
            int16_t         x, y;
            // 父节点方向
            int16_t         px,py;
            // 未使用变量
            uint32_t        unused;
            // 相等判断
            bool operator==(const NODE& node) {
                return this->x == node.x && this->y == node.y;
            }
        };
        // 操作
        using StepOp = impl::astar_step_op<CFDAStar>;
        // 线性表
        using List = std::vector<NODE>;
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
        // 可视化阶段
        uint8_t                 m_uPhase = 0;
        // 8方向
        bool                    m_unused;
    };
}


// pathfd::impl 命名空间
namespace PathFD { namespace impl {
    // 节点比较操作
    struct node_comp {
        // 节点
        using node = PathFD::CFDAStar::NODE;
        // () 运算符
        inline bool operator()(const node& a, const node& b) const noexcept {
            return a.fn > b.fn;
        }
    };
    // A* 算法适用二叉堆
    class astar_bheap /*: public std::priority_queue<
        PathFD::CFDAStar::NODE, 
        PathFD::CFDAStar::List, 
        node_comp>*/ {
        // 节点
        using node = PathFD::CFDAStar::NODE;
        // 线性表
        using vector = PathFD::CFDAStar::List;
    public:
        // 构造函数
        astar_bheap()  {  }
        // 获取数组
        const auto& get_c() const { return c; }
#if 1
        // 检查是否为空
        auto empty() const { return c.empty(); }
        // 获取最顶端数据
        auto&top() const { return c.front(); }
        // 添加元素
        void push(const node& data) {
            c.push_back(data); std::push_heap(c.begin(), c.end(), comp);
        }
        // 删除元素
        void pop() {
            assert(!c.empty()); std::pop_heap(c.begin(), c.end(), comp); c.pop_back();
        }
        // A* 提升某节点
        void astar_elevate(vector::iterator itr, const node& data) {
            assert(itr != c.end() && comp(*itr, data)); *itr = data; 
            std::push_heap(c.begin(), itr + 1, comp);
        }
        // 查找数据位置
        auto find(const node& data) { return std::find(c.begin(), c.end(), data); }
#endif
    protected:
        // 大于操作
        node_comp           comp;
        // 线性表
        vector              c;
    };
}}

/// <summary>
/// 创建A*算法
/// </summary>
/// <returns>A*算法接口</returns>
auto PathFD::CreateAStarAlgorithm() noexcept -> IFDAlgorithm* {
    return new(std::nothrow) PathFD::CFDAStar();
}


/// <summary>
/// <see cref="CFDAStar"/> 类构造函数
/// </summary>
PathFD::CFDAStar::CFDAStar() noexcept : m_opStep(*this){

}


// pathfd::impl 命名空间
namespace PathFD { namespace impl {
    // 找到路径
    auto path_found(const PathFD::CFDAStar::List& list) noexcept {
        assert(!list.empty());
        // 包括起始点
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
        size_t size = 0;
        {
            auto itr = list.crbegin();
            do { ++size; } while (find_node(itr) != noop);
        }
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
    // 创建节点
    template<int16_t xplus, int16_t yplus, uint16_t gplus, typename T, typename Y, typename U> 
    auto moveto(const CFDAStar::NODE& node, T insert2, Y check_pass, U hn)  {
        CFDAStar::NODE tmp;
        // 计算节点位置
        tmp.x = node.x + xplus; tmp.y = node.y + yplus;
        // 可以通行 
        if (!check_pass(tmp.x, tmp.y)) return;
        // 记录父节点位置
        tmp.px = -xplus; tmp.py = -yplus;
        // 计算g(n)
        tmp.gn = node.gn + gplus;
        // f(n) = g(n) + h(n)
        tmp.fn = tmp.gn + hn(tmp.x, tmp.y);
        // 插入
        insert2(tmp);
    };
    // 寻找路径ex
    template<typename OP>
    auto a_star_find_ex(OP& op, const PathFD::Finder& fd) {
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
        auto visited = std::make_unique<uint16_t[]>(fd.width * fd.height);
        // 标记需要数据
        auto mk_ptr = visited.get(); int16_t mk_width = fd.width;
        // 标记遍历
        auto mark_visited = [mk_ptr, mk_width](int16_t x, int16_t y, uint16_t fn) noexcept {
            mk_ptr[x + y * mk_width] = fn;
        };
        // 检查标记
        auto check_visited = [mk_ptr, mk_width](int16_t x, int16_t y) noexcept {
            return mk_ptr[x + y * mk_width];
        };
        // 检查OPEN表
        auto check_openbelow = [mk_ptr, mk_width](int16_t x, int16_t y, uint16_t fn) noexcept {
            auto v = mk_ptr[x + y * mk_width];
            return fn < v;
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
            return (std::abs(x - gx) + std::abs(y - gy)) * 2; 
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
        // 起点数据
        CFDAStar::NODE start; 
        start.x = sx; start.y = sy;
        start.gn = 0; start.fn = hn(start.x, start.y);
        start.px = 0; start.py = 0;
        // 终点数据
        struct { decltype(start.x) x, y; } end;
        end.x = gx; end.y = gy;
        // 起点加入OPEN表
        CFDAStar::List close;
        impl::astar_bheap open;
        open.push(start);
        mark_visited(start.x, start.y, start.fn);
        // 为操作设置表数据
        op.set_open_list(open.get_c()); op.set_close_list(close);
        // 解锁
        op.unlock();
        // 插入节点
        auto insert2 = [&](const CFDAStar::NODE& node) {
            // 比OPEN表的低
            if (check_openbelow(node.x, node.y, node.fn)) {
                // 标记
                mark_visited(node.x, node.y, node.fn);
                // 加锁
                impl::auto_locker<OP> locker(op);
                // 删除旧的
                open.astar_elevate(open.find(node), node);
                // 返回
                return;
            }
            // 并且没有遍历过
            if (!check_visited(node.x, node.y)) {
                // 标记
                mark_visited(node.x, node.y, node.fn);
                // 加锁
                impl::auto_locker<OP> locker(op);
                // 添加
                open.push(node);
            }
        };
        // 为空算法失败
        while (!open.empty() && op.go_on()) {
            // 加锁
            op.lock();
            // 从表头取一个结点 添加到CLOSE表
            close.push_back(open.top());
            // 事件处理
            op.get_node(close.back());
            // 弹出
            open.pop();
            // 获取
            const auto& node = close.back();
            // 解锁
            op.unlock();
            // 目标解
            if (node.x == end.x && node.y == end.y) {
                // 加锁
                impl::auto_locker<OP> locker(op);
                // 返回
                return op.found(close);
            }
            // 标记在CLOSE表中
            mark_visited(node.x, node.y, 1);
            // 强行手动内联,提高10%性能
            // 南
            moveto< 0,+1, 2>(node, insert2, check_pass, hn);
            // 西
            moveto<-1, 0, 2>(node, insert2, check_pass, hn);
            // 东
            moveto<+1, 0, 2>(node, insert2, check_pass, hn);
            // 北
            moveto< 0,-1, 2>(node, insert2, check_pass, hn);
            // 8方向
            if (direction8) {
                // 东南
                moveto<-1,+1, 3>(node, insert2, check_pass, hn);
                // 西南
                moveto<+1,+1, 3>(node, insert2, check_pass, hn);
                // 东北
                moveto<-1,-1, 3>(node, insert2, check_pass, hn);
                // 西北
                moveto<+1,-1, 3>(node, insert2, check_pass, hn);
            }
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
auto PathFD::CFDAStar::Execute(const PathFD::Finder& fd) noexcept -> PathFD::Path* {
    // 正式处理
    try { impl::astar_op op; return impl::a_star_find_ex(op, fd); }
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
bool PathFD::CFDAStar::NextStep(void* cells, void* num, bool refresh) noexcept {
    assert(cells && "bad pointer");
    // 阶段0: 显示
    if (m_uPhase == 0) {
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
            assert(m_opStep.openlist && m_opStep.closelist);
            static uint32_t s_index;
            s_index = 0;
            // 显示节点数据
            auto nodedisplay = [num](const NODE& node) noexcept {
                constexpr uint32_t COUNT = 3;
                char buffer[sizeof(NodeDisplay) + sizeof(uint32_t) * COUNT];
                auto& numdis = reinterpret_cast<NodeDisplay&>(*buffer);
                // 设置
                numdis.x = node.x;
                numdis.y = node.y;
                numdis.i = s_index++;
                numdis.argc = COUNT;
                // 节点
                numdis.d = PathFD::CaculateDirection(node.px, node.py);
                // fn = 
                numdis.argv[0] = node.fn;
                // gn = 
                numdis.argv[1] = node.gn;
                // hn = 
                numdis.argv[2] = node.fn - node.gn;
                // 显示数字
                impl::set_node_display(num, &numdis);
            };
            // 为OPEN表添加红色
            for (const auto& node : (*m_opStep.openlist)) {
                uint32_t index = node.x + node.y * m_fdData.width;
                impl::set_cell_color(cells, index, red);
                nodedisplay(node);
            }
            // 为CLOSE表添加绿色
            for (const auto& node : (*m_opStep.closelist)) {
                uint32_t index = node.x + node.y * m_fdData.width;
                impl::set_cell_color(cells, index, green);
                nodedisplay(node);
            }
            // 解数据访问锁
            m_opStep.unlock();
        }
        // 提示下一步
        m_opStep.signal();
    }
    else {
        if (refresh) {
            impl::color blue;
            blue.r = 1.f; blue.g = 1.f;  blue.b = 2.f; blue.a = 1.0f;
            uint32_t index = m_opStep.nodex + m_opStep.nodey * m_fdData.width;
            impl::set_cell_color(cells, index, blue);
        }
    }
    // 更换阶段
    if (refresh) {
        m_uPhase = !m_uPhase;
    }
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

