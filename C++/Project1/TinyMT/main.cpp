#include <cstdint>
#include "tinymt32.h"

#include <random>
#include <cstdio>
#include <algorithm>


class TinyMT {
public:
    TinyMT(uint32_t seed) noexcept;
    // 0 <= r < 2^32
    auto operator()() noexcept->uint32_t;
    // 0.0 <= r < 1.0
    auto operator()(float) noexcept { return this->gf(); }
private:
    auto gf() noexcept ->float;

    tinymt32_t  m_state;
};

TinyMT::TinyMT(uint32_t seed) noexcept {
    ::tinymt32_init(&m_state, seed);
}

auto TinyMT::operator()() noexcept -> uint32_t {
    return ::tinymt32_generate_uint32(&m_state);
}

auto TinyMT::gf() noexcept -> float {
    return ::tinymt32_generate_float(&m_state);
}


int main() {
    std::random_device dev;

    TinyMT ra(dev());

    constexpr int count = 131;
    constexpr int round = 100'0000;
    uint32_t buffer[count] = { 0 };

    for (int i = 0; i != round * count; ++i) {
        ++buffer[ra() % count];
    }

    std::sort(std::begin(buffer), std::end(buffer));
    
    const auto a = int(buffer[0]);
    const auto b = int(std::end(buffer)[-1]);
    const auto c = (a + b) / 2;

    std::printf("%d - %d [ %d ~ %d ] \n", a, b, c, round);


    // 模拟计算圆周率
    constexpr int pi_round = 1000'0000;
    int pi_count = 0;
    for (int i = 0; i != pi_round; ++i) {
        const auto x = ra({});
        const auto y = ra({});
        if (x*x + y * y <= 1.f) ++pi_count;
    }
    const double this_pi = double(pi_count) / double(pi_round) * 4;
    std::printf("pi ~= %lf\n", this_pi);

    std::getchar();
}
