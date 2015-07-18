#include <cstdlib>
#include <cstdio>
#include <memory>
#include <ctime>

int main() {
    const int MAX = 1000000000;        //10亿字节
    char* a = (char*)malloc(MAX);
    memset(a, '1', MAX);
    a[MAX - 1] = '\0';
    int nIndex = 0;
    long nBegin = clock();
    while (a[nIndex])
        nIndex++;
    long nIndexTime = clock();
    {
        auto itr = a;
        while (*itr) ++itr;
    }
    long nPointerTime = clock();
    printf("IndexCount: %d ms\n", nIndexTime - nBegin);
    //IndexCount: 1831 ms
    printf("PointerCount: %d ms", nPointerTime - nIndexTime);
    return 0;
}