#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

struct TEST {
    int x, y, z;
};

// 应用程序入口
int main(int argc, const char* argv[]) {
    std::string info = "infomation";
    std::cout << 0.f;
    std::cout << info << std::endl;
    //
    TEST test1 = { 0, 1, 2 };
    // C99
    //TEST test2 = { .y = 1,.z = 2 };
    (void)std::getchar();
    return EXIT_SUCCESS;
}