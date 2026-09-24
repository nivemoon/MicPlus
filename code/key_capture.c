#include "unicode.h"
#define _CRT_SECURE_NO_WARNINGS

#include "key_capture.h"

#include "key_names.h"
#include "localization.h"

#include <stdio.h>

static UINT key_capture_get_modifiers(void)
{
    UINT modifiers = 0;

    BOOL leftCtrl;
    BOOL rightCtrl;
    BOOL leftAlt;
    BOOL rightAlt;
    BOOL leftShift;
    BOOL rightShift;
    BOOL leftWin;
    BOOL rightWin;

    leftCtrl = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
    rightCtrl = (GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0;

    leftAlt = (GetAsyncKeyState(VK_LMENU) & 0x8000) != 0;
    rightAlt = (GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;

    leftShift = (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;
    rightShift = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0;

    leftWin = (GetAsyncKeyState(VK_LWIN) & 0x8000) != 0;
    rightWin = (GetAsyncKeyState(VK_RWIN) & 0x8000) != 0;

    if (leftCtrl || rightCtrl) {
        modifiers |= MOD_CONTROL;
    }

    if (leftAlt || rightAlt) {
        modifiers |= MOD_ALT;
    }

    if (leftShift || rightShift) {
        modifiers |= MOD_SHIFT;
    }

    if (leftWin || rightWin) {
        modifiers |= MOD_WIN;
    }

    if (leftCtrl && rightAlt) {
        modifiers &= ~MOD_CONTROL;
        modifiers |= MOD_ALT;
    }

    return modifiers;
}

static BOOL key_capture_is_modifier(
    UINT virtualKey)
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

    if (!outModifiers ||
        !outVirtualKey) {
        return FALSE;
    }

    message_box_utf8(
        hWnd,
        localization_get(
            language,
            textId
        ),
        localization_get(
            language,
            titleId
        ),
        MB_OK | MB_ICONINFORMATION
    );

    while (GetMessageA(
               &message,
               NULL,
               0,
               0) > 0) {
        if (message.message == WM_KEYDOWN ||
            message.message == WM_SYSKEYDOWN) {

            UINT code =
                (UINT)message.wParam;

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

    modifiers =
        key_capture_get_modifiers();

    if ((virtualKey == VK_RETURN ||
         virtualKey == VK_ESCAPE) &&
        modifiers == 0) {
        message_box_utf8(
            hWnd,
            localization_get(
                language,
                "MSG_ERR_TOGGLE_ENTER_ESC"
            ),
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
        localization_get(
            language,
            "MSG_SELECTED_HK"
        ),
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
        localization_get(
            language,
            "MSG_PTT_SELECT_TEXT"
        ),
        localization_get(
            language,
            "MSG_PTT_SELECT_TITLE"
        ),
        MB_OK | MB_ICONINFORMATION
    );

    while (GetMessageA(
               &message,
               NULL,
               0,
               0) > 0) {
        if (message.message == WM_KEYDOWN ||
            message.message == WM_SYSKEYDOWN) {

            virtualKey =
                (UINT)message.wParam;

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
            localization_get(
                language,
                "MSG_ERR_PTT_ENTER_ESC"
            ),
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
        localization_get(
            language,
            "MSG_SELECTED_PTT"
        ),
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
