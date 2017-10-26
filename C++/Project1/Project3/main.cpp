#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <cstdio>
#include <optional>

namespace detail::impl {
    void clear_input() noexcept {
        int c; while ((c = std::getchar()) != '\n' && c != EOF);
    }
}

template<class T> constexpr T pi = T(3.1415926535897932385);

struct AAA {
    ~AAA() {
        std::printf("%s\n", __FUNCTION__);
    }
};

int main() {
    {
        std::optional<AAA> result;
        result.emplace();
    }
    float value = pi<float>;
    value = std::nextafter(value, 0.f);
    decltype(auto) a = value;
    std::scanf("%f", &value);
    value = std::clamp(value, 0.f, 1.f);
    std::printf("%f", value);
    detail::impl::clear_input();
    std::getchar();
    return 0;
}