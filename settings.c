#define _CRT_SECURE_NO_WARNINGS

#include "settings.h"

#include <stdio.h>
#include <stdlib.h>

static int settings_read_bool(
    const char *section,
    const char *key,
    int defaultValue,
    const char *iniPath)
{
    char defaultText[8];
    char value[32];

    snprintf(
        defaultText,
        sizeof(defaultText),
        "%d",
        defaultValue ? 1 : 0
    );

    GetPrivateProfileStringA(
        section,
        key,
        defaultText,
        value,
        sizeof(value),
        iniPath
    );

    return value[0] != '0';
}

static UINT settings_read_uint(
    const char *section,
    const char *key,
    UINT defaultValue,
    const char *iniPath)
{
    char defaultText[32];
    char value[32];

    snprintf(
        defaultText,
        sizeof(defaultText),
        "%u",
        defaultValue
    );

    GetPrivateProfileStringA(
        section,
        key,
        defaultText,
        value,
        sizeof(value),
        iniPath
    );

    return (UINT)strtoul(
        value,
        NULL,
        10
    );
}

static void settings_read_string(
    const char *section,
    const char *key,
    const char *defaultValue,
    char *out,
    DWORD outSize,
    const char *iniPath)
{
    if (!out ||
        outSize == 0) {
        return;
    }

    GetPrivateProfileStringA(
        section,
        key,
        defaultValue,
        out,
        outSize,
        iniPath
    );
}

static void settings_write_bool(
    const char *section,
    const char *key,
    int value,
    const char *iniPath)
{
    WritePrivateProfileStringA(
        section,
        key,
        value ? "1" : "0",
        iniPath
    );
}

static void settings_write_uint(
    const char *section,
    const char *key,
    UINT value,
    const char *iniPath)
{
    char buffer[32];

    snprintf(
        buffer,
        sizeof(buffer),
        "%u",
        value
    );

    WritePrivateProfileStringA(
        section,
        key,
        buffer,
        iniPath
    );
}

static void settings_write_string(
    const char *section,
    const char *key,
    const char *value,
    const char *iniPath)
{
    WritePrivateProfileStringA(
        section,
        key,
        value ? value : "",
        iniPath
    );
}

void settings_load(
    AppState *state,
    const AppPaths *paths)
{
    char language[8];

    if (!state ||
        !paths ||
        paths->iniPath[0] == '\0') {
        return;
    }

    state->pttMode = settings_read_bool(
        "General",
        "PTTMode",
        0,
        paths->iniPath
    );

    state->vkToggle = settings_read_uint(
        "General",
        "ToggleVK",
        VK_F10,
        paths->iniPath
    );

    if (state->vkToggle == 0) {
        state->vkToggle = VK_F10;
    }

    state->modToggle = settings_read_uint(
        "General",
        "ToggleMods",
        0,
        paths->iniPath
    );

    state->pttButton =
        settings_read_uint(
            "General",
            "PTTButton",
            2,
            paths->iniPath
        ) == 1
            ? XBUTTON1
            : XBUTTON2;

    state->vkPttCustom = settings_read_uint(
        "General",
        "PttCustomVK",
        0,
        paths->iniPath
    );

    settings_read_string(
        "General",
        "Language",
        "ru",
        language,
        sizeof(language),
        paths->iniPath
    );

    state->lang =
        lstrcmpiA(language, "ru") == 0
            ? LANG_RU
            : LANG_EN;

    state->vkSoundToggle = settings_read_uint(
        "General",
        "SoundToggleVK",
        VK_F9,
        paths->iniPath
    );

    if (state->vkSoundToggle == 0) {
        state->vkSoundToggle = VK_F9;
    }

    state->modSoundToggle = MOD_CONTROL;

    state->autoStart = settings_read_bool(
        "General",
        "AutoStart",
        0,
        paths->iniPath
    );

    state->enableMicSounds = settings_read_bool(
        "General",
        "EnableMicSounds",
        1,
        paths->iniPath
    );

    state->enablePTTSounds = settings_read_bool(
        "General",
        "EnablePTTSounds",
        1,
        paths->iniPath
    );

    state->enableBalloonTips = settings_read_bool(
        "General",
        "EnableBalloonTips",
        1,
        paths->iniPath
    );

    settings_read_string(
        "General",
        "MicSoundOn",
        "mic_on.wav",
        state->micSoundOn,
        sizeof(state->micSoundOn),
        paths->iniPath
    );

    settings_read_string(
        "General",
        "MicSoundOff",
        "mic_off.wav",
        state->micSoundOff,
        sizeof(state->micSoundOff),
        paths->iniPath
    );

    settings_read_string(
        "General",
        "PTTSoundOn",
        "ptt_start.wav",
        state->pttSoundOn,
        sizeof(state->pttSoundOn),
        paths->iniPath
    );

    settings_read_string(
        "General",
        "PTTSoundOff",
        "ptt_end.wav",
        state->pttSoundOff,
        sizeof(state->pttSoundOff),
        paths->iniPath
    );

    state->iconMode = GetPrivateProfileIntA(
        "MicPlus",
        "IconMode",
        0,
        paths->iniPath
    );

    if (state->iconMode < 0 ||
        state->iconMode > 2) {
        state->iconMode = 0;
    }

    state->menuTheme = GetPrivateProfileIntA(
        "MicPlus",
        "MenuTheme",
        0,
        paths->iniPath
    );

    if (state->menuTheme < 0 ||
        state->menuTheme > 2) {
        state->menuTheme = 0;
    }

    // НОВОЕ: загрузка hideTrayIcon
    state->hideTrayIcon = settings_read_bool(
        "General",
        "HideTrayIcon",
        0,
        paths->iniPath
    );
}

