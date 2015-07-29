#include    <iostream>
using namespace std;

class A
{
public:
    A(int a) :aa(a) {};
    int aa;
    virtual void __stdcall fun1() { cout << "this is fun1" << endl; }
    virtual void __stdcall fun2() { cout << "this is fun2" << endl; }
};

typedef void(A::*FUN)();
using FUN2 = void(A::*)();
int main()
{
    sizeof(&A::fun1);
    return 1;
}