#ifndef HOOKS_H
#define HOOKS_H

#include <windows.h>

#include "app_types.h"

BOOL hooks_install(
    AppContext *context
);

void hooks_uninstall(
    AppContext *context
);

LRESULT CALLBACK hooks_mouse_proc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
);

LRESULT CALLBACK hooks_keyboard_proc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
);

#endif
