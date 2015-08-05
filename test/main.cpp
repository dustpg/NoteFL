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

#include <iostream>
#include <string>
#include <bitset>

int main() {
    int a = 0;
    (a += 1) += 1;
    using namespace std;
    bitset<8> bit(0b101);
    cout << bit << "  " << bit.to_ulong() << endl;
    return 0;
}