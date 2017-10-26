#define _CRT_SECURE_NO_WARNINGS
#include <cmath>
#include <cstdio>
#include <chrono>
#include <random>
#include <functional>

namespace my {
    inline float hypot(float x, float y) noexcept {
        return std::sqrt(x*x + y*y);
    }
}

int main() {
    std::random_device rd;
    std::mt19937 mt(rd());
    constexpr size_t count = 1'000'0000;
    size_t sum = 0;
    for (size_t i = 0; i != count; ++i) {
        size_t m = mt() & 1;
        if (!m) m = mt() & 1;
        sum += m;
    }
    std::printf("%lf\n", double(sum) / double(count));
    std::getchar();
}