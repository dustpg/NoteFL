#include <chrono>
#include <random>
#include <algorithm>

#include <cstdio>
#include <cstdlib>


inline bool is_0_or_1_v1(int a) noexcept {
    return a == 0 || a == 1;
}

inline bool is_0_or_1_v2(int a) noexcept {
    return ((a) >> 1) == 0;
}


int main() {
    constexpr size_t count = 100'0000;
    int buffer[1024];
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        for (auto&x : buffer) x = mt() % 3;
    }
    int a = 0, b = 0, c = 0, d= 0;

    const auto tp0 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i != count; ++i) {
        for (auto x : buffer) a += is_0_or_1_v1(x);
    }
    const auto tp1 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i != count; ++i) {
        for (auto x : buffer) b += is_0_or_1_v2(x);
    }
    const auto tp2 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i != count; ++i) {
        for (auto x : buffer) {
            c += is_0_or_1_v1(x);
            d += is_0_or_1_v2(x);
        }
    }
    const auto tp3 = std::chrono::high_resolution_clock::now();

    const std::chrono::duration<double> td1 = tp1 - tp0;
    const std::chrono::duration<double> td2 = tp2 - tp1;
    const std::chrono::duration<double> td3 = tp3 - tp2;

    const auto round0012 = td1.count() + td2.count();
    const auto round_012 = td3.count();
    const auto round___0 = round0012 - round_012;
    const auto round___1 = td1.count() - round___0;
    const auto round___2 = td2.count() - round___0;

    std::printf("a = %d round: %.3fs\n", a, round___1);
    std::printf("b = %d round: %.3fs\n", b, round___2);
    std::printf("c-d = %d, ALL took: %.3fs\n", c - d, td3.count());
    std::getchar();
}