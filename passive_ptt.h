#ifndef PASSIVE_PTT_H
#define PASSIVE_PTT_H

#include <windows.h>

typedef enum {
    PASSIVE_MODE_OFF = 0,
    PASSIVE_MODE_DOUBLE
} PassivePttMode;

typedef enum {
    PASSIVE_ACTION_NONE = 0,
    PASSIVE_ACTION_TOGGLE_MIC
} PassivePttAction;

void passive_ptt_start_capture(HWND owner);

PassivePttAction passive_ptt_process_key(
    const KBDLLHOOKSTRUCT *key,
    WPARAM message
);

BOOL passive_ptt_is_capturing(void);

BOOL passive_ptt_is_enabled(void);
UINT passive_ptt_get_double_vk(void);
void passive_ptt_clear(void);

void passive_ptt_load(void);
void passive_ptt_save(void);

#endif