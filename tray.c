#define _CRT_SECURE_NO_WARNINGS

#include "tray.h"

#include "key_names.h"
#include "localization.h"
#include "theme.h"
#include "unicode.h"

#include "resource.h"

#include <shellapi.h>
#include <stdio.h>

static const char *tray_get_ptt_name(
    const AppState *state)
{
    static char pttName[64];

    if (!state) {
        return "";
    }

    if (state->vkPttCustom != 0) {
        key_get_display_name(
            state->vkPttCustom,
            pttName,
            sizeof(pttName)
        );

        return pttName;
    }

    if (state->lang == LANG_RU) {
        return state->pttButton == XBUTTON1
            ? "Мышь 4"
            : "Мышь 5";
    }

    return state->pttButton == XBUTTON1
        ? "Mouse 4"
        : "Mouse 5";
}

static BOOL tray_copy_utf8_to_wide(
    const char *source,
    WCHAR *destination,
    int destinationCount)
{
    return utf8_to_wide(
        source,
        destination,
        destinationCount
    ) > 0;
}

BOOL tray_load_icons(
    AppState *state)
{
    HINSTANCE instance;

    if (!state) {
        return FALSE;
    }

    instance = state->hInst;

    if (!instance) {
        instance = GetModuleHandleA(NULL);
    }

    state->hIconOnLight = (HICON)LoadImageA(
        instance,
        MAKEINTRESOURCEA(IDI_ICON_ON_LIGHT),
        IMAGE_ICON,
        0,
        0,
        LR_DEFAULTSIZE
    );

    state->hIconOnDark = (HICON)LoadImageA(
        instance,
        MAKEINTRESOURCEA(IDI_ICON_ON_DARK),
        IMAGE_ICON,
        0,
        0,
        LR_DEFAULTSIZE
    );

    state->hIconOffRed = (HICON)LoadImageA(
        instance,
        MAKEINTRESOURCEA(IDI_ICON_OFF_RED),
        IMAGE_ICON,
        0,
        0,
        LR_DEFAULTSIZE
    );

    if (!state->hIconOnLight) {
    state->hIconOnLight = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION);  
}

if (!state->hIconOnDark) {
    state->hIconOnDark =
        state->hIconOnLight;
}

if (!state->hIconOffRed) {
    state->hIconOffRed = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION);  
}

    theme_update_icon(
        &state->hIconOnCurrent,
        state->hIconOnLight,
        state->hIconOnDark,
        state->iconMode
    );

    return state->hIconOnCurrent != NULL &&
           state->hIconOffRed != NULL;
}

void tray_update_tooltip(
    AppState *state)
{
    char micName[64];
    char soundName[64];
    char tooltip[256];

    WCHAR wideTooltip[ARRAYSIZE(
        state->nid.szTip
    )];

    const char *pttName;
    const char *format;

    if (!state ||
        !state->nid.hWnd) {
        return;
    }

    key_build_hotkey_name(
        state->modToggle,
        state->vkToggle,
        micName,
        sizeof(micName)
    );

    key_build_hotkey_name(
        state->modSoundToggle,
        state->vkSoundToggle,
        soundName,
        sizeof(soundName)
    );

    pttName = tray_get_ptt_name(
        state
    );

    format = localization_get(
        state->lang,
        state->muted
            ? "TOOLTIP_FORMAT_OFF"
            : "TOOLTIP_FORMAT_ON"
    );

    state->nid.uFlags =
        NIF_ICON |
        NIF_TIP;

    state->nid.hIcon = state->muted
        ? state->hIconOffRed
        : state->hIconOnCurrent;

    snprintf(
        tooltip,
        sizeof(tooltip),
        format,
        pttName,
        micName,
        soundName
    );

    if (!tray_copy_utf8_to_wide(
            tooltip,
            wideTooltip,
            ARRAYSIZE(wideTooltip))) {
        return;
    }

    lstrcpynW(
        state->nid.szTip,
        wideTooltip,
        ARRAYSIZE(state->nid.szTip)
    );

    Shell_NotifyIconW(
        NIM_MODIFY,
        &state->nid
    );
}

void tray_show_mic_balloon(
    AppState *state,
    int muted)
{
    const char *title;
    const char *text;

    WCHAR wideTitle[ARRAYSIZE(
        state->nid.szInfoTitle
    )];

    WCHAR wideText[ARRAYSIZE(
        state->nid.szInfo
    )];

    if (!state ||
        !state->nid.hWnd ||
        !state->enableBalloonTips) {
        return;
    }

    title = localization_get(
        state->lang,
        muted
            ? "BALLOON_MIC_OFF_TITLE"
            : "BALLOON_MIC_ON_TITLE"
    );

    text = localization_get(
        state->lang,
        muted
            ? "BALLOON_MIC_OFF_TEXT"
            : "BALLOON_MIC_ON_TEXT"
    );

    if (!tray_copy_utf8_to_wide(
            title,
            wideTitle,
            ARRAYSIZE(wideTitle))) {
        return;
    }

    if (!tray_copy_utf8_to_wide(
            text,
            wideText,
            ARRAYSIZE(wideText))) {
        return;
    }

    state->nid.uFlags =
        NIF_INFO;

    lstrcpynW(
        state->nid.szInfoTitle,
        wideTitle,
        ARRAYSIZE(state->nid.szInfoTitle)
    );

    lstrcpynW(
        state->nid.szInfo,
        wideText,
        ARRAYSIZE(state->nid.szInfo)
    );

    state->nid.dwInfoFlags =
        NIIF_INFO;

    Shell_NotifyIconW(
        NIM_MODIFY,
        &state->nid
    );
}

BOOL tray_add_icon(
    AppState *state,
    HWND hWnd)
{
    WCHAR initialTip[] = L"MicPlus";

    if (!state || !hWnd) {
        return FALSE;
    }

    ZeroMemory(
        &state->nid,
        sizeof(state->nid)
    );

    state->nid.cbSize =
        sizeof(state->nid);

    state->nid.hWnd =
        hWnd;

    state->nid.uID =
        1;

    state->nid.uFlags =
        NIF_MESSAGE |
        NIF_ICON |
        NIF_TIP;

    state->nid.uCallbackMessage =
        WM_APP_TRAY;

    if (!tray_load_icons(state)) {
        return FALSE;
    }

    state->nid.hIcon =
        state->muted
            ? state->hIconOffRed
            : state->hIconOnCurrent;

    lstrcpynW(
        state->nid.szTip,
        initialTip,
        ARRAYSIZE(state->nid.szTip)
    );

    if (!Shell_NotifyIconW(
            NIM_ADD,
            &state->nid)) {
        return FALSE;
    }

    state->nid.uVersion =
        NOTIFYICON_VERSION;

    Shell_NotifyIconW(
        NIM_SETVERSION,
        &state->nid
    );

    tray_update_tooltip(
        state
    );

    return TRUE;
}

void tray_remove_icon(
    AppState *state)
{
    if (!state ||
        !state->nid.hWnd) {
        return;
    }

    Shell_NotifyIconW(
        NIM_DELETE,
        &state->nid
    );

    state->nid.hWnd =
        NULL;
}