#include "unicode.h"
#define _CRT_SECURE_NO_WARNINGS

#include "key_capture.h"
#include "key_names.h"
#include "localization.h"

#include <stdio.h>

#define MIC_CHORD_WAIT_MS 350

static UINT key_capture_get_modifiers(void)
{
    UINT modifiers = 0;
    BOOL rightAlt =
        (GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;

    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
        modifiers |= MOD_CONTROL;
    }

    if (GetAsyncKeyState(VK_MENU) & 0x8000) {
        modifiers |= MOD_ALT;
    }

    if (rightAlt) {
        modifiers &= ~MOD_CONTROL;
    }

    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        modifiers |= MOD_SHIFT;
    }

    return modifiers;
}

static BOOL key_capture_is_modifier(UINT virtualKey)
{
    return virtualKey == VK_CONTROL ||
           virtualKey == VK_LCONTROL ||
           virtualKey == VK_RCONTROL ||
           virtualKey == VK_MENU ||
           virtualKey == VK_LMENU ||
           virtualKey == VK_RMENU ||
           virtualKey == VK_SHIFT ||
           virtualKey == VK_LSHIFT ||
           virtualKey == VK_RSHIFT ||
           virtualKey == VK_LWIN ||
           virtualKey == VK_RWIN;
}

BOOL key_capture_hotkey(
    HWND hWnd,
    APP_LANG language,
    const char *textId,
    const char *titleId,
    UINT *outModifiers,
    UINT *outVirtualKey)
{
    MSG message;
    UINT virtualKey = 0;
    UINT modifiers;
    char keyName[64];
    char resultText[256];

    if (!outModifiers || !outVirtualKey) {
        return FALSE;
    }

    message_box_utf8(
        hWnd,
        localization_get(language, textId),
        localization_get(language, titleId),
        MB_OK | MB_ICONINFORMATION
    );

    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        if (message.message == WM_KEYDOWN ||
            message.message == WM_SYSKEYDOWN) {
            UINT code = (UINT)message.wParam;

            if (code == VK_ESCAPE) {
                return FALSE;
            }

            if (key_capture_is_modifier(code)) {
                continue;
            }

            virtualKey = code;
            break;
        }

        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    if (virtualKey == 0) {
        return FALSE;
    }

    modifiers = key_capture_get_modifiers();

    if ((virtualKey == VK_RETURN ||
         virtualKey == VK_ESCAPE) &&
        modifiers == 0) {
        message_box_utf8(
            hWnd,
            localization_get(language, "MSG_ERR_TOGGLE_ENTER_ESC"),
            "MicPlus",
            MB_OK | MB_ICONINFORMATION
        );

        return FALSE;
    }

    *outModifiers = modifiers;
    *outVirtualKey = virtualKey;

    key_build_hotkey_name(
        modifiers,
        virtualKey,
        keyName,
        sizeof(keyName)
    );

    snprintf(
        resultText,
        sizeof(resultText),
        localization_get(language, "MSG_SELECTED_HK"),
        keyName
    );

    message_box_utf8(
        hWnd,
        resultText,
        "MicPlus",
        MB_OK | MB_ICONINFORMATION
    );

    return TRUE;
}

