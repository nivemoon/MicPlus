#define _CRT_SECURE_NO_WARNINGS

#include "micplus_ini.h"
#include "settings.h"
#include <string.h>
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

static void settings_fix_sound_extensions(AppState *state)
{
    char *fields[] = {
        state->micSoundOn,
        state->micSoundOff,
        state->pttSoundOn,
        state->pttSoundOff
    };
    int i;

    for (i = 0; i < 4; i++) {
        size_t len = strlen(fields[i]);

        if (len >= 4 &&
            _stricmp(fields[i] + len - 4, ".wav") == 0) {
            strcpy(fields[i] + len - 4, ".mp3");
        }
    }
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
    const char *ini = IniPath();

    (void)paths;

    if (!state || ini[0] == '\0') {
        return;
    }

    state->pttMode = settings_read_bool(
        "General",
        "PTTMode",
        1,
        ini
    );

    state->vkPttModeChordFirst = settings_read_uint(
        "General",
        "PttModeChordFirst",
        0,
        ini
    );

    state->vkPttModeChordSecond = settings_read_uint(
        "General",
        "PttModeChordSecond",
        0,
        ini
    );

    state->pttModeChordKeyDown = 0;

    state->vkPttModeToggle = settings_read_uint(
        "General",
        "PttModeToggleVK",
        'P',
        ini
    );

    state->modPttModeToggle = settings_read_uint(
        "General",
        "PttModeToggleMods",
        MOD_ALT,
        ini
    );

    state->vkToggle = settings_read_uint(
        "General",
        "ToggleVK",
        VK_F10,
        ini
    );

    if (state->vkToggle == 0) {
        state->vkToggle = VK_F10;
    }

    state->modToggle = settings_read_uint(
        "General",
        "ToggleMods",
        0,
        ini
    );

    state->vkMicChordFirst = settings_read_uint(
        "General",
        "MicChordFirst",
        0,
        ini
    );

    state->vkMicChordSecond = settings_read_uint(
        "General",
        "MicChordSecond",
        0,
        ini
    );

    state->micChordKeyDown = 0;

    {
        UINT savedButton = settings_read_uint(
            "General", "PTTButton", 2, ini);

        state->pttButton =
            savedButton == 1 ? XBUTTON1 :
            savedButton == 2 ? XBUTTON2 : 0;
    }

    state->vkPttCustom = settings_read_uint(
        "General",
        "PttCustomVK",
        0,
        ini
    );

    /* --- Хоткеи DUSK: панель / вывод / список / ввод --- */
    state->panelHotkeyMods = settings_read_uint(
        "General",
        "PanelHotkeyMods",
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        ini
    );

    state->panelHotkeyVk = settings_read_uint(
        "General",
        "PanelHotkeyVK",
        'D',
        ini
    );

    state->outputHotkeyMods = settings_read_uint(
        "General",
        "OutputHotkeyMods",
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        ini
    );

    state->outputHotkeyVk = settings_read_uint(
        "General",
        "OutputHotkeyVK",
        'S',
        ini
    );

    state->listHotkeyMods = settings_read_uint(
        "General",
        "ListHotkeyMods",
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        ini
    );

    state->listHotkeyVk = settings_read_uint(
        "General",
        "ListHotkeyVK",
        'U',
        ini
    );

    state->inputHotkeyMods = settings_read_uint(
        "General",
        "InputHotkeyMods",
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        ini
    );

    state->inputHotkeyVk = settings_read_uint(
        "General",
        "InputHotkeyVK",
        'K',
        ini
    );

    settings_read_string(
        "General",
        "Language",
        "ru",
        language,
        sizeof(language),
        ini
    );

    state->lang =
        lstrcmpiA(language, "ru") == 0
            ? LANG_RU
            : LANG_EN;

    state->vkSoundToggle = settings_read_uint(
        "General",
        "SoundToggleVK",
        VK_F9,
        ini
    );

    if (state->vkSoundToggle == 0) {
        state->vkSoundToggle = VK_F9;
    }

    state->modSoundToggle = MOD_CONTROL;

    state->autoStart = settings_read_bool(
        "General",
        "AutoStart",
        0,
        ini
    );

    state->enableMicSounds = settings_read_bool(
        "General",
        "EnableMicSounds",
        1,
        ini
    );

    state->soundVolume = (int)settings_read_uint(
        "General",
        "SoundVolume",
        40,
        ini
    );

    state->enablePTTSounds = settings_read_bool(
        "General",
        "EnablePTTSounds",
        1,
        ini
    );

    state->enablePTTModeSounds = settings_read_bool(
        "General",
        "EnablePTTModeSounds",
        1,
        ini
    );

    state->enableVolumeTickSound = settings_read_bool(
        "Sounds",
        "VolumeTick",
        1,
        ini
    );

    state->enableBalloonTips = settings_read_bool(
        "General",
        "EnableBalloonTips",
        0,
        ini
    );

    if (!state->micSoundOn[0]) {
        lstrcpynA(
            state->micSoundOn,
            "mic_on.mp3",
            sizeof(state->micSoundOn)
        );
    }

    if (!state->micSoundOff[0]) {
        lstrcpynA(state->micSoundOff, "mic_off.mp3", sizeof(state->micSoundOff));
    }

    if (!state->pttSoundOn[0]) {
        lstrcpynA(state->pttSoundOn, "ptt_start.mp3", sizeof(state->pttSoundOn));
    }

    if (!state->pttSoundOff[0]) {
        lstrcpynA(state->pttSoundOff, "ptt_end.mp3", sizeof(state->pttSoundOff));
    }

    state->iconMode = GetPrivateProfileIntA(
        "MicPlus",
        "IconMode",
        0,
        ini
    );

    if (state->iconMode < 0 ||
        state->iconMode > 2) {
        state->iconMode = 0;
    }

    state->useAudioIcon = settings_read_bool(
        "MicPlus",
        "UseAudioIcon",
        0,
        ini
    );

    state->menuTheme = GetPrivateProfileIntA(
        "MicPlus",
        "MenuTheme",
        0,
        ini
    );

    if (state->menuTheme < 0 ||
        state->menuTheme > 2) {
        state->menuTheme = 0;
    }

    state->leftClickAction = GetPrivateProfileIntA(
        "MicPlus",
        "LeftClickAction",
        1,
        ini
    );

    if (state->leftClickAction < 0 ||
        state->leftClickAction > 2) {
        state->leftClickAction = 1;
    }

    state->middleClickAction = GetPrivateProfileIntA(
        "MicPlus",
        "MiddleClickAction",
        0,
        ini
    );

    if (state->middleClickAction < 0 ||
        state->middleClickAction > 2) {
        state->middleClickAction = 0;
    }

    state->scrollAction = GetPrivateProfileIntA(
        "MicPlus",
        "ScrollAction",
        2,
        ini
    );

    if (state->scrollAction < 0 ||
        state->scrollAction > 2) {
        state->scrollAction = 2;
    }

    state->hideTrayIcon = settings_read_bool(
        "General",
        "HideTrayIcon",
        0,
        ini
    );

    state->muted = settings_read_bool(
        "General",
        "Muted",
        0,
        ini
    );

    state->voiceActivationEnabled = settings_read_bool(
        "General",
        "VoiceActivation",
        0,
        ini
    );

    state->voiceHangoverMs = GetPrivateProfileIntA(
        "MicPlus", "VoiceHangoverMs", 500, ini);

    state->voiceThresholdX1000 = GetPrivateProfileIntA(
        "MicPlus", "VoiceThresholdX1000", 20, ini);

    state->hideMicSelection = settings_read_bool(
        "General",
        "HideMicSelection",
        0,
        ini
    );

    state->keepHeadphonesAwake = settings_read_bool(
        "General",
        "KeepHeadphonesAwake",
        0,
        ini
    );

    state->keepAliveBalloonShown = settings_read_bool(
        "General",
        "KeepAliveBalloonShown",
        0,
        ini
    );

    settings_fix_sound_extensions(state);
}

