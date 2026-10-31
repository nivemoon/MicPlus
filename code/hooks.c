#include "hooks.h"

#include "microphone.h"
#include "sounds.h"
#include "system_sound.h"
#include "tray.h"
#include "passive_ptt.h"
#include "settings.h"
#include "voice_activation.h"

#include <stdio.h>

static AppContext *g_hooksContext = NULL;
static BOOL pttLatched = FALSE;

static void hooks_update_voice_activation(void);

static BOOL hooks_is_key_down(WPARAM message)
{
    return message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
}

static BOOL hooks_is_key_up(WPARAM message)
{
    return message == WM_KEYUP || message == WM_SYSKEYUP;
}

static UINT hooks_get_modifiers(void)
{
    UINT modifiers = 0;
    BOOL rightAlt = (GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;

    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
        modifiers |= MOD_CONTROL;
    }

    if (GetAsyncKeyState(VK_MENU) & 0x8000) {
        modifiers |= MOD_ALT;
    }

    if (rightAlt) {
        modifiers &= ~MOD_CONTROL;
    }

    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        modifiers |= MOD_SHIFT;
    }

    return modifiers;
}

static BOOL hooks_set_mic_state(int mute)
{
    AppContext *context = g_hooksContext;

    if (!context || !context->state.hMainWnd) {
        return FALSE;
    }

    PostMessage(
        context->state.hMainWnd,
        WM_APP_MIC_STATE,
        (WPARAM)mute,
        0
    );

    context->state.muted = mute ? 1 : 0;

    return TRUE;
}

static void hooks_play_ptt_sound(int start)
{
    AppContext *context = g_hooksContext;

    if (!context) {
        return;
    }

    sounds_play_ptt(
        context->paths.exePath,
        sounds_get_directory(&g_hooksContext->state),
        start ? context->state.pttSoundOn : context->state.pttSoundOff,
        context->state.enablePTTSounds,
        context->state.soundVolume
    );
}

static void hooks_update_ptt_state(AppState *state)
{
    BOOL active = state->pttButtonActive ||
                  state->pttKeyDown ||
                  state->voiceActive ||
                  pttLatched;

    if (active && !state->pttActive) {
        state->pttActive = 1;
        state->micSoundFromPtt = 1;
        hooks_set_mic_state(0);
        hooks_play_ptt_sound(1);
    } else if (!active && state->pttActive) {
        state->pttActive = 0;
        state->micSoundFromPtt = 1;
        hooks_set_mic_state(1);
        hooks_play_ptt_sound(0);
    }
}

static void hooks_voice_activation_callback(BOOL active)
{
    AppState *state = &g_hooksContext->state;

    state->voiceActive = active ? 1 : 0;
    hooks_update_ptt_state(state);
}

static void hooks_update_voice_activation(void)
{
    AppState *state = &g_hooksContext->state;
    BOOL shouldRun = state->pttMode &&
                     state->voiceActivationEnabled;
    BOOL running = voice_activation_is_running();

    if (shouldRun && !running) {
        voice_activation_start();

        voice_activation_set_threshold(
            state->voiceThresholdX1000 / 1000.0);
        voice_activation_set_hangover_ms(
            (DWORD)state->voiceHangoverMs);

        PostMessage(
            state->hMainWnd,
            WM_APP_VAD_STATE,
            (WPARAM)1,
            0
        );
    } else if (!shouldRun && running) {
        voice_activation_stop();

        PostMessage(
            state->hMainWnd,
            WM_APP_VAD_STATE,
            (WPARAM)0,
            0
        );
    }
}

static void hooks_toggle_mic(void)
{
    AppState *state = &g_hooksContext->state;

    hooks_set_mic_state(!state->muted);
}

LRESULT CALLBACK hooks_mouse_proc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    MSLLHOOKSTRUCT *mouseData;
    UINT button;

    if (nCode == HC_ACTION &&
        g_hooksContext &&
        g_hooksContext->state.pttMode &&
        lParam) {
        mouseData = (MSLLHOOKSTRUCT *)lParam;

        if (wParam == WM_XBUTTONDOWN ||
            wParam == WM_XBUTTONUP) {
            button = HIWORD(mouseData->mouseData);

            if (button == g_hooksContext->state.pttButton) {
                if (wParam == WM_XBUTTONDOWN) {
                    g_hooksContext->state.pttButtonActive = 1;
                } else {
                    g_hooksContext->state.pttButtonActive = 0;
                }

                hooks_update_ptt_state(&g_hooksContext->state);

                return 1;
            }
        }
    }

    return CallNextHookEx(
        g_hooksContext ? g_hooksContext->state.hMouseHook : NULL,
        nCode,
        wParam,
        lParam
    );
}

