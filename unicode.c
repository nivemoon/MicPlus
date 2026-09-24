#define _CRT_SECURE_NO_WARNINGS

#include "unicode.h"

#include <stdlib.h>

int utf8_to_wide(
    const char *source,
    WCHAR *destination,
    int destinationCount)
{
    int length;

    if (!source ||
        !destination ||
        destinationCount <= 0) {
        return 0;
    }

    destination[0] = L'\0';

    length = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        source,
        -1,
        destination,
        destinationCount
    );

    if (length > 0) {
        return length;
    }

    /*
       Запасной вариант для строк, которые
       случайно оказались невалидным UTF-8.
    */
    return MultiByteToWideChar(
        CP_ACP,
        0,
        source,
        -1,
        destination,
        destinationCount
    );
}

BOOL message_box_utf8(
    HWND hWnd,
    const char *text,
    const char *caption,
    UINT type)
{
    WCHAR wideText[1024];
    WCHAR wideCaption[256];

    if (!utf8_to_wide(
            text,
            wideText,
            ARRAYSIZE(wideText))) {
        return FALSE;
    }

    if (!utf8_to_wide(
            caption,
            wideCaption,
            ARRAYSIZE(wideCaption))) {
        return FALSE;
    }

    return MessageBoxW(
        hWnd,
        wideText,
        wideCaption,
        type
    ) != 0;
}

BOOL menu_insert_utf8(
    HMENU menu,
    UINT position,
    UINT flags,
    UINT_PTR command,
    const char *text)
{
    WCHAR wideText[512];

    if (!menu ||
        !text ||
        !utf8_to_wide(
            text,
            wideText,
            ARRAYSIZE(wideText))) {
        return FALSE;
    }

    return InsertMenuW(
        menu,
        position,
        flags,
        command,
        wideText
    );
}

BOOL menu_append_utf8(
    HMENU menu,
    UINT flags,
    UINT_PTR command,
    const char *text)
{
    WCHAR wideText[512];

    if (!menu ||
        !text ||
        !utf8_to_wide(
            text,
            wideText,
            ARRAYSIZE(wideText))) {
        return FALSE;
    }

    return AppendMenuW(
        menu,
        flags,
        command,
        wideText
    );
}