#ifndef KEY_CAPTURE_H
#define KEY_CAPTURE_H

#include <windows.h>

#include "app_types.h"

BOOL key_capture_hotkey(
    HWND hWnd,
    APP_LANG language,
    const char *textId,
    const char *titleId,
    UINT *outModifiers,
    UINT *outVirtualKey
);

BOOL key_capture_ptt(
    HWND hWnd,
    APP_LANG language,
    UINT *outVirtualKey
);

#endif
