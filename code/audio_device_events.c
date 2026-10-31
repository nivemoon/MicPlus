#define COBJMACROS

#include "audio_device_events.h"

#include <mmdeviceapi.h>
#include <stdlib.h>

static const IID MICPLUS_CLSID_MMDeviceEnumerator =
    { 0xBCDE0395, 0xE52F, 0x467C,
      { 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E } };

static const IID MICPLUS_IID_IMMDeviceEnumerator =
    { 0xA95664D2, 0x9614, 0x4F35,
      { 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6 } };

static const IID MICPLUS_IID_IMMNotificationClient =
    { 0x7991EEC9, 0x7E89, 0x4D85,
      { 0x83, 0x90, 0x6A, 0x70, 0xD5, 0xDE, 0x53, 0x07 } };

typedef struct AudioDeviceEvents {
    IMMNotificationClientVtbl *lpVtbl;
    LONG refCount;
    HWND hWnd;
} AudioDeviceEvents;

static HRESULT STDMETHODCALLTYPE ade_query_interface(
    IMMNotificationClient *self,
    REFIID riid,
    void **object)
{
    AudioDeviceEvents *events = (AudioDeviceEvents *)self;

    if (!object) {
        return E_POINTER;
    }

    *object = NULL;

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &MICPLUS_IID_IMMNotificationClient)) {
        *object = self;
        events->refCount++;
        return S_OK;
    }

    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE ade_add_ref(
    IMMNotificationClient *self)
{
    AudioDeviceEvents *events = (AudioDeviceEvents *)self;

    return (ULONG)InterlockedIncrement(&events->refCount);
}

static ULONG STDMETHODCALLTYPE ade_release(
    IMMNotificationClient *self)
{
    AudioDeviceEvents *events = (AudioDeviceEvents *)self;
    LONG count = InterlockedDecrement(&events->refCount);

    if (count == 0) {
        free(events);
    }

    return (ULONG)count;
}

static void ade_notify(HWND hWnd)
{
    if (hWnd) {
        PostMessageW(
            hWnd,
            WM_APP_DEVICE_CHANGED,
            0,
            0
        );
    }
}

static HRESULT STDMETHODCALLTYPE ade_on_device_state_changed(
    IMMNotificationClient *self,
    LPCWSTR deviceId,
    DWORD newState)
{
    AudioDeviceEvents *events = (AudioDeviceEvents *)self;

    (void)deviceId;
    (void)newState;

    ade_notify(events->hWnd);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE ade_on_device_added(
    IMMNotificationClient *self,
    LPCWSTR deviceId)
{
    AudioDeviceEvents *events = (AudioDeviceEvents *)self;

    (void)deviceId;
    ade_notify(events->hWnd);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE ade_on_device_removed(
    IMMNotificationClient *self,
    LPCWSTR deviceId)
{
    AudioDeviceEvents *events = (AudioDeviceEvents *)self;

    (void)deviceId;
    ade_notify(events->hWnd);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE ade_on_default_device_changed(
    IMMNotificationClient *self,
    EDataFlow flow,
    ERole role,
    LPCWSTR defaultDeviceId)
{
    AudioDeviceEvents *events = (AudioDeviceEvents *)self;

    (void)flow;
    (void)role;
    (void)defaultDeviceId;

    ade_notify(events->hWnd);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE ade_on_property_value_changed(
    IMMNotificationClient *self,
    LPCWSTR deviceId,
    const PROPERTYKEY key)
{
    AudioDeviceEvents *events = (AudioDeviceEvents *)self;

    (void)deviceId;
    (void)key;

    ade_notify(events->hWnd);
    return S_OK;
}

static IMMNotificationClientVtbl ade_vtbl = {
    ade_query_interface,
    ade_add_ref,
    ade_release,
    ade_on_device_state_changed,
    ade_on_device_added,
    ade_on_device_removed,
    ade_on_default_device_changed,
    ade_on_property_value_changed
};

static AudioDeviceEvents *g_events = NULL;
static IMMDeviceEnumerator *g_enumerator = NULL;

BOOL audio_device_events_start(HWND hWnd)
{
    HRESULT hr;

    if (g_events) {
        return TRUE;
    }

    g_events = (AudioDeviceEvents *)malloc(sizeof(AudioDeviceEvents));

    if (!g_events) {
        return FALSE;
    }

    g_events->lpVtbl = &ade_vtbl;
    g_events->refCount = 1;
    g_events->hWnd = hWnd;

    hr = CoCreateInstance(
        &MICPLUS_CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        &MICPLUS_IID_IMMDeviceEnumerator,
        (void **)&g_enumerator
    );

    if (FAILED(hr) || !g_enumerator) {
        free(g_events);
        g_events = NULL;
        return FALSE;
    }

    hr = IMMDeviceEnumerator_RegisterEndpointNotificationCallback(
        g_enumerator,
        (IMMNotificationClient *)g_events
    );

    if (FAILED(hr)) {
        IMMDeviceEnumerator_Release(g_enumerator);
        g_enumerator = NULL;
        free(g_events);
        g_events = NULL;
        return FALSE;
    }

    return TRUE;
}

void audio_device_events_stop(void)
{
    if (g_enumerator && g_events) {
        IMMDeviceEnumerator_UnregisterEndpointNotificationCallback(
            g_enumerator,
            (IMMNotificationClient *)g_events
        );
    }

    if (g_enumerator) {
        IMMDeviceEnumerator_Release(g_enumerator);
        g_enumerator = NULL;
    }

    if (g_events) {
        g_events->hWnd = NULL;
        ade_release((IMMNotificationClient *)g_events);
        g_events = NULL;
    }
}