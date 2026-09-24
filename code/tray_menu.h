#ifndef TRAY_MENU_H
#define TRAY_MENU_H

#include "app_types.h"
#include <windows.h>

void tray_menu_handle_command(
    AppContext *context,
    HWND hWnd,
    UINT command
);

void tray_menu_show(
    AppContext *context,
    HWND hWnd
);

#endif
