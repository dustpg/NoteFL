#include <iostream>

class DDD {
public:
    DDD() = default;
    int a = 0;
};


class MyInterface {
    friend class DDD;
public:
    virtual void Release() noexcept = 0;
};

class CCC : private MyInterface {
    virtual void Release() noexcept override { int a = 9; };
public:
    int ccc;
};



int main() {
    CCC c;
    return EXIT_SUCCESS;
}