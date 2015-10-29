/*#include <windows.h>
#include <msctf.h>
#include <cwchar>
#include <cstdio>

int main(int argc, char* argv[]) {
    CoInitialize(0);
    HRESULT hr = S_OK;

    ITfInputProcessorProfiles *pProfiles;

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfiles,
        (LPVOID*)&pProfiles);

    if (SUCCEEDED(hr)) {
        {
            LANGID* pid = nullptr; ULONG length = 0;
            pProfiles->GetLanguageList(&pid, &length);
            if (pid) {
                ::CoTaskMemFree(pid);
            }
        }
        {
        }
        IEnumTfLanguageProfiles* pEnumProf = 0;
        hr = pProfiles->EnumLanguageProfiles(0x0804, &pEnumProf);
        if (SUCCEEDED(hr) && pEnumProf) {
            TF_LANGUAGEPROFILE proArr[2];
            ULONG feOut = 0;
            while (S_OK == pEnumProf->Next(1, proArr, &feOut)) {
                BSTR bstrDest;
                hr = pProfiles->GetLanguageProfileDescription(proArr[0].clsid, 0x804, proArr[0].guidProfile, &bstrDest);
                OutputDebugStringW(bstrDest);
                printf("%S:%ls\n", bstrDest, bstrDest);

                BOOL bEnable = false;
                hr = pProfiles->IsEnabledLanguageProfile(proArr[0].clsid, 0x804, proArr[0].guidProfile, &bEnable);
                if (SUCCEEDED(hr)) {
                    printf("Enabled %d\n", bEnable);
                }
                SysFreeString(bstrDest);
            }
        }

        pProfiles->Release();
    }

    CoUninitialize();
    return 0;
}*/
# include <stdio.h>

void action1(int x, int y);
void action2(int x, int y);//声明不带形参会出现错误
int main(void)
{
    char ch = '\n';
    int a, b;

    printf("请输入你要处理的两个数据，中间以空格隔开。\n");
    scanf("%d %d", &a, &b);
    printf("请输入你要对数据的处理方式，若为加法请输入A ,若为乘法请输入B或者C。\n");
    while(ch == '\n') scanf("%c", &ch);

    switch(ch)
    {
    case'A':
        action1(a, b);
        break;
    case'B':
    case'C':
        action2(a, b);
        break;
    default:
        putchar('\a');//输入其它字符发出警告

    }

    return 0;
}

void action1(int x, int y)
{
    printf("x + y = %d\n",x+y);
}
void action2(int x, int y)//与action1中变量不冲突
{
    printf("x * y = %d\n",x*y);
}