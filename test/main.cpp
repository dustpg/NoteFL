#include <functional>
#include <algorithm>
#include <vector>
#include <map>


/// <summary>
/// Mains the specified argc.
/// </summary>
/// <param name="argc">The count of arguments</param>
/// <param name="charv">The arguments vector.</param>
/// <remarks>这条函数在大多数情况下立即返回, 除了等待IO</remarks>
/// <returns>return 0 if exit successfully</returns>
int main(int argc, char* charv[]) {
    for (int i = 0; i < argc; ++i) {
        printf("%s\r\n", charv[i]);
    }
    return 0;
}