void settings_save(
    const AppState *state,
    const AppPaths *paths)
{
    const char *ini = IniPath();

    (void)paths;

    if (!state || ini[0] == '\0') {
        return;
    }

    if (IniIsDeleted()) {
        return;
    }

    settings_write_bool(
        "General",
        "KeepHeadphonesAwake",
        state->keepHeadphonesAwake,
        ini
    );

    settings_write_bool(
        "General",
        "KeepAliveBalloonShown",
        state->keepAliveBalloonShown,
        ini
    );

    settings_write_bool(
        "General",
        "PTTMode",
        state->pttMode,
        ini
    );

    settings_write_uint(
        "General",
        "ToggleVK",
        state->vkToggle,
        ini
    );

    settings_write_uint(
        "General",
        "ToggleMods",
        state->modToggle,
        ini
    );

    settings_write_uint(
        "General",
        "MicChordFirst",
        state->vkMicChordFirst,
        ini
    );

    settings_write_uint(
        "General",
        "MicChordSecond",
        state->vkMicChordSecond,
        ini
    );

    settings_write_uint(
        "General",
        "PTTButton",
        state->pttButton == XBUTTON1 ? 1 :
        state->pttButton == XBUTTON2 ? 2 : 0,
        ini
    );

    settings_write_uint(
        "General",
        "PttCustomVK",
        state->vkPttCustom,
        ini
    );

    /* --- Хоткеи DUSK: панель / вывод / список / ввод --- */
    settings_write_uint(
        "General",
        "PanelHotkeyMods",
        state->panelHotkeyMods,
        ini
    );

    settings_write_uint(
        "General",
        "PanelHotkeyVK",
        state->panelHotkeyVk,
        ini
    );

    settings_write_uint(
        "General",
        "OutputHotkeyMods",
        state->outputHotkeyMods,
        ini
    );

    settings_write_uint(
        "General",
        "OutputHotkeyVK",
        state->outputHotkeyVk,
        ini
    );

    settings_write_uint(
        "General",
        "ListHotkeyMods",
        state->listHotkeyMods,
        ini
    );

    settings_write_uint(
        "General",
        "ListHotkeyVK",
        state->listHotkeyVk,
        ini
    );

    settings_write_uint(
        "General",
        "InputHotkeyMods",
        state->inputHotkeyMods,
        ini
    );

    settings_write_uint(
        "General",
        "InputHotkeyVK",
        state->inputHotkeyVk,
        ini
    );

    settings_write_string(
        "General",
        "Language",
        state->lang == LANG_RU
            ? "ru"
            : "en",
        ini
    );

    settings_write_uint(
        "General",
        "SoundToggleVK",
        state->vkSoundToggle,
        ini
    );

    settings_write_uint(
        "General",
        "PttModeToggleVK",
        state->vkPttModeToggle,
        ini
    );

    settings_write_uint(
        "General",
        "PttModeToggleMods",
        state->modPttModeToggle,
        ini
    );

    settings_write_uint(
        "General",
        "PttModeChordFirst",
        state->vkPttModeChordFirst,
        ini
    );

    settings_write_uint(
        "General",
        "PttModeChordSecond",
        state->vkPttModeChordSecond,
        ini
    );

    settings_write_bool(
        "General",
        "AutoStart",
        state->autoStart,
        ini
    );

    settings_write_bool(
        "General",
        "EnableMicSounds",
        state->enableMicSounds,
        ini
    );

    settings_write_uint(
        "General",
        "SoundVolume",
        (UINT)state->soundVolume,
        ini
    );

    settings_write_bool(
        "General",
        "EnablePTTSounds",
        state->enablePTTSounds,
        ini
    );

    settings_write_bool(
        "General",
        "EnablePTTModeSounds",
        state->enablePTTModeSounds,
        ini
    );

    settings_write_bool(
        "Sounds",
        "VolumeTick",
        state->enableVolumeTickSound,
        ini
    );

    settings_write_bool(
        "General",
        "EnableBalloonTips",
        state->enableBalloonTips,
        ini
    );

    settings_write_string(
        "General",
        "MicSoundOn",
        state->micSoundOn,
        ini
    );

    settings_write_string(
        "General",
        "MicSoundOff",
        state->micSoundOff,
        ini
    );

    settings_write_string(
        "General",
        "PTTSoundOn",
        state->pttSoundOn,
        ini
    );

    settings_write_string(
        "General",
        "PTTSoundOff",
        state->pttSoundOff,
        ini
    );

    settings_write_uint(
        "MicPlus",
        "IconMode",
        (UINT)state->iconMode,
        ini
    );

    settings_write_bool(
        "MicPlus",
        "UseAudioIcon",
        state->useAudioIcon,
        ini
    );

    settings_write_uint(
        "MicPlus",
        "MenuTheme",
        (UINT)state->menuTheme,
        ini
    );

    settings_write_uint(
        "MicPlus",
        "LeftClickAction",
        (UINT)state->leftClickAction,
        ini
    );

    settings_write_uint(
        "MicPlus",
        "MiddleClickAction",
        (UINT)state->middleClickAction,
        ini
    );

    settings_write_uint(
        "MicPlus",
        "ScrollAction",
        (UINT)state->scrollAction,
        ini
    );

       settings_write_bool(
        "General",
        "HideMicSelection",
        state->hideMicSelection,
        ini
    );
}