void hooks_ptt_unlatch(void)
{
    if (!g_hooksContext) {
        return;
    }

    if (pttLatched) {
        pttLatched = FALSE;
        hooks_update_ptt_state(&g_hooksContext->state);
    }
}

static void hooks_process_ptt_mode_chord(
    AppState *state,
    UINT virtualKey,
    WPARAM message)
{
    static BOOL firstKeyDown = FALSE;
    static BOOL secondKeyDown = FALSE;

    UINT firstKey = state->vkPttModeChordFirst;
    UINT secondKey = state->vkPttModeChordSecond;

    if (firstKey == 0 || secondKey == 0) {
        firstKeyDown = FALSE;
        secondKeyDown = FALSE;
        state->pttModeChordKeyDown = 0;
        return;
    }

    if (hooks_is_key_down(message)) {
        if (virtualKey == firstKey) {
            firstKeyDown = TRUE;
        } else if (virtualKey == secondKey) {
            secondKeyDown = TRUE;
        }

        if (firstKeyDown && secondKeyDown &&
            !state->pttModeChordKeyDown) {
            state->pttModeChordKeyDown = 1;

            state->pttMode = !state->pttMode;
            hooks_update_voice_activation();

            tray_show_ptt_mode_balloon(state, state->pttMode);
            sounds_play_ptt_mode(
                g_hooksContext->paths.exePath,
                sounds_get_directory(state),
                state->pttMode ? "ptt_mode_on.mp3" : "ptt_mode_off.mp3",
                state->enablePTTModeSounds,
                state->soundVolume
            );
            settings_save(state, &g_hooksContext->paths);
            tray_update_tooltip(state);
        }
    } else if (hooks_is_key_up(message)) {
        if (virtualKey == firstKey) {
            firstKeyDown = FALSE;
        } else if (virtualKey == secondKey) {
            secondKeyDown = FALSE;
        }

        if (!firstKeyDown && !secondKeyDown) {
            state->pttModeChordKeyDown = 0;
        }
    }
}

static void hooks_process_mic_chord(
    AppState *state,
    UINT virtualKey,
    WPARAM message)
{
    static BOOL firstKeyDown = FALSE;
    static BOOL secondKeyDown = FALSE;

    UINT firstKey = state->vkMicChordFirst;
    UINT secondKey = state->vkMicChordSecond;

    if (firstKey == 0 || secondKey == 0) {
        firstKeyDown = FALSE;
        secondKeyDown = FALSE;
        state->micChordKeyDown = 0;
        return;
    }

    if (hooks_is_key_down(message)) {
        if (virtualKey == firstKey) {
            firstKeyDown = TRUE;
        } else if (virtualKey == secondKey) {
            secondKeyDown = TRUE;
        }

        if (firstKeyDown && secondKeyDown &&
            !state->micChordKeyDown) {
            state->micChordKeyDown = 1;
            hooks_toggle_mic();
        }
    } else if (hooks_is_key_up(message)) {
        if (virtualKey == firstKey) {
            firstKeyDown = FALSE;
        } else if (virtualKey == secondKey) {
            secondKeyDown = FALSE;
        }

        if (!firstKeyDown && !secondKeyDown) {
            state->micChordKeyDown = 0;
        }
    }
}

