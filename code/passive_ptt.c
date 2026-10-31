#include <windows.h>

#include "passive_ptt.h"
#include "micplus_ini.h"

#include <stdio.h>

static PassivePttMode g_mode = PASSIVE_MODE_OFF;
static UINT g_doubleVk = 0;
static DWORD g_windowMs = 500;

static BOOL g_capturing = FALSE;
static UINT g_captureFirstVk = 0;
static DWORD g_firstTapTime = 0;

static BOOL g_waitingSecondTap = FALSE;
static DWORD g_lastTapTime = 0;
static BOOL g_actionDone = FALSE;
static BOOL g_keyIsDown = FALSE;

static BOOL passive_ptt_is_russian(void)
{
    return PRIMARYLANGID(GetKeyboardLayout(0)) == LANG_RUSSIAN;
}

void passive_ptt_start_capture(HWND owner)
{
    if (passive_ptt_is_russian()) {
        MessageBoxW(
            owner,
            L"Нажмите клавишу для быстрого РТТ.\n\n"
            L"Например: Q\n\n"
            L"Двойной клик срабатывает только при включённом режиме РТТ.",
            L"Двуклик",
            MB_OK | MB_ICONINFORMATION
        );
    } else {
        MessageBoxW(
            owner,
            L"Press a key to assign double-click PTT.\n\n"
            L"For example: Q\n\n"
            L"Double-click PTT only works when PTT mode is enabled.",
            L"Double-click PTT",
            MB_OK | MB_ICONINFORMATION
        );
    }

    g_capturing = TRUE;
    g_captureFirstVk = 0;
    g_firstTapTime = 0;
}

void passive_ptt_set_double_vk(UINT virtualKey)
{
    g_mode = PASSIVE_MODE_DOUBLE;
    g_doubleVk = virtualKey;

    g_capturing = FALSE;
    g_captureFirstVk = 0;
    g_firstTapTime = 0;

    g_waitingSecondTap = FALSE;
    g_lastTapTime = 0;
    g_actionDone = FALSE;

    passive_ptt_save();
}

BOOL passive_ptt_is_capturing(void)
{
    return g_capturing;
}

static PassivePttAction passive_ptt_finish_capture(
    UINT vk)
{
    g_doubleVk = vk;
    g_mode = PASSIVE_MODE_DOUBLE;

    g_capturing = FALSE;
    g_captureFirstVk = 0;
    g_firstTapTime = 0;

    passive_ptt_save();

    if (passive_ptt_is_russian()) {
        MessageBoxW(
            NULL,
            L"Двойной клик назначен.",
            L"Готово",
            MB_OK | MB_ICONINFORMATION
        );
    } else {
        MessageBoxW(
            NULL,
            L"Double-click PTT assigned.",
            L"Done",
            MB_OK | MB_ICONINFORMATION
        );
    }

    return PASSIVE_ACTION_NONE;
}

void passive_ptt_load(void)
{
    g_doubleVk = (UINT)GetPrivateProfileIntA(
        "General", "QuickPttVK", 0, IniPath());

    g_mode = (g_doubleVk != 0)
        ? PASSIVE_MODE_DOUBLE
        : PASSIVE_MODE_OFF;
}

void passive_ptt_save(void)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%u", g_doubleVk);

    WritePrivateProfileStringA(
        "General", "QuickPttVK", buffer, IniPath());
}

PassivePttAction passive_ptt_process_key(
    const KBDLLHOOKSTRUCT *key,
    WPARAM message)
{
    BOOL isUp;

    if (!key) {
        return PASSIVE_ACTION_NONE;
    }

    isUp = (key->flags & LLKHF_UP) != 0;

    if (g_capturing) {
        DWORD now;

        if (message != WM_KEYDOWN || isUp) {
            return PASSIVE_ACTION_NONE;
        }

        now = GetTickCount();

        if (g_captureFirstVk != 0 &&
            key->vkCode == g_captureFirstVk &&
            now - g_firstTapTime <= g_windowMs) {

            return passive_ptt_finish_capture(
                key->vkCode
            );
        }

        g_captureFirstVk = key->vkCode;
        g_firstTapTime = now;

        return PASSIVE_ACTION_NONE;
    }

    if (g_mode != PASSIVE_MODE_DOUBLE ||
        key->vkCode != g_doubleVk) {
        return PASSIVE_ACTION_NONE;
    }

    if (message == WM_KEYDOWN && !isUp) {
        DWORD now = GetTickCount();

        if (g_keyIsDown) {
            return PASSIVE_ACTION_NONE;
        }

        g_keyIsDown = TRUE;

        if (g_actionDone) {
            return PASSIVE_ACTION_NONE;
        }

        if (g_waitingSecondTap &&
            now - g_lastTapTime <= g_windowMs) {

            g_waitingSecondTap = FALSE;
            g_actionDone = TRUE;

            return PASSIVE_ACTION_TOGGLE_MIC;
        }

        g_waitingSecondTap = TRUE;
        g_lastTapTime = now;

        return PASSIVE_ACTION_NONE;
    }

    if (message == WM_KEYUP && isUp) {
        g_keyIsDown = FALSE;
        g_actionDone = FALSE;
    }

    return PASSIVE_ACTION_NONE;
}

BOOL passive_ptt_is_enabled(void)
{
    return g_mode == PASSIVE_MODE_DOUBLE &&
           g_doubleVk != 0;
}

UINT passive_ptt_get_double_vk(void)
{
    return g_doubleVk;
}

void passive_ptt_clear(void)
{
    g_mode = PASSIVE_MODE_OFF;
    g_doubleVk = 0;

    g_capturing = FALSE;
    g_captureFirstVk = 0;
    g_firstTapTime = 0;

    g_waitingSecondTap = FALSE;
    g_lastTapTime = 0;
    g_actionDone = FALSE;

    passive_ptt_save();
}