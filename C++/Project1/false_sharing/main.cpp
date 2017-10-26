#include <thread>
#include <chrono>

enum {
    int_len = std::hardware_destructive_interference_size / sizeof(int)
};

struct alignas(int_len * sizeof(int)) Data { int array[int_len + 1]; };

int main(int argc, const char* argv[]) {
    Data a = { 0 };

    void(*const ptr)(int* ptr) noexcept = [](int* ptr) noexcept { (*ptr)++;  };
    void(*const foo)(int* ptr) noexcept = [](int* ptr) noexcept { };
    const auto func = argc ? ptr : foo;
    const auto fake = argc ? foo : ptr;

    constexpr int loop_count = 5'0000'0000;

    const auto call = [&, loop_count, func](int index) {
        for (int i = 0; i != loop_count; ++i) {
            func(a.array + index);
        }
    };

    const auto t0 = std::chrono::high_resolution_clock::now();
    {
        std::thread thread{ call, 0 };
        call(1);
        thread.join();
    }
    const auto t1 = std::chrono::high_resolution_clock::now();
    {
        std::thread thread{ call, 0 };
        call(int_len);
        thread.join();
    }
    const auto t2 = std::chrono::high_resolution_clock::now();
    {
        const auto empty = [&, fake]() noexcept {
            for (int i = 0; i != loop_count; ++i) {
                fake(a.array + 0);
            }
        };
        std::thread thread{ empty};
        empty;
        thread.join();
    }
    const auto t3 = std::chrono::high_resolution_clock::now();

    const std::chrono::duration<double> d1 = t1 - t0;
    const std::chrono::duration<double> d2 = t2 - t1;
    const std::chrono::duration<double> d3 = t3 - t2;


    std::printf("%lf vs %lf ( empty: %lf)\n", d1.count(), d2.count(), d3.count());
    std::getchar();
}