#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    wstring x  = L"abcdefg";
    wcout  << x  << endl;
    wcout  << x.c_str() << endl; // 上下两句输出结果一样，没问题。
    wifstream fin("raw.txt", ios::in  | ios::binary);
    if (!fin.is_open())
    {
        wcerr  << "Error." << endl;
        return 0;
    }

    /*wofstream fout("out.txt", ios::out  | ios::binary);
    if (!fout.is_open())
    {
        wcerr  << "Error." << endl;
        return 0;
    }*/
    wstring w_str;
    if (!fin.is_open())
    {
        cout  << "Error." << endl;
        return 0;
    }
    fin.seekg(2, ios::_Seekcur);
    while (!fin.eof())
    {
        getline(fin, w_str);
        wcout  << w_str  << endl;
        wcout  << w_str.c_str() << endl;
        // 这里，在读取一行以后，上下两句输出结果不同了，是什么原因？
    }

    return 0;
}