#include "unicode.h"
#include <commctrl.h>
#include <ctype.h>
#define _CRT_SECURE_NO_WARNINGS

#include "app.h"
#include <stdio.h>
#include "app_types.h"
#include "hooks.h"
#include "localization.h"
#include "microphone.h"
#include "paths.h"
#include "settings.h"
#include "sounds.h"
#include "startup.h"
#include "system_sound.h"
#include "theme.h"
#include "tray.h"
#include "tray_menu.h"
#include "resource.h"
#include "audio_device.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <shobjidl.h>

#pragma comment(lib, "winmm.lib")

#define MICPLUS_CLASS_NAME "MICPLUS_CLASS"
#define MICPLUS_MUTEX_NAME "MicPlusSingleInstanceMutex"

#define ID_HOTKEY_SONG_D 2001
#define ID_HOTKEY_SONG_U 2002
#define ID_HOTKEY_SONG_S 2003
#define ID_HOTKEY_SONG_K 2004

#define MUSIC_ALIAS "MicPlusSong"

static AppContext g_context;
static HANDLE g_mutex = NULL;
static char g_currentSong = 0;

static BOOL app_create_mutex(void)
{
    g_mutex = CreateMutexA(NULL, TRUE, MICPLUS_MUTEX_NAME);

    if (!g_mutex) {
        return FALSE;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(g_mutex);
        g_mutex = NULL;
        return FALSE;
    }

    return TRUE;
}

static void app_close_mutex(void)
{
    if (!g_mutex) {
        return;
    }

    ReleaseMutex(g_mutex);
    CloseHandle(g_mutex);
    g_mutex = NULL;
}

static const char *app_get_sounds_directory(void)
{
    if (g_context.state.soundScheme == 1) {
        return "sounds\\micplus";
    }

    return "sounds\\classic";
}

static void app_play_mic_sound(int mute)
{
    sounds_play_mic(
    g_context.paths.exePath,
    app_get_sounds_directory(),
    mute ? g_context.state.micSoundOff : g_context.state.micSoundOn,
    g_context.state.enableMicSounds,
    g_context.state.soundVolume
);
}

static void app_update_mic_state(int mute, int playSound, int showBalloon)
{
    AppState *state = &g_context.state;

    audio_device_set_mic_mute(mute);

    state->muted = mute ? 1 : 0;

    if (playSound) {
        app_play_mic_sound(mute);
    }

    tray_update_tooltip(state);

    if (showBalloon) {
        tray_show_mic_balloon(state, state->muted);
    }
}

static void app_toggle_mic(void)
{
    app_update_mic_state(!g_context.state.muted, TRUE, TRUE);
}

static void app_toggle_ptt_mode(void)
{
    AppState *state = &g_context.state;

    state->pttMode = !state->pttMode;

    tray_show_ptt_mode_balloon(state, state->pttMode);

    sounds_play_ptt_mode(
    g_context.paths.exePath,
    sounds_get_directory(state),
    state->pttMode ? "ptt_mode_on.mp3" : "ptt_mode_off.mp3",
    state->enablePTTModeSounds,
    state->soundVolume
);

    settings_save(state, &g_context.paths);
    tray_update_tooltip(state);
}

static void app_toggle_system_sound(void)
{
    system_sound_toggle(
        g_context.state.hMainWnd,
        &g_context.state.soundMuted
    );

    tray_update_tooltip(&g_context.state);
}

static void app_get_assets_dir(char *buffer, size_t size)
{
    char *lastSlash;
    size_t dirLength;

    strncpy(buffer, g_context.paths.exePath, size - 1);
    buffer[size - 1] = '\0';

    lastSlash = strrchr(buffer, '\\');

    if (lastSlash) {
        *lastSlash = '\0';
    }

    dirLength = strlen(buffer);

    if (dirLength + 1 < size) {
        buffer[dirLength] = '\\';
        buffer[dirLength + 1] = '\0';
    }

    strncat(buffer, "assets", size - strlen(buffer) - 1);
}

