#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>


// 应用程序入口
int main(int argc, const char* argv[]) {
    std::string info = "infomation";
    int bbb = 100;
    const int a = bbb;
    const_cast<int&>(a) = 255;
    int* p = const_cast<int*>(&a);
    *p = 200;
    std::cout << &a << '[' 
        << *(&a) << ']' << ' ' << p << '[' << *p << ']' << std::endl;
    //
    std::printf("%p[%d] %p[%d]", &a, *(&a), p, *p);
    (void)std::getchar();
    return EXIT_SUCCESS;
}