#include <cstdlib>
#include <cstdio>

// 应用程序入口
int main(int argc, const char* argv[]) {
    auto pointer = ::_aligned_malloc(256, 256);
    ::_aligned_free(pointer);
    (void)std::getchar();
    return EXIT_SUCCESS;
}