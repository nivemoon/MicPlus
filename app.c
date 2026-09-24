#include "unicode.h"
#include <commctrl.h>
#define _CRT_SECURE_NO_WARNINGS

#include "app.h"

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
#include <stdio.h>
#include <shobjidl.h>

#define MICPLUS_CLASS_NAME "MICPLUS_CLASS"
#define MICPLUS_MUTEX_NAME "MicPlusSingleInstanceMutex"

static AppContext g_context;
static HANDLE g_mutex = NULL;

static BOOL app_create_mutex(void)
{
    g_mutex = CreateMutexA(NULL, TRUE, MICPLUS_MUTEX_NAME);
    if (!g_mutex) return FALSE;
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(g_mutex);
        g_mutex = NULL;
        return FALSE;
    }
    return TRUE;
}

static void app_close_mutex(void)
{
    if (!g_mutex) return;
    ReleaseMutex(g_mutex);
    CloseHandle(g_mutex);
    g_mutex = NULL;
}


static void app_play_mic_sound(int mute)
{
    sounds_play_mic(
        g_context.paths.exePath,
        "sounds",
        mute ? g_context.state.micSoundOff : g_context.state.micSoundOn,
        g_context.state.enableMicSounds
    );
}

static void app_update_mic_state(int mute, int playSound, int showBalloon)
{
    AppState *state = &g_context.state;

    microphone_run_ctl(g_context.paths.exePath, mute ? 1 : 0);

    state->muted = mute ? 1 : 0;

    if (playSound) app_play_mic_sound(mute);

    tray_update_tooltip(state);

    if (showBalloon) tray_show_mic_balloon(state, state->muted);
}

static void app_toggle_mic(void)
{
    app_update_mic_state(!g_context.state.muted, TRUE, TRUE);
}

static void app_toggle_system_sound(void)
{
    system_sound_toggle(g_context.state.hMainWnd, &g_context.state.soundMuted);
    tray_update_tooltip(&g_context.state);
}

static void app_shutdown(void)
{
    hooks_uninstall(&g_context);
    
    settings_save(&g_context.state, &g_context.paths);
    
    tray_remove_icon(&g_context.state);
    app_close_mutex();
}

static LRESULT CALLBACK app_window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
                    g_context.state.pttMode = !g_context.state.pttMode;
                    if (g_context.state.pttMode) {
                        app_update_mic_state(1, FALSE, FALSE);
                    }
                    settings_save(&g_context.state, &g_context.paths);
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
                    g_context.state.hideTrayIcon = !g_context.state.hideTrayIcon;
                    
                    if (g_context.state.hideTrayIcon) {
                        tray_remove_icon(&g_context.state);
                    } else {
                        tray_add_icon(&g_context.state, g_context.state.hMainWnd);
                    }
                    
                    settings_save(&g_context.state, &g_context.paths);
                    break;
            }
            return 0;

        case WM_SETTINGCHANGE:
            if (g_context.state.iconMode == 0) {
                theme_update_icon(&g_context.state.hIconOnCurrent, g_context.state.hIconOnLight, g_context.state.hIconOnDark, g_context.state.iconMode);
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

    if (!RegisterClassExA(&windowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return FALSE;
    }

    hWnd = CreateWindowExA(WS_EX_TOOLWINDOW | WS_EX_LAYERED, MICPLUS_CLASS_NAME, "MicPlusHidden", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!hWnd) return FALSE;

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

    if (!InitCommonControlsEx(&commonControls)) return FALSE;

    g_context.state.hInst = hInstance;
    paths_build(&g_context.paths);
    settings_load(&g_context.state, &g_context.paths);
    g_context.state.autoStart = startup_is_enabled(&g_context.paths);

    if (!app_create_hidden_window(hInstance)) return FALSE;

    if (!g_context.state.hideTrayIcon) {
        if (!tray_add_icon(&g_context.state, g_context.state.hMainWnd)) return FALSE;
    }
    
    RegisterHotKey(
        g_context.state.hMainWnd,
        ID_HK_HIDE_TRAY,
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        'M'
    );
    
    if (!hooks_install(&g_context)) {
        message_box_utf8(NULL, localization_get(g_context.state.lang, "MSG_ERR_HOOK"), "MicPlus", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    return TRUE;
}

int app_run(HINSTANCE hInstance)
{
    MSG message;

    if (!app_create_mutex()) return 0;
    if (!app_initialize(hInstance)) {
        app_shutdown();
        return 1;
    }

    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    app_update_mic_state(0, FALSE, FALSE);
    app_shutdown();

    return 0;
}

#include "updater.h"