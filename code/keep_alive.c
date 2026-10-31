#include "keep_alive.h"

#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <windows.h>

static HANDLE g_keepAliveThread = NULL;
static volatile LONG g_keepAliveRun = 0;

static DWORD WINAPI keep_alive_thread(LPVOID param)
{
    IMMDeviceEnumerator *enumerator = NULL;
    IMMDevice *device = NULL;
    IAudioClient *client = NULL;
    IAudioRenderClient *render = NULL;
    WAVEFORMATEX *fmt = NULL;
    UINT32 bufferFrames = 0;
    UINT32 padding = 0;
    UINT32 available = 0;
    BYTE *data = NULL;
    HRESULT hr;

    (void)param;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    hr = CoCreateInstance(
        &CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        &IID_IMMDeviceEnumerator,
        (void**)&enumerator);

    if (FAILED(hr) || !enumerator) {
        goto done;
    }

    hr = enumerator->lpVtbl->GetDefaultAudioEndpoint(
        enumerator,
        eRender,
        eConsole,
        &device);

    if (FAILED(hr) || !device) {
        goto done;
    }

    hr = device->lpVtbl->Activate(
        device,
        &IID_IAudioClient,
        CLSCTX_ALL,
        NULL,
        (void**)&client);

    if (FAILED(hr) || !client) {
        goto done;
    }

    hr = client->lpVtbl->GetMixFormat(client, &fmt);

    if (FAILED(hr) || !fmt) {
        goto done;
    }

    hr = client->lpVtbl->Initialize(
        client,
        AUDCLNT_SHAREMODE_SHARED,
        0,
        10000000, /* 1 second */
        0,
        fmt,
        NULL);

    if (FAILED(hr)) {
        goto done;
    }

    hr = client->lpVtbl->GetBufferSize(client, &bufferFrames);

    if (FAILED(hr)) {
        goto done;
    }

    hr = client->lpVtbl->GetService(
        client,
        &IID_IAudioRenderClient,
        (void**)&render);

    if (FAILED(hr) || !render) {
        goto done;
    }

    hr = client->lpVtbl->Start(client);

    if (FAILED(hr)) {
        goto done;
    }

    while (InterlockedCompareExchange(&g_keepAliveRun, 0, 0)) {
        padding = 0;

        if (FAILED(client->lpVtbl->GetCurrentPadding(
                client,
                &padding))) {
            break;
        }

        if (padding < bufferFrames) {
            available = bufferFrames - padding;

            if (SUCCEEDED(render->lpVtbl->GetBuffer(
                    render,
                    available,
                    &data)) && data) {

                ZeroMemory(
                    data,
                    (size_t)available * fmt->nBlockAlign);

                render->lpVtbl->ReleaseBuffer(
                    render,
                    available,
                    0);
            }
        }

        Sleep(50);
    }

    client->lpVtbl->Stop(client);

done:
    if (fmt) {
        CoTaskMemFree(fmt);
    }
    if (render) {
        render->lpVtbl->Release(render);
    }
    if (client) {
        client->lpVtbl->Release(client);
    }
    if (device) {
        device->lpVtbl->Release(device);
    }
    if (enumerator) {
        enumerator->lpVtbl->Release(enumerator);
    }

    CoUninitialize();

    return 0;
}

void keep_alive_start(void)
{
    if (InterlockedCompareExchange(&g_keepAliveRun, 0, 0)) {
        return;
    }

    InterlockedExchange(&g_keepAliveRun, 1);

    g_keepAliveThread = CreateThread(
        NULL,
        0,
        keep_alive_thread,
        NULL,
        0,
        NULL);
}

void keep_alive_stop(void)
{
    HANDLE thread;

    if (!InterlockedCompareExchange(&g_keepAliveRun, 0, 0)) {
        return;
    }

    InterlockedExchange(&g_keepAliveRun, 0);

    thread = InterlockedExchangePointer(
        &g_keepAliveThread,
        NULL);

    if (thread) {
        WaitForSingleObject(thread, 3000);
        CloseHandle(thread);
    }
}

int keep_alive_is_running(void)
{
    return InterlockedCompareExchange(&g_keepAliveRun, 0, 0) ? 1 : 0;
}