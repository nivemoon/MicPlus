#define COBJMACROS

#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#include "audio_device_type.h"

static int contains_ci(const wchar_t *haystack, const wchar_t *needle) {
    wchar_t h[256], n[64];
    int i;

    if (!haystack || !needle) return 0;

    for (i = 0; haystack[i] && i < 255; i++)
        h[i] = towlower(haystack[i]);
    h[i] = 0;

    for (i = 0; needle[i] && i < 63; i++)
        n[i] = towlower(needle[i]);
    n[i] = 0;

    return wcsstr(h, n) != NULL;
}

AudioDeviceType audio_device_type_detect(void) {
    IMMDeviceEnumerator *enumerator = NULL;
    IMMDevice *device = NULL;
    IPropertyStore *props = NULL;
    PROPVARIANT var;
    AudioDeviceType result = AUDIO_TYPE_UNKNOWN;
    wchar_t name[256] = L"";
    UINT formFactor = 0xFFFFFFFF;
    int hasFormFactor = 0;

    if (FAILED(CoCreateInstance(
            &CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
            &IID_IMMDeviceEnumerator, (void **)&enumerator)))
        return AUDIO_TYPE_UNKNOWN;

    if (FAILED(enumerator->lpVtbl->GetDefaultAudioEndpoint(
            enumerator, eRender, eMultimedia, &device)))
        goto done;

    if (FAILED(device->lpVtbl->OpenPropertyStore(
            device, STGM_READ, &props)))
        goto done;

    PropVariantInit(&var);

    if (SUCCEEDED(props->lpVtbl->GetValue(
            props, &PKEY_AudioEndpoint_FormFactor, &var))) {
        if (var.vt == VT_UI4) {
            formFactor = var.ulVal;
            hasFormFactor = 1;
        }
        PropVariantClear(&var);
    }

    if (SUCCEEDED(props->lpVtbl->GetValue(
            props, &PKEY_Device_FriendlyName, &var))) {
        if (var.vt == VT_LPWSTR && var.pwszVal) {
            wcsncpy(name, var.pwszVal, 255);
            name[255] = 0;
        }
        PropVariantClear(&var);
    }

    if (contains_ci(name, L"airpods") ||
        contains_ci(name, L"buds") ||
        contains_ci(name, L"tws") ||
        contains_ci(name, L"earbuds") ||
        contains_ci(name, L"airdots")) {
        result = AUDIO_TYPE_AIRBUDS;
    }
    else if (hasFormFactor) {
        switch (formFactor) {
            case 3: /* Headphones */
            case 4: /* Headset */
                result = AUDIO_TYPE_HEADPHONES;
                break;
            case 1: /* Speakers */
            case 6: /* Desktop speakers */
                result = AUDIO_TYPE_SPEAKERS;
                break;
            default:
                result = AUDIO_TYPE_UNKNOWN;
        }
    }

done:
    if (props) props->lpVtbl->Release(props);
    if (device) device->lpVtbl->Release(device);
    if (enumerator) enumerator->lpVtbl->Release(enumerator);

    return result;
}

BOOL audio_device_output_active(void)
{
    IMMDeviceEnumerator *enumerator = NULL;
    IMMDevice *device = NULL;
    DWORD state = 0;
    BOOL active = FALSE;

    HRESULT hr = CoCreateInstance(
        &CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        &IID_IMMDeviceEnumerator,
        (void **)&enumerator
    );

    if (FAILED(hr) || !enumerator) {
        return FALSE;
    }

    hr = IMMDeviceEnumerator_GetDefaultAudioEndpoint(
        enumerator,
        eRender,
        eConsole,
        &device
    );

    if (SUCCEEDED(hr) && device) {
        if (SUCCEEDED(IMMDevice_GetState(device, &state))) {
            active = (state == DEVICE_STATE_ACTIVE);
        }

        IMMDevice_Release(device);
    }

    IMMDeviceEnumerator_Release(enumerator);

    return active;
}