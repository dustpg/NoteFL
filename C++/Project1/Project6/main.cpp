#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdint>
#include <cstdlib>


class Class {
public:
    Class() noexcept {};
    Class(const Class&) noexcept {
        std::printf("FUNC<%s> called\n", __FUNCTION__);
    };
    Class(Class&& x) noexcept : m_ptr(x.m_ptr) { 
        m_ptr = nullptr; 
        std::printf("FUNC<%s> called\n", __FUNCTION__);
    }
    ~Class() noexcept { std::free(m_ptr); }
    auto&operator=(const Class& ) noexcept {
        std::printf("FUNC<%s> called\n", __FUNCTION__);
        return *this;
    }
    auto&operator=(Class&& ) noexcept {
        std::printf("FUNC<%s> called\n", __FUNCTION__);
        return *this;
    }
private:
    void*   m_ptr = std::malloc(1024);
};


auto func(Class&& c) noexcept -> int {
    Class a = c;
    return sizeof(a);
}

int main() {
    func(Class{});
    std::getchar();
    return 0;
}
