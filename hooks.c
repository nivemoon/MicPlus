#include "hooks.h"

#include "microphone.h"
#include "sounds.h"
#include "system_sound.h"
#include "tray.h"
#include "passive_ptt.h"
#include "settings.h"

#include <stdio.h>

static AppContext *g_hooksContext = NULL;

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
    BOOL rightAlt =
        (GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;

    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
        modifiers |= MOD_CONTROL;
    }

    if (GetAsyncKeyState(VK_MENU) & 0x8000) {
        modifiers |= MOD_ALT;
    }

    /* Правый Alt (AltGr) в русской раскладке
       Windows видит как Ctrl+Alt — убираем Ctrl,
       чтобы правый Alt работал как обычный Alt. */
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

    if (!context) {
        return FALSE;
    }

    if (!microphone_run_ctl(context->paths.exePath, mute)) {
        return FALSE;
    }

    context->state.muted = mute ? 1 : 0;
    tray_update_tooltip(&context->state);

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

static void hooks_play_mic_sound(int mute)
{
    AppContext *context = g_hooksContext;

    if (!context) {
        return;
    }

    sounds_play_mic(
        context->paths.exePath,
        sounds_get_directory(&context->state),
        mute ? context->state.micSoundOff : context->state.micSoundOn,
        context->state.enableMicSounds,
        context->state.soundVolume
    );
}

static void hooks_toggle_mic(void)
{
    AppState *state = &g_hooksContext->state;

    if (hooks_set_mic_state(!state->muted)) {
        hooks_play_mic_sound(state->muted);
        tray_show_mic_balloon(state, state->muted);
    }
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
                    hooks_set_mic_state(0);
                    hooks_play_ptt_sound(1);
                } else {
                    hooks_set_mic_state(1);
                    hooks_play_ptt_sound(0);
                }
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

    UINT firstKey;
    UINT secondKey;

    firstKey = state->vkMicChordFirst;
    secondKey = state->vkMicChordSecond;

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

        if (firstKeyDown &&
            secondKeyDown &&
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
    static BOOL altKeyDown = FALSE;
    static BOOL pttModeKeyDown = FALSE;

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
    
    if (passive_ptt_is_capturing()) {
        passive_ptt_process_key(keyboardData, wParam);
        return 1;
    }

    if (passive_ptt_process_key(keyboardData, wParam) ==
    PASSIVE_ACTION_TOGGLE_MIC) {
    if (state->pttMode) {
        int wasMuted = state->muted;

        if (hooks_set_mic_state(!wasMuted)) {
            hooks_play_ptt_sound(wasMuted ? 1 : 0);
            tray_show_mic_balloon(state, state->muted);
        }
    }
}

    if (keyboardData->vkCode == VK_LMENU ||
        keyboardData->vkCode == VK_RMENU ||
        keyboardData->vkCode == VK_MENU) {
        altKeyDown = hooks_is_key_down(wParam);
    }

                 if (keyboardData->vkCode == state->vkPttModeToggle) {
        if (hooks_is_key_down(wParam)) {
            UINT currentModifiers = hooks_get_modifiers();

            if (currentModifiers == state->modPttModeToggle &&
                !pttModeKeyDown) {
                pttModeKeyDown = TRUE;

                state->pttMode = !state->pttMode;

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
        keyboardData->vkCode == state->vkPttCustom) {
        if (hooks_is_key_down(wParam)) {
            if (!state->pttKeyDown) {
                state->pttKeyDown = 1;

                if (hooks_set_mic_state(0)) {
                    hooks_play_ptt_sound(1);
                }
            }
        } else if (hooks_is_key_up(wParam)) {
            if (state->pttKeyDown) {
                state->pttKeyDown = 0;

                if (hooks_set_mic_state(1)) {
                    hooks_play_ptt_sound(0);
                }
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

        return CallNextHookEx(
            state->hKeyboardHook,
            nCode,
            wParam,
            lParam
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
        state->hInst,
        0
    );

    if (!state->hMouseHook) {
        mouseError = GetLastError();
    }

    state->hKeyboardHook = SetWindowsHookExA(
        WH_KEYBOARD_LL,
        hooks_keyboard_proc,
        state->hInst,
        0
    );

    if (!state->hKeyboardHook) {
        keyboardError = GetLastError();
    }

    if (!state->hMouseHook || !state->hKeyboardHook) {
        char debug[256];

        snprintf(
            debug,
            sizeof(debug),
            "Mouse hook: %p (err %lu)\nKeyboard hook: %p (err %lu)",
            (void *)state->hMouseHook,
            (unsigned long)mouseError,
            (void *)state->hKeyboardHook,
            (unsigned long)keyboardError
        );

        
        hooks_uninstall(context);
        return FALSE;
    }

    return TRUE;
}

  void hooks_uninstall(AppContext *context)
{
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