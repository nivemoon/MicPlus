#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wchar.h>

#include "switch_devices.h"

BOOL switch_devices_get_output(SwitchDeviceList *list)
{
    IMMDeviceEnumerator *enumerator = NULL;
    IMMDeviceCollection *collection = NULL;
    IMMDevice *defaultDevice = NULL;
    HRESULT hr;
    UINT count;
    UINT i;

    if (!list) {
        return FALSE;
    }

    list->count = 0;

    hr = CoCreateInstance(
        &CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        &IID_IMMDeviceEnumerator,
        (void **)&enumerator
    );

    if (FAILED(hr) || !enumerator) {
        return FALSE;
    }

    /* Текущее устройство по умолчанию */
    hr = enumerator->lpVtbl->GetDefaultAudioEndpoint(
        enumerator,
        eRender,
        eConsole,
        &defaultDevice
    );

    /* Все активные устройства вывода */
    hr = enumerator->lpVtbl->EnumAudioEndpoints(
        enumerator,
        eRender,
        DEVICE_STATE_ACTIVE,
        &collection
    );

    if (FAILED(hr) || !collection) {
        if (defaultDevice) defaultDevice->lpVtbl->Release(defaultDevice);
        enumerator->lpVtbl->Release(enumerator);
        return FALSE;
    }

    collection->lpVtbl->GetCount(collection, &count);

    for (i = 0; i < count && list->count < SWITCH_DEVICES_MAX; i++) {
        IMMDevice *device = NULL;
        IPropertyStore *props = NULL;
        PROPVARIANT varName;
        LPWSTR deviceId = NULL;
        SwitchDevice *item;

        PropVariantInit(&varName);

        if (FAILED(collection->lpVtbl->Item(collection, i, &device))) {
            continue;
        }

        item = &list->devices[list->count];

        /* ID устройства */
        if (SUCCEEDED(device->lpVtbl->GetId(device, &deviceId))) {
            wcsncpy(item->id, deviceId, 127);
            item->id[127] = L'\0';
            CoTaskMemFree(deviceId);
        }

        /* Имя устройства */
        if (SUCCEEDED(device->lpVtbl->OpenPropertyStore(
                device, STGM_READ, &props))) {

            if (SUCCEEDED(props->lpVtbl->GetValue(
                    props,
                    &PKEY_Device_FriendlyName,
                    &varName))) {

                if (varName.vt == VT_LPWSTR && varName.pwszVal) {
                    wcsncpy(item->name, varName.pwszVal,
                            SWITCH_DEVICES_NAME_LEN - 1);
                    item->name[SWITCH_DEVICES_NAME_LEN - 1] = L'\0';
                }
            }

            PropVariantClear(&varName);
            props->lpVtbl->Release(props);
        }

        /* Это устройство — default? */
        item->isDefault = FALSE;
        if (defaultDevice) {
            LPWSTR defId = NULL;
            if (SUCCEEDED(defaultDevice->lpVtbl->GetId(
                    defaultDevice, &defId))) {
                if (wcscmp(item->id, defId) == 0) {
                    item->isDefault = TRUE;
                }
                CoTaskMemFree(defId);
            }
        }

        list->count++;
        device->lpVtbl->Release(device);
    }

    if (defaultDevice) defaultDevice->lpVtbl->Release(defaultDevice);
    collection->lpVtbl->Release(collection);
    enumerator->lpVtbl->Release(enumerator);

    return list->count > 0;
}

/* ---- Переключение устройства по умолчанию ---- */

static const CLSID CLSID_PolicyConfigClient =
    { 0x870af99c, 0x171d, 0x4f9e,
      { 0xaf, 0x0d, 0xe6, 0x3d, 0xf4, 0x0c, 0x2b, 0xc9 } };

static const IID IID_IPolicyConfig =
    { 0xf8679f50, 0x850a, 0x41cf,
      { 0x9c, 0x72, 0x43, 0x0f, 0x29, 0x02, 0x90, 0xc8 } };

typedef struct IPolicyConfig IPolicyConfig;

typedef struct IPolicyConfigVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IPolicyConfig *, REFIID, void **);
    ULONG (STDMETHODCALLTYPE *AddRef)(IPolicyConfig *);
    ULONG (STDMETHODCALLTYPE *Release)(IPolicyConfig *);
    HRESULT (STDMETHODCALLTYPE *GetMixFormat)(
        IPolicyConfig *, PCWSTR, WAVEFORMATEX **);
    HRESULT (STDMETHODCALLTYPE *GetDeviceFormat)(
        IPolicyConfig *, PCWSTR, INT, WAVEFORMATEX **);
    HRESULT (STDMETHODCALLTYPE *ResetDeviceFormat)(
        IPolicyConfig *, PCWSTR);
    HRESULT (STDMETHODCALLTYPE *SetDeviceFormat)(
        IPolicyConfig *, PCWSTR, WAVEFORMATEX *, WAVEFORMATEX *);
    HRESULT (STDMETHODCALLTYPE *GetProcessingPeriod)(
        IPolicyConfig *, PCWSTR, INT, PINT64, PINT64);
    HRESULT (STDMETHODCALLTYPE *SetProcessingPeriod)(
        IPolicyConfig *, PCWSTR, PINT64);
    HRESULT (STDMETHODCALLTYPE *GetShareMode)(
        IPolicyConfig *, PCWSTR, void *);
    HRESULT (STDMETHODCALLTYPE *SetShareMode)(
        IPolicyConfig *, PCWSTR, void *);
    HRESULT (STDMETHODCALLTYPE *GetPropertyValue)(
        IPolicyConfig *, PCWSTR, const PROPERTYKEY *, PROPVARIANT *);
    HRESULT (STDMETHODCALLTYPE *SetPropertyValue)(
        IPolicyConfig *, PCWSTR, const PROPERTYKEY *, PROPVARIANT *);
    HRESULT (STDMETHODCALLTYPE *SetDefaultEndpoint)(
        IPolicyConfig *, PCWSTR, INT);
    HRESULT (STDMETHODCALLTYPE *SetEndpointVisibility)(
        IPolicyConfig *, PCWSTR, INT);
} IPolicyConfigVtbl;

