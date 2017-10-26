#include <functional>
#include <thread>
#include <chrono>
#include <new>

namespace test {
    enum : std::size_t {
        abcd = std::hardware_constructive_interference_size,
        len = abcd / sizeof(int)
    };
    struct alignas(abcd) a  {
        int ary[len + 1];
    } ;
}



int main() {
    test::a a;
    const auto call = [&a](std::size_t len, int index) noexcept {
        for(size_t i = 0; i != len; ++ i) a.ary[index]++;
    };
    constexpr size_t count = 20'0000'0000ll;

    const auto t0 = std::chrono::system_clock::now();

    std::thread thread{ call, count, test::len };

    std::function<void(std::size_t len, int index)>{ call } (count, 0);

    thread.join();

    const auto t1 = std::chrono::system_clock::now();

    const std::chrono::duration<double> d0 = t1 - t0;

    std::printf("time: +%lfs\n", d0.count());
    std::getchar();
    return 0;
}