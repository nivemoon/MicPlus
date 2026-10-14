#include "system_sound.h"

BOOL system_sound_toggle(
    HWND targetWindow,
    int *mutedState)
{
    HWND window;

    window = targetWindow;

    if (!window) {
        window = GetForegroundWindow();
    }

    if (!window) {
        return FALSE;
    }

    SendMessageA(
        window,
        WM_APPCOMMAND,
        0,
        MAKELPARAM(
            0,
            APPCOMMAND_VOLUME_MUTE
        )
    );

    if (mutedState) {
        *mutedState = !(*mutedState);
    }

    return TRUE;
}