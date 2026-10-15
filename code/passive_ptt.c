#include <windows.h>

#include "passive_ptt.h"

static PassivePttMode g_mode = PASSIVE_MODE_OFF;
static UINT g_doubleVk = 0;
static DWORD g_windowMs = 300;

static BOOL g_capturing = FALSE;
static UINT g_captureFirstVk = 0;
static DWORD g_firstTapTime = 0;

static BOOL g_waitingSecondTap = FALSE;
static DWORD g_lastTapTime = 0;
static BOOL g_actionDone = FALSE;
static BOOL g_keyIsDown = FALSE;

void passive_ptt_start_capture(HWND owner)
{
        MessageBoxW(
        owner,
        L"Нажмите одну и ту же клавишу два раза подряд.\n\n"
        L"Например: Q, Q\n\n"
        L"Быстрый РТТ срабатывает только при включённом режиме РТТ.",
        L"Быстрый РТТ",
        MB_OK | MB_ICONINFORMATION
    );

    g_capturing = TRUE;
    g_captureFirstVk = 0;
    g_firstTapTime = 0;
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

        MessageBoxW(
        NULL,
        L"Пассивный РТТ назначен.",
        L"Готово",
        MB_OK | MB_ICONINFORMATION
    );

    return PASSIVE_ACTION_NONE;
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

    /* Режим назначения пассивного PTT */
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

    /* Обычная работа пассивного PTT */
    if (g_mode != PASSIVE_MODE_DOUBLE ||
        key->vkCode != g_doubleVk) {
        return PASSIVE_ACTION_NONE;
    }

    if (message == WM_KEYDOWN && !isUp) {
        DWORD now = GetTickCount();

        /* Автоповтор при удержании: клавиша ещё не была отпущена */
        if (g_keyIsDown) {
            return PASSIVE_ACTION_NONE;
        }

        g_keyIsDown = TRUE;

        if (g_actionDone) {
            return PASSIVE_ACTION_NONE;
        }

        /* Второй тап засчитывается только если клавиша
           была отпущена после первого тапа */
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
    
void passive_ptt_load(void)
{
    /* Подключим к settings.c после первого успешного теста */
}

void passive_ptt_save(void)
{
    /* Подключим к settings.c после первого успешного теста */
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