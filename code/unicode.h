#ifndef UNICODE_HELPERS_H
#define UNICODE_HELPERS_H

#include <windows.h>

int utf8_to_wide(
    const char *source,
    WCHAR *destination,
    int destinationCount
);

BOOL message_box_utf8(
    HWND hWnd,
    const char *text,
    const char *caption,
    UINT type
);

BOOL menu_insert_utf8(
    HMENU menu,
    UINT position,
    UINT flags,
    UINT_PTR command,
    const char *text
);

BOOL menu_append_utf8(
    HMENU menu,
    UINT flags,
    UINT_PTR command,
    const char *text
);

#endif
