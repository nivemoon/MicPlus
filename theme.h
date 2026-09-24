#ifndef THEME_H
#define THEME_H

#include <windows.h>

int theme_is_system_light(void);

void theme_update_icon(
    HICON *current,
    HICON iconLight,
    HICON iconDark,
    int iconMode
);

int theme_is_menu_dark(int menuTheme);

#endif