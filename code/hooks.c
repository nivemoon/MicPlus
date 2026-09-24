#include "hooks.h"

#include "microphone.h"
#include "sounds.h"
#include "system_sound.h"
#include "tray.h"

#include <stdio.h>

static AppContext *g_hooksContext = NULL;

static BOOL hooks_is_key_down(
    WPARAM message)
{
    return message == WM_KEYDOWN ||
           message == WM_SYSKEYDOWN;
}

static BOOL hooks_is_key_up(
    WPARAM message)
{
    return message == WM_KEYUP ||
           message == WM_SYSKEYUP;
}

static UINT hooks_get_modifiers(void)
{
    UINT modifiers = 0;

    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
        modifiers |= MOD_CONTROL;
    }

    if (GetAsyncKeyState(VK_MENU) & 0x8000) {
        modifiers |= MOD_ALT;
    }

    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        modifiers |= MOD_SHIFT;
    }

    if ((GetAsyncKeyState(VK_LWIN) & 0x8000) ||
        (GetAsyncKeyState(VK_RWIN) & 0x8000)) {
        modifiers |= MOD_WIN;
    }

    return modifiers;
}

static BOOL hooks_set_mic_state(
    int mute)
{
    AppContext *context =
        g_hooksContext;

    if (!context) {
        return FALSE;
    }

    if (!microphone_run_ctl(
            context->paths.exePath,
            mute)) {
        return FALSE;
    }

    context->state.muted =
        mute ? 1 : 0;

    tray_update_tooltip(
        &context->state
    );

    return TRUE;
}

static void hooks_play_ptt_sound(
    int start)
{
    AppContext *context =
        g_hooksContext;

    if (!context) {
        return;
    }

    sounds_play_ptt(
        context->paths.exePath,
        "sounds",
        start
            ? context->state.pttSoundOn
            : context->state.pttSoundOff,
        context->state.enablePTTSounds
    );
}

static void hooks_play_mic_sound(
    int mute)
{
    AppContext *context =
        g_hooksContext;

    if (!context) {
        return;
    }

    sounds_play_mic(
        context->paths.exePath,
        "sounds",
        mute
            ? context->state.micSoundOff
            : context->state.micSoundOn,
        context->state.enableMicSounds
    );
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
        mouseData =
            (MSLLHOOKSTRUCT *)lParam;

        if (wParam == WM_XBUTTONDOWN ||
            wParam == WM_XBUTTONUP) {
            button = HIWORD(
                mouseData->mouseData
            );

            if (button ==
                g_hooksContext->state.pttButton) {
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
        g_hooksContext
            ? g_hooksContext->state.hMouseHook
            : NULL,
        nCode,
        wParam,
        lParam
    );
}

LRESULT CALLBACK hooks_keyboard_proc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    static int soundKeyDown = 0;

    KBDLLHOOKSTRUCT *keyboardData;
    AppState *state;
    UINT modifiers;

    if (nCode != HC_ACTION ||
        !g_hooksContext ||
        !lParam) {
        return CallNextHookEx(
            g_hooksContext
                ? g_hooksContext->state.hKeyboardHook
                : NULL,
            nCode,
            wParam,
            lParam
        );
    }

    keyboardData =
        (KBDLLHOOKSTRUCT *)lParam;

    state =
        &g_hooksContext->state;

    modifiers =
        hooks_get_modifiers();

    if (keyboardData->vkCode ==
        VK_F9) {
        if (hooks_is_key_down(wParam)) {
            if (modifiers == MOD_CONTROL &&
                !soundKeyDown) {
                soundKeyDown = 1;

                system_sound_toggle(
                    state->hMainWnd,
                    &state->soundMuted
                );

                tray_update_tooltip(
                    state
                );

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
        keyboardData->vkCode ==
            state->vkPttCustom) {
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

    if (keyboardData->vkCode ==
        state->vkToggle) {
        if (hooks_is_key_down(wParam)) {
            if (!state->toggleKeyDown &&
                modifiers == state->modToggle) {
                state->toggleKeyDown = 1;

                if (hooks_set_mic_state(
                        !state->muted)) {
                    hooks_play_mic_sound(
                        state->muted
                    );

                    tray_show_mic_balloon(
                        state,
                        state->muted
                    );
                }
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

BOOL hooks_install(
    AppContext *context)
{
    AppState *state;

    if (!context ||
        !context->state.hInst) {
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

    state->hKeyboardHook = SetWindowsHookExA(
        WH_KEYBOARD_LL,
        hooks_keyboard_proc,
        state->hInst,
        0
    );

    if (!state->hMouseHook ||
        !state->hKeyboardHook) {
        hooks_uninstall(context);
        return FALSE;
    }

    return TRUE;
}

void hooks_uninstall(
    AppContext *context)
{
    if (!context) {
        return;
    }

    if (context->state.hMouseHook) {
        UnhookWindowsHookEx(
            context->state.hMouseHook
        );

        context->state.hMouseHook = NULL;
    }

    if (context->state.hKeyboardHook) {
        UnhookWindowsHookEx(
            context->state.hKeyboardHook
        );

        context->state.hKeyboardHook = NULL;
    }

    if (g_hooksContext == context) {
        g_hooksContext = NULL;
    }
}
