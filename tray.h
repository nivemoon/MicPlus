#ifndef TRAY_H
#define TRAY_H

#include <windows.h>

#include "app_types.h"

BOOL tray_load_icons(
    AppState *state
);

BOOL tray_add_icon(
    AppState *state,
    HWND hWnd
);

void tray_update_tooltip(
    AppState *state
);

void tray_show_mic_balloon(
    AppState *state,
    int muted
);

void tray_remove_icon(
    AppState *state
);

#endif