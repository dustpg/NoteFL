#include <windows.h>
#include <Mmdeviceapi.h>
#include <Endpointvolume.h>
#include <cstdio>


class MyUnknown : private IUnknown {

};

class MySad : public MyUnknown {
public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) noexcept override {
        return E_NOINTERFACE;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(void) noexcept override {
        return 2;
    }
    virtual ULONG STDMETHODCALLTYPE Release(void) noexcept override {
        return 1;
    }
};


int main() {
    MySad sad;
    MyUnknown* my = &sad;
    my->AddRef();


    ::CoInitialize(nullptr);
    IMMDeviceEnumerator *pEnum = nullptr;
    IMMDevice *pDevice = nullptr;
    //IAudioMeterInformation *pMeter = nullptr;
    IDeviceTopology* pDeviceTopology = nullptr;

    HRESULT hr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        (void**)&pEnum);

    hr = pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

    hr = pDevice->Activate(__uuidof(pDeviceTopology),
        CLSCTX_ALL,
        NULL,
        (void**)&pDeviceTopology);


    IPart* part = nullptr;
    hr = pDeviceTopology->GetPartById(0, &part);

    IAudioPeakMeter* meter = nullptr;
    part->Activate(
        CLSCTX_ALL,
        __uuidof(meter),
        (void**)&meter);
    while (true) {
        UINT count = 0;
        float peak = 0.f;
        meter->GetChannelCount(&count);
        meter->GetLevel(0, &peak);
        //pMeter->GetPeakValue(&peak);

        printf("%d : %f\n", count, peak);
        Sleep(20);
    }

    return EXIT_SUCCESS;
}