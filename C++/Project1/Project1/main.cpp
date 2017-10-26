#include <cstdio>
#include <type_traits>


struct A {
    virtual void func() {
        std::printf("%s\n", __FUNCTION__);
    }
};

struct B : A {
    virtual void func() override {
        std::printf("%s\n", __FUNCTION__);
    }
};


void call(A& a, void(A::*ptr)()) {
    (a.*ptr)();
}


struct node {
    int a;
};


constexpr int func() {
    constexpr int a = std::is_pod<node>::value;
    int b = a;
    return b;
}

int main() {
    node n = { 5 };

    B laji; A dalao;
    const auto func = &A::func;
    call(laji, func);
    call(dalao, func);
    int bk = 9;
}