LRESULT CALLBACK hooks_keyboard_proc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    static int soundKeyDown = 0;
    static BOOL pttModeKeyDown = FALSE;
    static BOOL duskKeyDown = FALSE;
    static BOOL voiceChordVDown = FALSE;
    static BOOL voiceChordUDown = FALSE;
    static BOOL voiceChordFired = FALSE;

    static BOOL emergencyKeyDown = FALSE;

    KBDLLHOOKSTRUCT *keyboardData;
    AppState *state;
    UINT modifiers;

    if (nCode != HC_ACTION ||
        !g_hooksContext ||
        !lParam) {
        return CallNextHookEx(
            g_hooksContext ? g_hooksContext->state.hKeyboardHook : NULL,
            nCode,
            wParam,
            lParam
        );
    }

    keyboardData = (KBDLLHOOKSTRUCT *)lParam;
    state = &g_hooksContext->state;
    modifiers = hooks_get_modifiers();

    if (passive_ptt_process_key(keyboardData, wParam) ==
        PASSIVE_ACTION_TOGGLE_MIC) {
        pttLatched = !pttLatched;
        hooks_update_ptt_state(state);
    }

    if (keyboardData->vkCode == state->vkPttModeToggle) {
        if (hooks_is_key_down(wParam)) {
            UINT currentModifiers = hooks_get_modifiers();

            if (currentModifiers == state->modPttModeToggle &&
                !pttModeKeyDown) {
                pttModeKeyDown = TRUE;

                state->pttMode = !state->pttMode;
                hooks_update_voice_activation();

                tray_show_ptt_mode_balloon(state, state->pttMode);
                sounds_play_ptt_mode(
                    g_hooksContext->paths.exePath,
                    sounds_get_directory(state),
                    state->pttMode ? "ptt_mode_on.mp3" : "ptt_mode_off.mp3",
                    state->enablePTTModeSounds,
                    state->soundVolume
                );
                settings_save(state, &g_hooksContext->paths);
                tray_update_tooltip(state);

                return 1;
            }
        } else if (hooks_is_key_up(wParam)) {
            pttModeKeyDown = FALSE;
        }
    }

    if (keyboardData->vkCode == VK_F9) {
        if (hooks_is_key_down(wParam)) {
            if (modifiers == MOD_CONTROL && !soundKeyDown) {
                soundKeyDown = 1;

                system_sound_toggle(
                    state->hMainWnd,
                    &state->soundMuted
                );

                tray_update_tooltip(state);
                return 1;
            }
        }

        if (hooks_is_key_up(wParam)) {
            if (soundKeyDown) {
                soundKeyDown = 0;
                return 1;
            }
        }
    }

    if (state->vkPttCustom != 0 &&
        state->pttMode &&
        keyboardData->vkCode == state->vkPttCustom &&
        !(passive_ptt_is_enabled() &&
          passive_ptt_get_double_vk() ==
              keyboardData->vkCode)) {
        if (hooks_is_key_down(wParam)) {
            if (!state->pttKeyDown) {
                state->pttKeyDown = 1;
                hooks_update_ptt_state(state);
            }
        } else if (hooks_is_key_up(wParam)) {
            if (state->pttKeyDown) {
                state->pttKeyDown = 0;
                hooks_update_ptt_state(state);
            }
        }
    }

    if (state->vkPttModeChordFirst != 0 &&
        state->vkPttModeChordSecond != 0) {
        hooks_process_ptt_mode_chord(
            state,
            keyboardData->vkCode,
            wParam
        );
    }

    if (state->vkMicChordFirst != 0 &&
        state->vkMicChordSecond != 0) {
        hooks_process_mic_chord(
            state,
            keyboardData->vkCode,
            wParam
        );
    }

    if (keyboardData->vkCode == state->vkToggle) {
        if (hooks_is_key_down(wParam)) {
            if (!state->toggleKeyDown &&
                modifiers == state->modToggle) {
                state->toggleKeyDown = 1;
                hooks_toggle_mic();
            }
        } else if (hooks_is_key_up(wParam)) {
            state->toggleKeyDown = 0;
        }
    }

    /* Голосовой режим: V+U одновременно (тогл) */
    if (keyboardData->vkCode == 'V') {
        if (hooks_is_key_down(wParam)) {
            voiceChordVDown = TRUE;
        } else {
            voiceChordVDown = FALSE;
            voiceChordFired = FALSE;
        }
    }

    if (keyboardData->vkCode == 'U') {
        if (hooks_is_key_down(wParam)) {
            voiceChordUDown = TRUE;
        } else {
            voiceChordUDown = FALSE;
            voiceChordFired = FALSE;
        }
    }

    if (voiceChordVDown &&
        voiceChordUDown &&
        !voiceChordFired) {

        voiceChordFired = TRUE;

        state->voiceActivationEnabled =
            !state->voiceActivationEnabled;

        hooks_update_voice_activation();
        settings_save(state, &g_hooksContext->paths);
        tray_update_tooltip(state);
    }

    /* DUSK-хоткеи: панель / список / вывод / ввод */
    if (hooks_is_key_down(wParam) && !duskKeyDown) {
        UINT vk = keyboardData->vkCode;

        if (state->panelHotkeyVk != 0 &&
            vk == state->panelHotkeyVk &&
            modifiers == state->panelHotkeyMods) {
            duskKeyDown = TRUE;

            PostMessage(
                state->hMainWnd,
                WM_APP_DUSK_PANEL,
                0,
                0
            );

            return 1;
        }

        if (state->listHotkeyVk != 0 &&
            vk == state->listHotkeyVk &&
            modifiers == state->listHotkeyMods) {
            duskKeyDown = TRUE;

            PostMessage(
                state->hMainWnd,
                WM_APP_DUSK_LIST,
                0,
                0
            );

            return 1;
        }

        if (state->outputHotkeyVk != 0 &&
            vk == state->outputHotkeyVk &&
            modifiers == state->outputHotkeyMods) {
            duskKeyDown = TRUE;

            PostMessage(
                state->hMainWnd,
                WM_APP_DUSK_OUTPUT,
                0,
                0
            );

            return 1;
        }

        if (state->inputHotkeyVk != 0 &&
            vk == state->inputHotkeyVk &&
            modifiers == state->inputHotkeyMods) {
            duskKeyDown = TRUE;

            PostMessage(
                state->hMainWnd,
                WM_APP_DUSK_INPUT,
                0,
                0
            );

            return 1;
        }
    } else if (hooks_is_key_up(wParam)) {
        duskKeyDown = FALSE;
    }

    /* Аварийное отключение: Ctrl+Alt+F10 (глобально) */
    if (keyboardData->vkCode == VK_F10) {
        if (hooks_is_key_down(wParam) &&
            modifiers == (MOD_CONTROL | MOD_ALT) &&
            !emergencyKeyDown) {

            emergencyKeyDown = TRUE;

            PostMessage(
                state->hMainWnd,
                WM_APP_EMERGENCY,
                0,
                0
            );
        } else if (hooks_is_key_up(wParam)) {
            emergencyKeyDown = FALSE;
        }
    }

    return CallNextHookEx(
        state->hKeyboardHook,
        nCode,
        wParam,
        lParam
    );
}

