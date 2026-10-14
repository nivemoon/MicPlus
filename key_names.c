#define _CRT_SECURE_NO_WARNINGS

#include "key_names.h"

#include <stdio.h>
#include <string.h>

static void key_trim_spaces(char *text)
{
    size_t length;
    char *start = text;

    while (*start == ' ' || *start == '\t') {
        start++;
    }

    if (start != text) {
        memmove(text, start, strlen(start) + 1);
    }

    length = strlen(text);

    while (length > 0 &&
           (text[length - 1] == ' ' ||
            text[length - 1] == '\t')) {
        text[--length] = '\0';
    }
}

static void key_copy_text(
    char *out,
    int outSize,
    const char *text)
{
    if (!out || outSize <= 0) {
        return;
    }

    lstrcpynA(
        out,
        text ? text : "",
        outSize
    );

    key_trim_spaces(out);
}

void key_get_display_name(
    UINT virtualKey,
    char *out,
    int outSize)
{
    UINT scanCode;
    LONG keyNameParam;
    int length;

    if (!out || outSize <= 0) {
        return;
    }

    out[0] = '\0';

    switch (virtualKey) {
        case VK_F1:
            key_copy_text(out, outSize, "F1");
            return;

        case VK_F2:
            key_copy_text(out, outSize, "F2");
            return;

        case VK_F3:
            key_copy_text(out, outSize, "F3");
            return;

        case VK_F4:
            key_copy_text(out, outSize, "F4");
            return;

        case VK_F5:
            key_copy_text(out, outSize, "F5");
            return;

        case VK_F6:
            key_copy_text(out, outSize, "F6");
            return;

        case VK_F7:
            key_copy_text(out, outSize, "F7");
            return;

        case VK_F8:
            key_copy_text(out, outSize, "F8");
            return;

        case VK_F9:
            key_copy_text(out, outSize, "F9");
            return;

        case VK_F10:
            key_copy_text(out, outSize, "F10");
            return;

        case VK_F11:
            key_copy_text(out, outSize, "F11");
            return;

        case VK_F12:
            key_copy_text(out, outSize, "F12");
            return;
    }

    if ((virtualKey >= 'A' &&
         virtualKey <= 'Z') ||
        (virtualKey >= '0' &&
         virtualKey <= '9')) {
        char text[2];

        text[0] = (char)virtualKey;
        text[1] = '\0';

        key_copy_text(
            out,
            outSize,
            text
        );

        return;
    }

    if (virtualKey == VK_OEM_3) {
        key_copy_text(
            out,
            outSize,
            "~"
        );

        return;
    }

    scanCode = MapVirtualKeyW(
        virtualKey,
        MAPVK_VK_TO_VSC
    );

    if (scanCode != 0) {
        wchar_t wideName[64];

        ZeroMemory(
            wideName,
            sizeof(wideName)
        );

        keyNameParam = (LONG)(scanCode << 16);

        length = GetKeyNameTextW(
            keyNameParam,
            wideName,
            64
        );

        if (length > 0) {
            WideCharToMultiByte(
                CP_UTF8,
                0,
                wideName,
                length,
                out,
                outSize,
                NULL,
                NULL
            );

            out[outSize - 1] = '\0';

            key_trim_spaces(out);

            return;
        }
    }

    snprintf(
        out,
        outSize,
        "VK %u",
        virtualKey
    );
}

void key_build_hotkey_name(
    UINT modifiers,
    UINT virtualKey,
    char *out,
    int outSize)
{
    char result[128];
    char keyName[64];

    if (!out || outSize <= 0) {
        return;
    }

    result[0] = '\0';

    if (modifiers & MOD_CONTROL) {
        lstrcatA(result, "Ctrl+");
    }

    if (modifiers & MOD_ALT) {
        lstrcatA(result, "Alt+");
    }

    if (modifiers & MOD_SHIFT) {
        lstrcatA(result, "Shift+");
    }

    if (modifiers & MOD_WIN) {
        lstrcatA(result, "Win+");
    }

    key_get_display_name(
        virtualKey,
        keyName,
        sizeof(keyName)
    );

    key_trim_spaces(keyName);

    lstrcatA(
        result,
        keyName
    );

    lstrcpynA(
        out,
        result,
        outSize
    );
}

void key_get_ptt_label(
    UINT pttButton,
    UINT customVirtualKey,
    char *out,
    int outSize)
{
    if (!out || outSize <= 0) {
        return;
    }

    if (customVirtualKey != 0) {
        key_get_display_name(
            customVirtualKey,
            out,
            outSize
        );

        return;
    }

    if (pttButton == XBUTTON1) {
        key_copy_text(
            out,
            outSize,
            "Mouse 4"
        );

        return;
    }

    if (pttButton == XBUTTON2) {
        key_copy_text(
            out,
            outSize,
            "Mouse 5"
        );

        return;
    }

    key_copy_text(
        out,
        outSize,
        "~"
    );
}

void key_build_chord_name(
    UINT firstVirtualKey,
    UINT secondVirtualKey,
    char *out,
    int outSize)
{
    char firstName[64];
    char secondName[64];

    if (!out || outSize <= 0) {
        return;
    }

    key_get_display_name(
        firstVirtualKey,
        firstName,
        sizeof(firstName)
    );

    key_get_display_name(
        secondVirtualKey,
        secondName,
        sizeof(secondName)
    );

    key_trim_spaces(firstName);
    key_trim_spaces(secondName);

    snprintf(
        out,
        outSize,
        "%s + %s",
        firstName,
        secondName
    );
}