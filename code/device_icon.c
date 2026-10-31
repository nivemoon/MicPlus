#include "device_icon.h"
#include "audio_icons.h"
#include <windows.h>
#include <stdio.h>
#include <wchar.h>

static void ini_path(wchar_t *path, size_t size)
{
    wchar_t exePath[MAX_PATH];
    wchar_t *slash;

    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    slash = wcsrchr(exePath, L'\\');
    if (slash) *slash = 0;

    _snwprintf(path, size, L"%s\\icon_overrides.ini", exePath);
    path[size - 1] = 0;
}

static const wchar_t *type_to_name(AudioDeviceType type)
{
    switch (type) {
        case AUDIO_TYPE_HEADPHONES: return L"headphones";
        case AUDIO_TYPE_AIRBUDS:    return L"airbuds";
        case AUDIO_TYPE_SPEAKERS:   return L"speakers";
        default:                    return L"";
    }
}

static AudioDeviceType name_to_type(const wchar_t *name)
{
    if (_wcsicmp(name, L"headphones") == 0) return AUDIO_TYPE_HEADPHONES;
    if (_wcsicmp(name, L"airbuds") == 0)    return AUDIO_TYPE_AIRBUDS;
    if (_wcsicmp(name, L"speakers") == 0)   return AUDIO_TYPE_SPEAKERS;
    return AUDIO_TYPE_UNKNOWN;
}

AudioDeviceType device_icon_get_override(const wchar_t *deviceId)
{
    wchar_t path[MAX_PATH];
    wchar_t value[64];

    if (!deviceId || !deviceId[0]) {
        return AUDIO_TYPE_UNKNOWN;
    }

    ini_path(path, MAX_PATH);

    GetPrivateProfileStringW(
        L"IconOverrides",
        deviceId,
        L"",
        value,
        64,
        path
    );

    return name_to_type(value);
}

void device_icon_set_override(const wchar_t *deviceId, const wchar_t *typeName)
{
    wchar_t path[MAX_PATH];

    if (!deviceId || !deviceId[0]) {
        return;
    }

    ini_path(path, MAX_PATH);

    if (!typeName || !typeName[0]) {
        WritePrivateProfileStringW(
            L"IconOverrides", deviceId, NULL, path);
    } else {
        WritePrivateProfileStringW(
            L"IconOverrides", deviceId, typeName, path);
    }
}

HICON device_icon_load(
    const wchar_t *deviceId,
    AudioDeviceType detected,
    int active)
{
    AudioDeviceType type = device_icon_get_override(deviceId);

    if (type == AUDIO_TYPE_UNKNOWN) {
        type = detected;
    }

    return audio_icon_load(type, active);
}

HICON device_icon_load_ptt(
    const wchar_t *deviceId,
    AudioDeviceType detected)
{
    AudioDeviceType type = device_icon_get_override(deviceId);

    if (type == AUDIO_TYPE_UNKNOWN) {
        type = detected;
    }

    return audio_icon_load_ptt(type);
}