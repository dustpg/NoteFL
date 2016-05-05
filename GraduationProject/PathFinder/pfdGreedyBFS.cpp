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

贪心最佳搜索算法流程：

首先将起始结点S放入OPEN表，CLOSE表置空，算法开始时：
1、如果OPEN表不为空，从表头取一个结点n，如果为空算法失败。

2、n是目标解吗？是，找到一个解（继续寻找，或终止算法）。

3、将n的所有后继结点展开，就是从n可以直接关联的结点（子结点），
如果不在CLOSE表中，就将它们放入OPEN表，并把S放入CLOSE表，
同时计算每一个后继结点的启发值h(n)，将OPEN表按h(x)排序，
最小的放在表头，重复算法，回到1。
*/

// pathfd 命名空间
namespace PathFD {
    // impl
    namespace impl {
        // 步进操作类
        template<typename T> struct gbfs_step_op {
            // 列表
            using List = typename T::Vector;
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
            gbfs_step_op(T& parent) : parent(parent) {}
            // 构造本类函数
            ~gbfs_step_op() { impl::destroy(ev); impl::destroy(mx);}
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
        struct gbfs_op {
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
    // GreedyBFS算法
    class CFDGreedyBFS final : public IFDAlgorithm {
    public:
        // 节点
        struct NODE {
            // 节点启发值(hn)
            uint16_t        hn;
            // 坐标
            int16_t         x, y;
            // 父节点
            int8_t          px, py;
        };
        // 小于
        struct Less {
            // () 运算
            bool operator()(const NODE& a, const NODE& b) noexcept {
                return a.hn > b.hn;
            }
        };
        // 列表
        using Vector = std::vector<CFDGreedyBFS::NODE>;
        // 优先队列
        struct Queue : std::priority_queue<CFDGreedyBFS::NODE, Vector, Less> {
            using Super = std::priority_queue<CFDGreedyBFS::NODE, Vector, Less>;
            // 构造函数
            Queue(const Vector& c) : Super(Less(), c) {  }
            // 获取容器
            const auto& get_c() const { return this->c; }
        };
        // 操作
        using StepOp = impl::gbfs_step_op<CFDGreedyBFS>;
    public:
        // 构造函数
        CFDGreedyBFS() noexcept;
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
        ~CFDGreedyBFS() noexcept;
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


/// <summary>
/// 创建A*算法
/// </summary>
/// <returns>A*算法接口</returns>
auto PathFD::CreateGreedyBFSAlgorithm() noexcept -> IFDAlgorithm* {
    return new(std::nothrow) PathFD::CFDGreedyBFS();
}


/// <summary>
/// <see cref="CFDGreedyBFS"/> 类构造函数
/// </summary>
PathFD::CFDGreedyBFS::CFDGreedyBFS() noexcept : m_opStep(*this) {

}


// pathfd::impl 命名空间
namespace PathFD { namespace impl {
    // 找到路径
    auto path_found(const PathFD::CFDGreedyBFS::Vector& list) noexcept {
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
    // 寻找路径ex
    template<typename OP>
    auto gbfs_find_ex(OP& op, const PathFD::Finder& fd) {
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
        // 估值函数 h(n)
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
        // 起点数据
        CFDGreedyBFS::NODE start; 
        start.x = sx; start.y = sy;
        start.hn = hn(start.x, start.y);
        start.px = 0; start.py = 0;
        // 终点数据
        struct { decltype(start.x) x, y; } end;
        end.x = gx; end.y = gy;
        // OPEN表(优先队列) CLOSE表(线性表)
        CFDGreedyBFS::Vector close;
        constexpr auto reserved = 4096;
        close.reserve(reserved);
        CFDGreedyBFS::Queue open(close);
        // 起点加入OPEN表
        open.push(start);
        mark_visited(start.x, start.y);
        // 为操作设置表数据
        op.set_open_list(open.get_c()); op.set_close_list(close);
        // 解锁
        op.unlock();
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
            // 移动
            auto moveto = [&](int8_t xplus, int8_t yplus) {
                CFDGreedyBFS::NODE tmp; 
                tmp.x = node.x + xplus; 
                tmp.y = node.y + yplus; 
                // 可以通行 并且没有遍历过
                if (check_pass(tmp.x, tmp.y) && !check_visited(tmp.x, tmp.y)) {
                    // 标记
                    mark_visited(tmp.x, tmp.y);
                    // 记录父节点位置
                    tmp.px = -xplus;
                    tmp.py = -yplus;
                    // 计算h(n)
                    tmp.hn = hn(tmp.x, tmp.y);
                    // 加锁
                    impl::auto_locker<OP> locker(op);
                    // 添加
                    open.push(tmp);
                }
            };
            // 东南西北
            moveto( 0,+1); moveto(-1, 0); moveto(+1, 0); moveto( 0, -1);
            // 8方向
            if (direction8) { moveto(-1,+1); moveto(+1,+1); moveto(-1,-1); moveto(+1,-1); }
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
auto PathFD::CFDGreedyBFS::Execute(const PathFD::Finder& fd) noexcept -> PathFD::Path* {
    // 正式处理
    try { impl::gbfs_op op; return impl::gbfs_find_ex(op, fd); }
    // 出现异常
    catch (...) { return nullptr; }
}

// 可视化步进
void PathFD::CFDGreedyBFS::BeginStep(const PathFD::Finder& fd) noexcept {
    assert(m_thdStep.joinable() == false);
    std::this_thread::yield();
    m_fdData = fd;
    try {
        auto& data = m_fdData;
        auto& op = m_opStep;
        m_thdStep.std::thread::~thread();
        m_thdStep.std::thread::thread([&op, &data]() {
            impl::gbfs_find_ex(op, data);
        });
    }
    catch (...) { m_opStep.failed(); }
}
// 可视化步进
bool PathFD::CFDGreedyBFS::NextStep(void* cells, void* num, bool refresh) noexcept {
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
                // hn = 
                numdis.argv[0] = node.hn;
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
    m_uPhase = !m_uPhase;
    return false;
}

// 结束可视化步进
void PathFD::CFDGreedyBFS::EndStep() noexcept {
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
PathFD::CFDGreedyBFS::~CFDGreedyBFS() noexcept {
    m_opStep.signal();
    m_bExit = true;
    if (m_thdStep.joinable()) m_thdStep.join();
}