static void app_stop_song(void)
{
    if (!g_currentSong) {
        return;
    }

    mciSendStringA("stop " MUSIC_ALIAS, NULL, 0, NULL);
    mciSendStringA("close " MUSIC_ALIAS, NULL, 0, NULL);

    g_currentSong = 0;
}

static void app_toggle_song(char key)
{
    char assetsDir[MAX_PATH];
    char openCommand[600];
    char songFile[16];

    if (g_currentSong == key) {
        app_stop_song();
        return;
    }

    app_stop_song();

    snprintf(songFile, sizeof(songFile), "%c.mp3", (char)toupper(key));

    app_get_assets_dir(assetsDir, sizeof(assetsDir));

    snprintf(
        openCommand,
        sizeof(openCommand),
        "open \"%s\\%s\" type mpegvideo alias " MUSIC_ALIAS,
        assetsDir,
        songFile
    );

    if (mciSendStringA(openCommand, NULL, 0, NULL) == 0 &&
        mciSendStringA("play " MUSIC_ALIAS, NULL, 0, NULL) == 0) {
        g_currentSong = key;
    } else {
        mciSendStringA("close " MUSIC_ALIAS, NULL, 0, NULL);
        g_currentSong = 0;
    }
}

static void app_shutdown(void)
{
    hooks_uninstall(&g_context);

        app_stop_song();

    UnregisterHotKey(g_context.state.hMainWnd, ID_HK_HIDE_TRAY);
    UnregisterHotKey(g_context.state.hMainWnd, ID_HOTKEY_SONG_D);
    UnregisterHotKey(g_context.state.hMainWnd, ID_HOTKEY_SONG_U);
    UnregisterHotKey(g_context.state.hMainWnd, ID_HOTKEY_SONG_S);
    UnregisterHotKey(g_context.state.hMainWnd, ID_HOTKEY_SONG_K);

    settings_save(&g_context.state, &g_context.paths);

    tray_remove_icon(&g_context.state);
	audio_device_shutdown();
    app_close_mutex();
}

static LRESULT CALLBACK app_window_proc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {
        case WM_APP_TRAY:
            switch ((UINT)lParam) {
                case WM_RBUTTONUP:
                case WM_CONTEXTMENU:
                case WM_LBUTTONUP:
                case WM_LBUTTONDBLCLK:
                    tray_menu_show(&g_context, hWnd);
                    break;
            }

            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_TRAY_PTT_MODE:
                    app_toggle_ptt_mode();
                    return 0;

                case ID_TRAY_MIC_TOGGLE:
                    app_toggle_mic();
                    return 0;

                case ID_TRAY_SOUND_TOGGLE:
                    app_toggle_system_sound();
                    return 0;

                case ID_TRAY_EXIT:
                    DestroyWindow(hWnd);
                    return 0;

                default:
                    tray_menu_handle_command(&g_context, hWnd, LOWORD(wParam));
                    return 0;
            }

        case WM_HOTKEY:
            switch ((int)wParam) {
                case ID_HK_HIDE_TRAY:
                    g_context.state.hideTrayIcon =
                        !g_context.state.hideTrayIcon;

                    if (g_context.state.hideTrayIcon) {
                        tray_remove_icon(&g_context.state);
                    } else {
                        tray_add_icon(
                            &g_context.state,
                            g_context.state.hMainWnd
                        );
                    }

                    settings_save(
                        &g_context.state,
                        &g_context.paths
                    );
                    break;

                                case ID_HOTKEY_SONG_D:
                    app_toggle_song('d');
                    break;

                case ID_HOTKEY_SONG_U:
                    app_toggle_song('u');
                    break;

                case ID_HOTKEY_SONG_S:
                    app_toggle_song('s');
                    break;

                case ID_HOTKEY_SONG_K:
                    app_toggle_song('k');
                    break;
            }

            return 0;

        case WM_SETTINGCHANGE:
            if (g_context.state.iconMode == 0) {
                theme_update_icon(
                    &g_context.state.hIconOnCurrent,
                    g_context.state.hIconOnLight,
                    g_context.state.hIconOnDark,
                    g_context.state.iconMode
                );

                tray_update_tooltip(&g_context.state);
            }

            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcA(hWnd, message, wParam, lParam);
}

