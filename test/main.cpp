#include <iostream>

enum XX {
    XX_1st = 0,
    XX_2nd,
    XX_SIZE,
};

class XXHelper {
public:
    static constexpr auto GetCount() { return static_cast<uint32_t>(XX_SIZE); }
};

class XXXHelper {
public:
    static constexpr auto GetCount() { return static_cast<uint32_t>(XX_SIZE) +1; }
};


template<class T>
class TT {
public:
    int     data[T::GetCount()];
};

// main
int main(int argc, const char* argv[]) {
    TT<XXHelper> tta;
    TT<XXXHelper> ttb;
    constexpr int ttaa = sizeof(tta);
    constexpr int ttbb = sizeof(ttb);
    constexpr bool aaa = ttaa == ttbb;
    (void)std::getchar();
    return EXIT_SUCCESS;
}