BOOL key_capture_mic_hotkey(
    HWND hWnd,
    APP_LANG language,
    UINT *outModifiers,
    UINT *outVirtualKey,
    UINT *outChordFirst,
    UINT *outChordSecond)
{
    MSG message;
    UINT firstKey = 0;
    UINT secondKey = 0;
    UINT modifiers;
    DWORD waitStart;
    char hotkeyName[128];
    char resultText[256];

    if (!outModifiers ||
        !outVirtualKey ||
        !outChordFirst ||
        !outChordSecond) {
        return FALSE;
    }

    message_box_utf8(
        hWnd,
        localization_get(language, "MSG_TOGGLE_SELECT_TEXT"),
        localization_get(language, "MSG_TOGGLE_SELECT_TITLE"),
        MB_OK | MB_ICONINFORMATION
    );

    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        if (message.message == WM_KEYDOWN ||
            message.message == WM_SYSKEYDOWN) {
            UINT code = (UINT)message.wParam;

            if (code == VK_ESCAPE) {
                return FALSE;
            }

            if (key_capture_is_modifier(code)) {
                continue;
            }

            firstKey = code;
            break;
        }

        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    if (firstKey == 0) {
        return FALSE;
    }

    waitStart = GetTickCount();

    while (GetTickCount() - waitStart < MIC_CHORD_WAIT_MS) {
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_KEYDOWN ||
                message.message == WM_SYSKEYDOWN) {
                UINT code = (UINT)message.wParam;

                if (code == VK_ESCAPE) {
                    return FALSE;
                }

                if (!key_capture_is_modifier(code) &&
                    code != firstKey) {
                    secondKey = code;
                    break;
                }
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        if (secondKey != 0) {
            break;
        }

        Sleep(10);
    }

    *outChordFirst = 0;
    *outChordSecond = 0;

    if (secondKey != 0) {
        *outChordFirst = firstKey;
        *outChordSecond = secondKey;
        *outModifiers = 0;
        *outVirtualKey = firstKey;

        key_build_chord_name(
            firstKey,
            secondKey,
            hotkeyName,
            sizeof(hotkeyName)
        );
    } else {
        modifiers = key_capture_get_modifiers();
		      

        if ((firstKey == VK_RETURN ||
             firstKey == VK_ESCAPE) &&
            modifiers == 0) {
            message_box_utf8(
                hWnd,
                localization_get(language, "MSG_ERR_TOGGLE_ENTER_ESC"),
                "MicPlus",
                MB_OK | MB_ICONINFORMATION
            );

            return FALSE;
        }

        *outModifiers = modifiers;
        *outVirtualKey = firstKey;

        key_build_hotkey_name(
            modifiers,
            firstKey,
            hotkeyName,
            sizeof(hotkeyName)
        );
    }

    snprintf(
        resultText,
        sizeof(resultText),
        localization_get(language, "MSG_SELECTED_HK"),
        hotkeyName
    );

    message_box_utf8(
        hWnd,
        resultText,
        "MicPlus",
        MB_OK | MB_ICONINFORMATION
    );

    return TRUE;
}

BOOL key_capture_ptt(
    HWND hWnd,
    APP_LANG language,
    UINT *outVirtualKey)
{
    MSG message;
    UINT virtualKey = 0;
    char keyName[64];
    char resultText[256];

    if (!outVirtualKey) {
        return FALSE;
    }

    message_box_utf8(
        hWnd,
        localization_get(language, "MSG_PTT_SELECT_TEXT"),
        localization_get(language, "MSG_PTT_SELECT_TITLE"),
        MB_OK | MB_ICONINFORMATION
    );

    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        if (message.message == WM_KEYDOWN ||
            message.message == WM_SYSKEYDOWN) {
            virtualKey = (UINT)message.wParam;

            if (virtualKey == VK_ESCAPE) {
                return FALSE;
            }

            break;
        }

        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    if (virtualKey == 0) {
        return FALSE;
    }

    if (virtualKey == VK_RETURN ||
        virtualKey == VK_ESCAPE) {
        message_box_utf8(
            hWnd,
            localization_get(language, "MSG_ERR_PTT_ENTER_ESC"),
            "MicPlus",
            MB_OK | MB_ICONINFORMATION
        );

        return FALSE;
    }

    *outVirtualKey = virtualKey;

    key_get_display_name(
        virtualKey,
        keyName,
        sizeof(keyName)
    );

    snprintf(
        resultText,
        sizeof(resultText),
        localization_get(language, "MSG_SELECTED_PTT"),
        keyName
    );

    message_box_utf8(
        hWnd,
        resultText,
        "MicPlus",
        MB_OK | MB_ICONINFORMATION
    );

    return TRUE;
}