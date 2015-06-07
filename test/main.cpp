#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

class Base {
public:
    Base() { 
        for (auto& i : m_base) 
            i = 0; 
    }
    Base(int base) { 
        for (auto& i : m_base) 
            i = base; 
    }
protected:
    int     m_base[128];
};

class Dase1 : public virtual Base {
public:
    Dase1(int base) : m_dase1(base), Base(base){}
private:
    int     m_dase1;
};


class Dase2 : public virtual Base {
public:
    Dase2(int base) : m_dase2(base), Base(base) {}
private:
    int     m_dase2;
};

class Fase : public Dase1, public Dase2 {
public:
    Fase(int base) : Dase1(base), Dase2(base), m_fase(base) {}
private:
    int     m_fase;
};


// 应用程序入口
int main(int argc, const char* argv[]) {
    {
        Fase fase(9);
    }
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