BOOL hooks_install(AppContext *context)
{
    AppState *state;
    DWORD mouseError = 0;
    DWORD keyboardError = 0;

    if (!context || !context->state.hInst) {
        return FALSE;
    }

    g_hooksContext = context;
    state = &context->state;

    state->hMouseHook = SetWindowsHookExA(
        WH_MOUSE_LL,
        hooks_mouse_proc,
        GetModuleHandle(NULL),
        0
    );

    state->hKeyboardHook = SetWindowsHookExA(
        WH_KEYBOARD_LL,
        hooks_keyboard_proc,
        GetModuleHandle(NULL),
        0
    );

    if (!state->hMouseHook) {
        mouseError = GetLastError();
    }

    if (!state->hKeyboardHook) {
        keyboardError = GetLastError();
    }

    if (!state->hMouseHook || !state->hKeyboardHook) {
        char debug[256];

        snprintf(
            debug,
            sizeof(debug),
            "Mouse hook: %p (err %lu)\nKeyboard hook: %p (err %lu)",
            (void*)state->hMouseHook,
            (unsigned long)mouseError,
            (void*)state->hKeyboardHook,
            (unsigned long)keyboardError
        );

        MessageBoxA(
            NULL,
            debug,
            "MicPlus hooks debug",
            MB_OK | MB_ICONERROR
        );

        hooks_uninstall(context);
        return FALSE;
    }

    voice_activation_set_callback(
        hooks_voice_activation_callback
    );

    hooks_update_voice_activation();

    return TRUE;
}

void hooks_uninstall(AppContext *context)
{
    voice_activation_stop();

    if (!context) {
        return;
    }

    if (context->state.hMouseHook) {
        UnhookWindowsHookEx(context->state.hMouseHook);
        context->state.hMouseHook = NULL;
    }

    if (context->state.hKeyboardHook) {
        UnhookWindowsHookEx(context->state.hKeyboardHook);
        context->state.hKeyboardHook = NULL;
    }

    if (g_hooksContext == context) {
        g_hooksContext = NULL;
    }
}

void hooks_set_voice_activation(int enabled)
{
    AppState *state = &g_hooksContext->state;

    state->voiceActivationEnabled = enabled ? 1 : 0;
    hooks_update_voice_activation();

    settings_save(state, &g_hooksContext->paths);
    tray_update_tooltip(state);
}