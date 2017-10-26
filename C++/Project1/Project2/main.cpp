#include <chrono>
#include <vector>
#include <map>
#include <unordered_map>

enum : size_t {
    len = 15,
    count = 1000000,
};

int main() {
    std::map<int, int> a;
    std::unordered_map<int, int> b;
    std::vector<int> c;
    c.resize(len);
    for (size_t i = 0; i != len; ++i) {
        a[i] = i;
        b[i] = i;
        c[i] = i;
    }

    auto func = [](auto& x) {
        for (size_t i = 0; i != count; ++i) {
            for (size_t j = 0; j != len; ++j) {
                x[j] = j;
            }
        }
    };

    const auto t0 = std::chrono::high_resolution_clock::now();
    func(a);
    const auto t1 = std::chrono::high_resolution_clock::now();
    func(b);
    const auto t2 = std::chrono::high_resolution_clock::now();
    func(c);
    const auto t3 = std::chrono::high_resolution_clock::now();

    using dur_t = std::chrono::duration<double>;
    const dur_t d1 = t1 - t0;
    const dur_t d2 = t2 - t1;
    const dur_t d3 = t3 - t2;

    std::printf("%lf  vs  %lf  vs  %lf\n", d1.count(), d2.count(), d3.count());
    std::getchar();
}
