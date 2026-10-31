#include "audio_icons.h"
#include <windows.h>
#include <stdio.h>
#include <wchar.h>

static const wchar_t *type_name(AudioDeviceType type) {
    switch (type) {
        case AUDIO_TYPE_HEADPHONES: return L"headphones";
        case AUDIO_TYPE_AIRBUDS:    return L"airbuds";
        case AUDIO_TYPE_SPEAKERS:   return L"speakers";
        default:                    return L"headphones";
    }
}

static HICON audio_icon_load_file(const wchar_t *fileName) {
    wchar_t exePath[MAX_PATH];
    wchar_t iconPath[MAX_PATH];
    wchar_t *slash;

    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    slash = wcsrchr(exePath, L'\\');
    if (slash) *slash = 0;

    _snwprintf(
        iconPath,
        MAX_PATH,
        L"%s\\assets\\icons\\%s",
        exePath,
        fileName
    );
    iconPath[MAX_PATH - 1] = 0;

    return (HICON)LoadImageW(
        NULL,
        iconPath,
        IMAGE_ICON,
        0,
        0,
        LR_LOADFROMFILE | LR_DEFAULTSIZE
    );
}

HICON audio_icon_load(AudioDeviceType type, int active) {
    wchar_t fileName[MAX_PATH];

    if (type == AUDIO_TYPE_UNKNOWN) {
        type = AUDIO_TYPE_HEADPHONES;
    }

    _snwprintf(
        fileName,
        MAX_PATH,
        L"audio_%s_%s.ico",
        type_name(type),
        active ? L"on" : L"off"
    );
    fileName[MAX_PATH - 1] = 0;

    return audio_icon_load_file(fileName);
}

HICON audio_icon_load_ptt(AudioDeviceType type) {
    wchar_t fileName[MAX_PATH];
    HICON icon;

    if (type == AUDIO_TYPE_UNKNOWN) {
        type = AUDIO_TYPE_HEADPHONES;
    }

    _snwprintf(
        fileName,
        MAX_PATH,
        L"audio_%s_ptt.ico",
        type_name(type)
    );
    fileName[MAX_PATH - 1] = 0;

    icon = audio_icon_load_file(fileName);

    if (!icon) {
        icon = audio_icon_load_ptt_off();
    }

    return icon;
}

HICON audio_icon_load_ptt_off(void) {
    return audio_icon_load_file(L"off_ptt.ico");
}