static void app_set_user_model_id(void)
{
    SetCurrentProcessExplicitAppUserModelID(L"Nivemoon.MicPlus");
}

static BOOL app_create_hidden_window(HINSTANCE hInstance)
{
    WNDCLASSEXA windowClass;
    HWND hWnd;

    app_set_user_model_id();

    ZeroMemory(&windowClass, sizeof(windowClass));

    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = app_window_proc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = MICPLUS_CLASS_NAME;
    windowClass.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_APP_ICON));
    windowClass.hIconSm = windowClass.hIcon;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;

    if (!RegisterClassExA(&windowClass) &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return FALSE;
    }

    hWnd = CreateWindowExA(
        WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        MICPLUS_CLASS_NAME,
        "MicPlusHidden",
        WS_OVERLAPPEDWINDOW,
        0,
        0,
        0,
        0,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd) {
        return FALSE;
    }

    g_context.state.hMainWnd = hWnd;

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    return TRUE;
}

static BOOL app_initialize(HINSTANCE hInstance)
{
    INITCOMMONCONTROLSEX commonControls;

    ZeroMemory(&g_context, sizeof(g_context));

    commonControls.dwSize = sizeof(commonControls);
    commonControls.dwICC = ICC_LINK_CLASS;

    if (!InitCommonControlsEx(&commonControls)) {
        return FALSE;
    }

	g_context.state.hInst = hInstance;

    paths_build(&g_context.paths);
    settings_load(&g_context.state, &g_context.paths);

    /* Инициализация Core Audio */
    audio_device_init();

    /* Читаем РЕАЛЬНОЕ состояние микрофона из Windows */
    {
        int realMute = audio_device_get_mic_mute();

        if (realMute >= 0) {
            g_context.state.muted = realMute;
        }
    }

    g_context.state.autoStart = startup_is_enabled(&g_context.paths);

    if (!app_create_hidden_window(hInstance)) {
        return FALSE;
    }

    if (!g_context.state.hideTrayIcon) {
        if (!tray_add_icon(&g_context.state, g_context.state.hMainWnd)) {
            return FALSE;
        }
    }

    RegisterHotKey(
        g_context.state.hMainWnd,
        ID_HK_HIDE_TRAY,
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        'M'
    );

        RegisterHotKey(
        g_context.state.hMainWnd,
        ID_HOTKEY_SONG_D,
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        'D'
    );

    RegisterHotKey(
        g_context.state.hMainWnd,
        ID_HOTKEY_SONG_U,
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        'U'
    );

    RegisterHotKey(
        g_context.state.hMainWnd,
        ID_HOTKEY_SONG_S,
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        'S'
    );

    RegisterHotKey(
        g_context.state.hMainWnd,
        ID_HOTKEY_SONG_K,
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        'K'
    );

        if (!hooks_install(&g_context)) {
        char errorMessage[256];

        snprintf(
            errorMessage,
            sizeof(errorMessage),
            "%s\nError code: %lu",
            localization_get(g_context.state.lang, "MSG_ERR_HOOK"),
            (unsigned long)GetLastError()
        );

        message_box_utf8(
            NULL,
            errorMessage,
            "MicPlus",
            MB_OK | MB_ICONERROR
        );

        return FALSE;
    }

    return TRUE;
}

int app_run(HINSTANCE hInstance)
{
    MSG message;

        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    if (!app_create_mutex()) {
        CoUninitialize();
        return 0;
    }

    if (!app_initialize(hInstance)) {
        app_shutdown();
        CoUninitialize();
        return 1;
    }

    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    app_shutdown();

    CoUninitialize();

    return 0;
}