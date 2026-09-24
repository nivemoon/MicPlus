#ifndef SYSTEM_SOUND_H
#define SYSTEM_SOUND_H

#include <windows.h>

BOOL system_sound_toggle(
    HWND targetWindow,
    int *mutedState
);

#endif