void settings_save(
    const AppState *state,
    const AppPaths *paths)
{
    if (!state ||
        !paths ||
        paths->iniPath[0] == '\0') {
        return;
    }

    settings_write_bool(
        "General",
        "PTTMode",
        state->pttMode,
        paths->iniPath
    );

    settings_write_uint(
        "General",
        "ToggleVK",
        state->vkToggle,
        paths->iniPath
    );

    settings_write_uint(
        "General",
        "ToggleMods",
        state->modToggle,
        paths->iniPath
    );

    settings_write_uint(
        "General",
        "PTTButton",
        state->pttButton == XBUTTON1
            ? 1
            : 2,
        paths->iniPath
    );

    settings_write_uint(
        "General",
        "PttCustomVK",
        state->vkPttCustom,
        paths->iniPath
    );

    settings_write_string(
        "General",
        "Language",
        state->lang == LANG_RU
            ? "ru"
            : "en",
        paths->iniPath
    );

    settings_write_uint(
        "General",
        "SoundToggleVK",
        state->vkSoundToggle,
        paths->iniPath
    );

    settings_write_bool(
        "General",
        "AutoStart",
        state->autoStart,
        paths->iniPath
    );

    settings_write_bool(
        "General",
        "EnableMicSounds",
        state->enableMicSounds,
        paths->iniPath
    );

    settings_write_bool(
        "General",
        "EnablePTTSounds",
        state->enablePTTSounds,
        paths->iniPath
    );

    settings_write_bool(
        "General",
        "EnableBalloonTips",
        state->enableBalloonTips,
        paths->iniPath
    );

    settings_write_string(
        "General",
        "MicSoundOn",
        state->micSoundOn,
        paths->iniPath
    );

    settings_write_string(
        "General",
        "MicSoundOff",
        state->micSoundOff,
        paths->iniPath
    );

    settings_write_string(
        "General",
        "PTTSoundOn",
        state->pttSoundOn,
        paths->iniPath
    );

    settings_write_string(
        "General",
        "PTTSoundOff",
        state->pttSoundOff,
        paths->iniPath
    );

    settings_write_uint(
        "MicPlus",
        "IconMode",
        (UINT)state->iconMode,
        paths->iniPath
    );

    settings_write_uint(
        "MicPlus",
        "MenuTheme",
        (UINT)state->menuTheme,
        paths->iniPath
    );

    // НОВОЕ: сохранение hideTrayIcon
    settings_write_bool(
        "General",
        "HideTrayIcon",
        state->hideTrayIcon,
        paths->iniPath
    );
}