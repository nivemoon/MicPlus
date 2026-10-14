#define _CRT_SECURE_NO_WARNINGS

#include "audio_device.h"
#include <initguid.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

static IMMDeviceEnumerator *g_enumerator = NULL;
static IMMDevice *g_device = NULL;
static IAudioEndpointVolume *g_volume = NULL;

int audio_device_init(void)
{
    HRESULT hr;

    if (g_volume) {
        return 1;
    }

    hr = CoCreateInstance(
        &CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        &IID_IMMDeviceEnumerator,
        (void **)&g_enumerator
    );

    if (FAILED(hr) || !g_enumerator) {
        return 0;
    }

    hr = g_enumerator->lpVtbl->GetDefaultAudioEndpoint(
        g_enumerator,
        eCapture,
        eCommunications,
        &g_device
    );

    if (FAILED(hr) || !g_device) {
        return 0;
    }

    hr = g_device->lpVtbl->Activate(
        g_device,
        &IID_IAudioEndpointVolume,
        CLSCTX_ALL,
        NULL,
        (void **)&g_volume
    );

    if (FAILED(hr) || !g_volume) {
        return 0;
    }

    return 1;
}

void audio_device_shutdown(void)
{
    if (g_volume) {
        g_volume->lpVtbl->Release(g_volume);
        g_volume = NULL;
    }

    if (g_device) {
        g_device->lpVtbl->Release(g_device);
        g_device = NULL;
    }

    if (g_enumerator) {
        g_enumerator->lpVtbl->Release(g_enumerator);
        g_enumerator = NULL;
    }
}

int audio_device_get_mic_mute(void)
{
    BOOL muted = FALSE;

    if (!g_volume) {
        return -1;
    }

    if (FAILED(g_volume->lpVtbl->GetMute(g_volume, &muted))) {
        return -1;
    }

    return muted ? 1 : 0;
}

void audio_device_set_mic_mute(int mute)
{
    if (!g_volume) {
        return;
    }

    g_volume->lpVtbl->SetMute(
        g_volume,
        mute ? TRUE : FALSE,
        NULL
    );
}