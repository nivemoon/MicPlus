#include "unicode.h"
#include <commctrl.h>
#include <ctype.h>
#define _CRT_SECURE_NO_WARNINGS
#define IDM_AUDIO_DEVICE_BASE 3000
#define IDM_AUDIO_INPUT_BASE 3100
#define ID_AUDIO_HIDE_MIC 3400

#include "app.h"
#include <stdio.h>
#include "app_types.h"
#include "audio_icons.h"
#include "audio_device_type.h"
#include "hooks.h"
#include "passive_ptt.h"
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
#include "keep_alive.h"
#include "audio_device.h"
#include "switch_devices.h"
#include "audio_device_events.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <shobjidl.h>
#include "micplus_ini.h"
#include "multimedia.h"
#include "multimedia_window.h"
#include "alias.h"

#define MICPLUS_CLASS_NAME "MICPLUS_CLASS"
#define MICPLUS_MUTEX_NAME "MicPlusSingleInstanceMutex"

#ifndef ID_KEEP_ALIVE_TOGGLE
#define ID_KEEP_ALIVE_TOGGLE 5460
#endif

static AppContext g_context;
static HANDLE g_mutex = NULL;

static void app_play_volume_tick(void);
static const char *app_get_sounds_directory(void);

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

static void app_emergency_off(void)
{
    g_context.state.muted = 1;
    g_context.state.pttMode = 0;
    g_context.state.voiceActivationEnabled = 0;

    g_context.state.micSoundFromPtt = 1;

    PostMessage(
        g_context.state.hMainWnd,
        WM_APP_MIC_STATE,
        (WPARAM)1,
        0
    );

    sounds_play_volume_tick(
        g_context.paths.exePath,
        app_get_sounds_directory(),
        "volume_tick.mp3",
        1,
        20
    );

    settings_save(&g_context.state, &g_context.paths);
    tray_update_tooltip(&g_context.state);
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
    return "sounds";
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

static DWORD WINAPI app_play_mic_sound_thread(LPVOID param)
{
    int mute = (int)(intptr_t)param;

    app_play_mic_sound(mute);

    return 0;
}

static DWORD WINAPI app_play_vad_sound_thread(LPVOID param)
{
    (void)param;

    sounds_play_mic(
        g_context.paths.exePath,
        app_get_sounds_directory(),
        "mic_off.mp3",
        1,
        g_context.state.soundVolume
    );

    return 0;
}

static void app_play_volume_tick(void)
{
    sounds_play_volume_tick(
        g_context.paths.exePath,
        app_get_sounds_directory(),
        "volume_tick.mp3",
        g_context.state.enableVolumeTickSound,
        30
    );
}

static DWORD WINAPI app_play_ptt_sound_thread(LPVOID param)
{
    AppState *state = &g_context.state;
    int on = (int)(intptr_t)param;

    sounds_play_ptt_mode(
        g_context.paths.exePath,
        sounds_get_directory(state),
        on ? "ptt_mode_on.mp3" : "ptt_mode_off.mp3",
        state->enablePTTModeSounds,
        state->soundVolume
    );

    return 0;
}

static DWORD WINAPI app_mic_toggle_thread(LPVOID param)
{
    int mute = (int)(intptr_t)param;

    microphone_run_ctl(g_context.paths.exePath, mute);

    return 0;
}

static void app_toggle_mic(void)
{
    int mute = !g_context.state.muted;

    PostMessage(
        g_context.state.hMainWnd,
        WM_APP_MIC_STATE,
        (WPARAM)mute,
        0
    );
}

static void app_toggle_ptt_mode(void)
{
    AppState *state = &g_context.state;

    state->pttMode = !state->pttMode;

    CreateThread(
        NULL,
        0,
        app_play_ptt_sound_thread,
        (LPVOID)(intptr_t)state->pttMode,
        0,
        NULL
    );

    settings_save(state, &g_context.paths);
    tray_update_tooltip(state);
}

static DWORD WINAPI app_system_sound_thread(LPVOID param)
{
    (void)param;
    system_sound_toggle(
        g_context.state.hMainWnd,
        &g_context.state.soundMuted
    );

    tray_update_tooltip(&g_context.state);

    return 0;
}

static void app_toggle_system_sound(void)
{
    CreateThread(
        NULL,
        0,
        app_system_sound_thread,
        NULL,
        0,
        NULL
    );
}

static void app_cycle_output(void)
{
    SwitchDeviceList devices;
    int i;
    int next = -1;

    if (!switch_devices_get_output(&devices) || devices.count < 2) {
        return;
    }

    for (i = 0; i < devices.count; i++) {
        if (devices.devices[i].isDefault) {
            next = (i + 1) % devices.count;
            break;
        }
    }

    if (next < 0) {
        next = 0;
    }

    switch_devices_set_default(devices.devices[next].id);
    tray_update_tooltip(&g_context.state);
}

static void app_cycle_input(void)
{
    SwitchDeviceList devices;
    int i;
    int next = -1;

    if (!switch_devices_get_input(&devices) || devices.count < 2) {
        return;
    }

    for (i = 0; i < devices.count; i++) {
        if (devices.devices[i].isDefault) {
            next = (i + 1) % devices.count;
            break;
        }
    }

    if (next < 0) {
        next = 0;
    }

    switch_devices_set_default_input(devices.devices[next].id);
}

static void app_show_device_list(HWND hWnd)
{
    POINT pt;
    HMENU menu;
    SwitchDeviceList outDevices;
    SwitchDeviceList inDevices;
    int i;
    UINT flags;
    const wchar_t *alias;
    const wchar_t *label;

    if (!GetCursorPos(&pt)) {
        return;
    }

    menu = CreatePopupMenu();

    if (switch_devices_get_output(&outDevices)) {
        for (i = 0; i < outDevices.count; i++) {
            flags = MF_STRING;
            if (outDevices.devices[i].isDefault) {
                flags |= MF_CHECKED;
            }

            alias = alias_get(outDevices.devices[i].id);
            label = alias ? alias : outDevices.devices[i].name;

            AppendMenuW(menu, flags,
                IDM_AUDIO_DEVICE_BASE + i, label);
        }
    }

    if (switch_devices_get_input(&inDevices) && inDevices.count > 0) {
        AppendMenuW(menu, MF_SEPARATOR, 0, NULL);

        for (i = 0; i < inDevices.count; i++) {
            flags = MF_STRING;
            if (inDevices.devices[i].isDefault) {
                flags |= MF_CHECKED;
            }

            alias = alias_get(inDevices.devices[i].id);
            label = alias ? alias : inDevices.devices[i].name;

            AppendMenuW(menu, flags,
                IDM_AUDIO_INPUT_BASE + i, label);
        }
    }

    SetForegroundWindow(hWnd);

    TrackPopupMenu(menu,
        TPM_RIGHTBUTTON | TPM_RETURNCMD,
        pt.x, pt.y, 0, hWnd, NULL);

    SendMessageA(hWnd, WM_NULL, 0, 0);

    DestroyMenu(menu);
}

static void app_shutdown(void)
{
    keep_alive_stop();

    hooks_uninstall(&g_context);

    UnregisterHotKey(g_context.state.hMainWnd, ID_HK_HIDE_TRAY);

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
        case WM_APP_MIC_STATE: {
            int mute = (int)wParam;

            g_context.state.muted = mute ? 1 : 0;

            if (!(g_context.state.voiceActivationEnabled &&
                  g_context.state.pttMode)) {
                CreateThread(
                    NULL,
                    0,
                    app_mic_toggle_thread,
                    (LPVOID)(intptr_t)mute,
                    0,
                    NULL
                );
            }

            if (g_context.state.micSoundFromPtt) {
                g_context.state.micSoundFromPtt = 0;
            } else {
                CreateThread(
                    NULL,
                    0,
                    app_play_mic_sound_thread,
                    (LPVOID)(intptr_t)mute,
                    0,
                    NULL
                );
            }

            tray_update_tooltip(&g_context.state);
            tray_show_mic_balloon(&g_context.state, mute);

            return 0;
        }

        case WM_APP_VAD_STATE: {
            int unmute = (int)wParam;

            CreateThread(
                NULL,
                0,
                app_mic_toggle_thread,
                (LPVOID)(intptr_t)(!unmute),
                0,
                NULL
            );

            CreateThread(
                NULL,
                0,
                app_play_vad_sound_thread,
                NULL,
                0,
                NULL
            );

            return 0;
        }

        case WM_APP_EMERGENCY:
            app_emergency_off();
            return 0;

        case WM_APP_DUSK_PANEL:
            multimedia_window_show(hWnd);
            return 0;

        case WM_APP_DUSK_LIST:
            app_show_device_list(hWnd);
            return 0;

        case WM_APP_DUSK_OUTPUT:
            app_cycle_output();
            return 0;

        case WM_APP_DUSK_INPUT:
            app_cycle_input();
            return 0;

        case WM_APP_DEVICE_CHANGED:
            tray_update_tooltip(&g_context.state);
            return 0;

        case WM_APP_TRAY: {
            UINT trayMsg = (UINT)lParam;

            switch (trayMsg) {
                case WM_RBUTTONUP:
                case WM_CONTEXTMENU:
                    tray_menu_show(&g_context, hWnd);
                    break;

                case WM_LBUTTONUP:
                case WM_LBUTTONDBLCLK:
                    if (g_context.state.leftClickAction == 0) {
                        multimedia_show(
                            hWnd,
                            g_context.state.hideMicSelection
                        );
                    } else if (g_context.state.leftClickAction == 1) {
                        tray_menu_show(&g_context, hWnd);
                    }
                    break;

                case WM_MBUTTONUP:
                    if (g_context.state.middleClickAction == 0) {
                        app_toggle_mic();
                    } else if (g_context.state.middleClickAction == 1) {
                        multimedia_window_show(hWnd);
                    }
                    break;

                case WM_MOUSEWHEEL: {
                    int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                    int i;

                    if (delta > 0) {
                        for (i = 0; i < 3; i++) {
                            keybd_event(VK_VOLUME_UP, 0, 0, 0);
                            keybd_event(VK_VOLUME_UP, 0, KEYEVENTF_KEYUP, 0);
                        }
                    } else if (delta < 0) {
                        for (i = 0; i < 3; i++) {
                            keybd_event(VK_VOLUME_DOWN, 0, 0, 0);
                            keybd_event(VK_VOLUME_DOWN, 0, KEYEVENTF_KEYUP, 0);
                        }
                    }

                    break;
                }
            }

            return 0;
        }

        case WM_COMMAND: {
            int cmd = LOWORD(wParam);

            if (cmd >= IDM_AUDIO_DEVICE_BASE &&
                cmd < IDM_AUDIO_DEVICE_BASE + SWITCH_DEVICES_MAX) {

                SwitchDeviceList devices;
                int index = cmd - IDM_AUDIO_DEVICE_BASE;

                if (switch_devices_get_output(&devices) &&
                    index < devices.count) {

                    switch_devices_set_default(
                        devices.devices[index].id
                    );

                    tray_update_tooltip(&g_context.state);
                }

                return 0;
            }

            if (cmd >= IDM_AUDIO_INPUT_BASE &&
                cmd < IDM_AUDIO_INPUT_BASE + SWITCH_DEVICES_MAX) {

                SwitchDeviceList devices;
                int index = cmd - IDM_AUDIO_INPUT_BASE;

                if (switch_devices_get_input(&devices) &&
                    index < devices.count) {

                    switch_devices_set_default_input(
                        devices.devices[index].id
                    );
                }

                return 0;
            }

            if (cmd >= 9200 && cmd < 9200 + SWITCH_DEVICES_MAX) {
                SwitchDeviceList devices;
                int index = cmd - 9200;

                if (switch_devices_get_output(&devices) &&
                    index < devices.count) {

                    switch_devices_set_default(devices.devices[index].id);
                    tray_update_tooltip(&g_context.state);
                }

                return 0;
            }

            if (cmd >= 9300 && cmd < 9300 + SWITCH_DEVICES_MAX) {
                SwitchDeviceList devices;
                int index = cmd - 9300;

                if (switch_devices_get_input(&devices) &&
                    index < devices.count) {

                    switch_devices_set_default_input(devices.devices[index].id);
                    tray_update_tooltip(&g_context.state);
                }

                return 0;
            }

            switch (cmd) {
                case ID_MOUSE_LEFT_PANEL:
                    g_context.state.leftClickAction = 0;
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_MOUSE_LEFT_MENU:
                    g_context.state.leftClickAction = 1;
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_MOUSE_LEFT_NONE:
                    g_context.state.leftClickAction = 2;
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_MOUSE_MIDDLE_MIC:
                    g_context.state.middleClickAction = 0;
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_MOUSE_MIDDLE_PANEL:
                    g_context.state.middleClickAction = 1;
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_MOUSE_MIDDLE_NONE:
                    g_context.state.middleClickAction = 2;
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_ICON_MODE_MICPLUS:
                    g_context.state.iconMode = 0;
                    settings_save(&g_context.state, &g_context.paths);
                    tray_update_tooltip(&g_context.state);
                    return 0;

                case ID_ICON_MODE_AUDIO:
                    g_context.state.iconMode = 1;
                    settings_save(&g_context.state, &g_context.paths);
                    tray_update_tooltip(&g_context.state);
                    return 0;

                case ID_ICON_MODE_TWO:
                    g_context.state.iconMode = 2;
                    settings_save(&g_context.state, &g_context.paths);
                    tray_update_tooltip(&g_context.state);
                    return 0;

                case ID_MOUSE_WHEEL_SYSTEM:
                    g_context.state.scrollAction = 0;
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_MOUSE_WHEEL_MICPLUS:
                    g_context.state.scrollAction = 1;
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_MOUSE_WHEEL_NONE:
                    g_context.state.scrollAction = 2;
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_SOUND_VOLUME_20:
                    g_context.state.soundVolume = 20;
                    app_play_volume_tick();
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_SOUND_VOLUME_40:
                    g_context.state.soundVolume = 40;
                    app_play_volume_tick();
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_SOUND_VOLUME_60:
                    g_context.state.soundVolume = 60;
                    app_play_volume_tick();
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_SOUND_VOLUME_80:
                    g_context.state.soundVolume = 80;
                    app_play_volume_tick();
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_SOUND_VOLUME_100:
                    g_context.state.soundVolume = 100;
                    app_play_volume_tick();
                    settings_save(&g_context.state, &g_context.paths);
                    return 0;

                case ID_AUDIO_HIDE_MIC:
                    g_context.state.hideMicSelection =
                        !g_context.state.hideMicSelection;

                    settings_save(
                        &g_context.state,
                        &g_context.paths
                    );
                    return 0;

case ID_KEEP_ALIVE_TOGGLE:
    g_context.state.keepHeadphonesAwake =
        !g_context.state.keepHeadphonesAwake;

    if (g_context.state.keepHeadphonesAwake) {
        keep_alive_start();

        if (!g_context.state.keepAliveBalloonShown) {
            tray_show_balloon(
                &g_context.state,
                "Наушники не будут засыпать",
                "MicPlus держит Bluetooth-наушники активными,\n"
                "чтобы убрать задержку перед началом звука.\n"
                "Батарея наушников будет расходоваться быстрее."
            );

            g_context.state.keepAliveBalloonShown = 1;
        }
    } else {
        keep_alive_stop();
    }

    settings_save(&g_context.state, &g_context.paths);
    return 0;

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
                    tray_menu_handle_command(&g_context, hWnd, cmd);
                    return 0;
            }
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
            audio_device_events_stop();
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

    IniInit();

    paths_build(&g_context.paths);
    passive_ptt_load();

    alias_set_path(
        g_context.paths.exePath,
        g_context.paths.appDataPath
    );

    settings_load(&g_context.state, &g_context.paths);
if (g_context.state.keepHeadphonesAwake) {
    keep_alive_start();
}
    audio_device_init();

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

    audio_device_events_start(g_context.state.hMainWnd);


    RegisterHotKey(
        g_context.state.hMainWnd,
        ID_HK_HIDE_TRAY,
        MOD_CONTROL | MOD_ALT | MOD_SHIFT,
        'M'
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
        MessageBoxA(NULL,
            "MicPlus is already running.",
            "MicPlus",
            MB_OK | MB_ICONINFORMATION);
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