struct IPolicyConfig {
    IPolicyConfigVtbl *lpVtbl;
};

BOOL switch_devices_set_default(const wchar_t *deviceId)
{
    IPolicyConfig *policy = NULL;
    HRESULT hr;

    if (!deviceId || !deviceId[0]) {
        return FALSE;
    }

    hr = CoCreateInstance(
        &CLSID_PolicyConfigClient,
        NULL,
        CLSCTX_INPROC_SERVER,
        &IID_IPolicyConfig,
        (void **)&policy
    );

    if (FAILED(hr) || !policy) {
        return FALSE;
    }

    /* 0 = eConsole (обычные приложения) */
    hr = policy->lpVtbl->SetDefaultEndpoint(
        policy,
        deviceId,
        0
    );

    policy->lpVtbl->Release(policy);

    return SUCCEEDED(hr);
}

/* ---- Устройства ввода (микрофоны) ---- */

BOOL switch_devices_get_input(SwitchDeviceList *list)
{
    IMMDeviceEnumerator *enumerator = NULL;
    IMMDeviceCollection *collection = NULL;
    IMMDevice *defaultDevice = NULL;
    HRESULT hr;
    UINT count;
    UINT i;

    if (!list) {
        return FALSE;
    }

    list->count = 0;

    hr = CoCreateInstance(
        &CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        &IID_IMMDeviceEnumerator,
        (void **)&enumerator
    );

    if (FAILED(hr) || !enumerator) {
        return FALSE;
    }

    hr = enumerator->lpVtbl->GetDefaultAudioEndpoint(
        enumerator,
        eCapture,
        eCommunications,
        &defaultDevice
    );

    hr = enumerator->lpVtbl->EnumAudioEndpoints(
        enumerator,
        eCapture,
        DEVICE_STATE_ACTIVE,
        &collection
    );

    if (FAILED(hr) || !collection) {
        if (defaultDevice) defaultDevice->lpVtbl->Release(defaultDevice);
        enumerator->lpVtbl->Release(enumerator);
        return FALSE;
    }

    collection->lpVtbl->GetCount(collection, &count);

    for (i = 0; i < count && list->count < SWITCH_DEVICES_MAX; i++) {
        IMMDevice *device = NULL;
        IPropertyStore *props = NULL;
        PROPVARIANT varName;
        LPWSTR deviceId = NULL;
        SwitchDevice *item;

        PropVariantInit(&varName);

        if (FAILED(collection->lpVtbl->Item(collection, i, &device))) {
            continue;
        }

        item = &list->devices[list->count];

        if (SUCCEEDED(device->lpVtbl->GetId(device, &deviceId))) {
            wcsncpy(item->id, deviceId, 127);
            item->id[127] = L'\0';
            CoTaskMemFree(deviceId);
        }

        if (SUCCEEDED(device->lpVtbl->OpenPropertyStore(
                device, STGM_READ, &props))) {

            if (SUCCEEDED(props->lpVtbl->GetValue(
                    props,
                    &PKEY_Device_FriendlyName,
                    &varName))) {

                if (varName.vt == VT_LPWSTR && varName.pwszVal) {
                    wcsncpy(item->name, varName.pwszVal,
                            SWITCH_DEVICES_NAME_LEN - 1);
                    item->name[SWITCH_DEVICES_NAME_LEN - 1] = L'\0';
                }
            }

            PropVariantClear(&varName);
            props->lpVtbl->Release(props);
        }

        item->isDefault = FALSE;
        if (defaultDevice) {
            LPWSTR defId = NULL;
            if (SUCCEEDED(defaultDevice->lpVtbl->GetId(
                    defaultDevice, &defId))) {
                if (wcscmp(item->id, defId) == 0) {
                    item->isDefault = TRUE;
                }
                CoTaskMemFree(defId);
            }
        }

        list->count++;
        device->lpVtbl->Release(device);
    }

    if (defaultDevice) defaultDevice->lpVtbl->Release(defaultDevice);
    collection->lpVtbl->Release(collection);
    enumerator->lpVtbl->Release(enumerator);

    return list->count > 0;
}

BOOL switch_devices_set_default_input(const wchar_t *deviceId)
{
    IPolicyConfig *policy = NULL;
    HRESULT hr;

    if (!deviceId || !deviceId[0]) {
        return FALSE;
    }

    hr = CoCreateInstance(
        &CLSID_PolicyConfigClient,
        NULL,
        CLSCTX_INPROC_SERVER,
        &IID_IPolicyConfig,
        (void **)&policy
    );

    if (FAILED(hr) || !policy) {
        return FALSE;
    }

    /* 2 = eCommunications — то, что используют Discord, игры, звонки */
    hr = policy->lpVtbl->SetDefaultEndpoint(
        policy,
        deviceId,
        2
    );

    policy->lpVtbl->Release(policy);

    return SUCCEEDED(hr);
}