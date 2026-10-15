#include "unicode.h"
#define _CRT_SECURE_NO_WARNINGS

#include "tray_menu.h"
#include "passive_ptt.h"
#include "key_capture.h"
#include "key_names.h"
#include "localization.h"
#include "settings.h"
#include "startup.h"
#include "theme.h"
#include "tray.h"
#include "updater.h"

#include <commctrl.h>
#include <shellapi.h>
#include <string.h>

#include "resource.h"
#include "sounds.h"

#include <stdio.h>

#define MICPLUS_WEBSITE \
    "https://github.com/nivemoon/micplus"

#ifndef ID_HK_MIC_CHORD_SELECT
#define ID_HK_MIC_CHORD_SELECT 5101
#endif

#ifndef ID_HK_MIC_CHORD_CLEAR
#define ID_HK_MIC_CHORD_CLEAR 5102
#endif

#ifndef ID_TRAY_PTT_HEADER
#define ID_TRAY_PTT_HEADER 5201
#endif

#ifndef ID_HK_QUICK_PTT_ASSIGN
#define ID_HK_QUICK_PTT_ASSIGN  5301
#endif

#ifndef ID_HK_QUICK_PTT_CHANGE
#define ID_HK_QUICK_PTT_CHANGE  5302
#endif

#ifndef ID_HK_QUICK_PTT_REMOVE
#define ID_HK_QUICK_PTT_REMOVE  5303
#endif

#ifndef ID_HK_PTT_MODE_CHANGE
#define ID_HK_PTT_MODE_CHANGE 5401
#endif

#ifndef ID_HK_PTT_MODE_RESET
#define ID_HK_PTT_MODE_RESET 5402
#endif

#ifndef ID_SOUND_HEADER
#define ID_SOUND_HEADER 6100
#endif

#ifndef ID_SOUND_VOLUME_0
#define ID_SOUND_VOLUME_0 6101
#endif

#ifndef ID_SOUND_VOLUME_25
#define ID_SOUND_VOLUME_25 6102
#endif

#ifndef ID_SOUND_VOLUME_50
#define ID_SOUND_VOLUME_50 6103
#endif

#ifndef ID_SOUND_VOLUME_75
#define ID_SOUND_VOLUME_75 6104
#endif

#ifndef ID_SOUND_VOLUME_100
#define ID_SOUND_VOLUME_100 6105
#endif

#ifndef ID_SOUND_MIC
#define ID_SOUND_MIC 6106
#endif

#ifndef ID_SOUND_PTT
#define ID_SOUND_PTT 6107
#endif

#ifndef ID_SOUND_PTT_MODE
#define ID_SOUND_PTT_MODE 6108
#endif

#ifndef ID_SOUND_SCHEME_CLASSIC
#define ID_SOUND_SCHEME_CLASSIC 6109
#endif

#ifndef ID_SOUND_SCHEME_MICPLUS
#define ID_SOUND_SCHEME_MICPLUS 6110
#endif

static const char *format_hotkey(UINT modifiers, UINT vk)
{
    static char buffer[64];
    const char *keyName = NULL;

    switch (vk) {
        case VK_F1: keyName = "F1"; break;
        case VK_F2: keyName = "F2"; break;
        case VK_F3: keyName = "F3"; break;
        case VK_F4: keyName = "F4"; break;
        case VK_F5: keyName = "F5"; break;
        case VK_F6: keyName = "F6"; break;
        case VK_F7: keyName = "F7"; break;
        case VK_F8: keyName = "F8"; break;
        case VK_F9: keyName = "F9"; break;
        case VK_F10: keyName = "F10"; break;
        case VK_F11: keyName = "F11"; break;
        case VK_F12: keyName = "F12"; break;

        /* Знаки — всегда английские названия */
        case VK_OEM_1:     keyName = ";";  break;  /* ж */
        case VK_OEM_PLUS:  keyName = "=";  break;
        case VK_OEM_COMMA: keyName = ",";  break;  /* б */
        case VK_OEM_MINUS: keyName = "-";  break;
        case VK_OEM_PERIOD:keyName = ".";  break;  /* ю */
        case VK_OEM_2:     keyName = "/";  break;  /* . */
        case VK_OEM_3:     keyName = "`";  break;  /* ё */
        case VK_OEM_4:     keyName = "[";  break;  /* х */
        case VK_OEM_5:     keyName = "\\"; break;
        case VK_OEM_6:     keyName = "]";  break;  /* ъ */
        case VK_OEM_7:     keyName = "'";  break;  /* э */

        case VK_SPACE:     keyName = "Space"; break;
        case VK_TAB:       keyName = "Tab"; break;
        case VK_CAPITAL:   keyName = "Caps Lock"; break;
        case VK_BACK:      keyName = "Backspace"; break;
        case VK_NUMLOCK:   keyName = "Num Lock"; break;
        case VK_SCROLL:    keyName = "Scroll Lock"; break;
        case VK_INSERT:    keyName = "Insert"; break;
        case VK_DELETE:    keyName = "Delete"; break;
        case VK_HOME:      keyName = "Home"; break;
        case VK_END:       keyName = "End"; break;
        case VK_PRIOR:     keyName = "Page Up"; break;
        case VK_NEXT:      keyName = "Page Down"; break;

        /* Нумпад */
        case VK_NUMPAD0: keyName = "Num 0"; break;
        case VK_NUMPAD1: keyName = "Num 1"; break;
        case VK_NUMPAD2: keyName = "Num 2"; break;
        case VK_NUMPAD3: keyName = "Num 3"; break;
        case VK_NUMPAD4: keyName = "Num 4"; break;
        case VK_NUMPAD5: keyName = "Num 5"; break;
        case VK_NUMPAD6: keyName = "Num 6"; break;
        case VK_NUMPAD7: keyName = "Num 7"; break;
        case VK_NUMPAD8: keyName = "Num 8"; break;
        case VK_NUMPAD9: keyName = "Num 9"; break;
        case VK_MULTIPLY: keyName = "Num *"; break;
        case VK_ADD:      keyName = "Num +"; break;
        case VK_SUBTRACT: keyName = "Num -"; break;
        case VK_DECIMAL:  keyName = "Num ."; break;
        case VK_DIVIDE:   keyName = "Num /"; break;

        default:
            if (vk >= 'A' && vk <= 'Z') {
                static char letter[2];
                letter[0] = (char)vk;
                letter[1] = '\0';
                keyName = letter;
            } else if (vk >= '0' && vk <= '9') {
                static char digit[2];
                digit[0] = (char)vk;
                digit[1] = '\0';
                keyName = digit;
            } else {
                keyName = "?";
            }
            break;
    }

    buffer[0] = '\0';

    if (modifiers & MOD_CONTROL) {
        strcat(buffer, "Ctrl+");
    }

    if (modifiers & MOD_ALT) {
        strcat(buffer, "Alt+");
    }

    if (modifiers & MOD_SHIFT) {
        strcat(buffer, "Shift+");
    }

    if (modifiers & MOD_WIN) {
        strcat(buffer, "Win+");
    }

    strcat(buffer, keyName);

    return buffer;
}
   
static void tray_menu_save(AppContext *context)
{
    settings_save(&context->state, &context->paths);
}

static void tray_menu_refresh(AppContext *context)
{
    tray_update_tooltip(&context->state);
}

static void tray_menu_set_language(AppContext *context, APP_LANG language)
{
    context->state.lang = language;
    tray_menu_save(context);
    tray_menu_refresh(context);
}

static void tray_menu_set_icon_mode(AppContext *context, int iconMode)
{
    context->state.iconMode = iconMode;

    theme_update_icon(
        &context->state.hIconOnCurrent,
        context->state.hIconOnLight,
        context->state.hIconOnDark,
        context->state.iconMode
    );

    tray_menu_save(context);
    tray_menu_refresh(context);
}

static void tray_menu_set_ptt_mouse(AppContext *context, UINT button)
{
    context->state.pttButton = button;
    context->state.vkPttCustom = 0;
    context->state.pttKeyDown = 0;

    tray_menu_save(context);
    tray_menu_refresh(context);
}

static void tray_menu_set_ptt_tilde(AppContext *context)
{
    context->state.pttButton = 0;
    context->state.vkPttCustom = VK_OEM_3;
    context->state.pttKeyDown = 0;

    tray_menu_save(context);
    tray_menu_refresh(context);
}

static void tray_menu_select_ptt(AppContext *context, HWND hWnd)
{
    UINT virtualKey;

    if (!key_capture_ptt(
            hWnd,
            context->state.lang,
            &virtualKey)) {
        return;
    }

    context->state.pttButton = 0;
    context->state.vkPttCustom = virtualKey;
    context->state.pttKeyDown = 0;

    tray_menu_save(context);
    tray_menu_refresh(context);
}

static void tray_menu_select_mic_hotkey(AppContext *context, HWND hWnd)
{
    UINT modifiers;
    UINT virtualKey;
    UINT chordFirst;
    UINT chordSecond;

    if (!key_capture_mic_hotkey(
            hWnd,
            context->state.lang,
            &modifiers,
            &virtualKey,
            &chordFirst,
            &chordSecond)) {
        return;
    }

    if (chordFirst != 0 && chordSecond != 0) {
        context->state.vkMicChordFirst = chordFirst;
        context->state.vkMicChordSecond = chordSecond;
        context->state.micChordKeyDown = 0;

        context->state.vkToggle = chordFirst;
        context->state.modToggle = 0;
        context->state.toggleCustomSet = 1;
    } else {
        if (modifiers == MOD_CONTROL &&
            virtualKey == VK_F9) {

            if (context->state.lang == LANG_RU) {
                MessageBoxW(
                    hWnd,
                    L"Ctrl+F9 зарезервирована для управления звуком.",
                    L"MicPlus",
                    MB_OK | MB_ICONWARNING
                );
            } else {
                MessageBoxA(
                    hWnd,
                    "Ctrl+F9 is reserved for sound control.",
                    "MicPlus",
                    MB_OK | MB_ICONWARNING
                );
            }

            return;
        }

        context->state.vkMicChordFirst = 0;
        context->state.vkMicChordSecond = 0;
        context->state.micChordKeyDown = 0;

        context->state.modToggle = modifiers;
        context->state.vkToggle = virtualKey;
        context->state.toggleCustomSet = 1;
    }

    tray_menu_save(context);
    tray_menu_refresh(context);
}

static void tray_menu_select_ptt_mode_hotkey(AppContext *context, HWND hWnd)
{
    UINT modifiers;
    UINT virtualKey;
    UINT chordFirst;
    UINT chordSecond;

    if (!key_capture_mic_hotkey(
            hWnd,
            context->state.lang,
            &modifiers,
            &virtualKey,
            &chordFirst,
            &chordSecond)) {
        return;
    }

    if (chordFirst != 0 && chordSecond != 0) {
        context->state.vkPttModeChordFirst = chordFirst;
        context->state.vkPttModeChordSecond = chordSecond;
        context->state.pttModeChordKeyDown = 0;

        context->state.vkPttModeToggle = 0;
        context->state.modPttModeToggle = 0;
    } else {
        context->state.vkPttModeChordFirst = 0;
        context->state.vkPttModeChordSecond = 0;
        context->state.pttModeChordKeyDown = 0;

        context->state.modPttModeToggle = modifiers;
        context->state.vkPttModeToggle = virtualKey;
    }

    tray_menu_save(context);
    tray_menu_refresh(context);
}

static INT_PTR CALLBACK tray_about_dialog_proc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    AppContext *context;

    context = (AppContext *)GetWindowLongPtrA(
        hDlg,
        DWLP_USER
    );

    switch (message) {
        case WM_INITDIALOG:
            SetWindowLongPtrA(
                hDlg,
                DWLP_USER,
                (LONG_PTR)lParam
            );

            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_ABOUT_CLOSE:
                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return TRUE;

                case IDC_ABOUT_CHECKUPDATE:
                    updater_check_and_show(hDlg);
                    return TRUE;

                case IDC_ABOUT_RESET:
                    if (!context) {
                        return TRUE;
                    }

                    if (context->state.lang == LANG_RU) {
                        static const WCHAR russianPrompt[] =
                            L"Удалить файл настроек MicPlus?\n\n"
                            L"Все сохранённые настройки будут удалены.\n"
                            L"После удаления программа будет закрыта.\n"
                            L"При следующем запуске будут использованы "
                            L"настройки по умолчанию.\n\n"
                            L"Продолжить?";

                        if (MessageBoxW(
                                hDlg,
                                russianPrompt,
                                L"MicPlus",
                                MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2
                            ) != IDYES) {
                            return TRUE;
                        }
                    } else {
                        MessageBoxA(
                            hDlg,
                            "Delete the MicPlus settings file?\n\n"
                            "All saved settings will be deleted.\n"
                            "The program will close after deletion.\n"
                            "Default settings will be used next time.\n\n"
                            "Continue?",
                            "MicPlus",
                            MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2
                        );
                    }

                    if (DeleteFileA(context->paths.iniPath)) {
                        EndDialog(hDlg, 1);
                        PostQuitMessage(0);
                    } else {
                        if (context->state.lang == LANG_RU) {
                            MessageBoxW(
                                hDlg,
                                L"Не удалось удалить файл настроек.",
                                L"MicPlus",
                                MB_OK | MB_ICONERROR
                            );
                        } else {
                            MessageBoxA(
                                hDlg,
                                "The settings file could not be deleted.",
                                "MicPlus",
                                MB_OK | MB_ICONERROR
                            );
                        }
                    }

                    return TRUE;
            }

            break;

        case WM_NOTIFY:
            if (lParam) {
                NMHDR *notificationHeader =
                    (NMHDR *)lParam;

                if (notificationHeader->idFrom == IDC_ABOUT_LINK &&
                    notificationHeader->code == NM_CLICK) {

                    ShellExecuteA(
                        hDlg,
                        "open",
                        MICPLUS_WEBSITE,
                        NULL,
                        NULL,
                        SW_SHOWNORMAL
                    );

                    return TRUE;
                }
            }

            break;
    }

    return FALSE;
}

static void tray_menu_show_about(AppContext *context, HWND hWnd)
{
    INITCOMMONCONTROLSEX commonControls;
    INT_PTR result;

    commonControls.dwSize = sizeof(commonControls);
    commonControls.dwICC = ICC_LINK_CLASS;

    InitCommonControlsEx(&commonControls);

    result = DialogBoxParamA(
        GetModuleHandleA(NULL),
        MAKEINTRESOURCEA(IDD_ABOUT),
        hWnd,
        tray_about_dialog_proc,
        (LPARAM)context
    );

    if (result == -1) {
        char errorMessage[128];

        snprintf(
            errorMessage,
            sizeof(errorMessage),
            "DialogBoxParamA failed.\nError code: %lu",
            (unsigned long)GetLastError()
        );

        MessageBoxA(
            hWnd,
            errorMessage,
            "MicPlus",
            MB_OK | MB_ICONERROR
        );
    }
}

void tray_menu_handle_command(AppContext *context, HWND hWnd, UINT command)
{
    if (!context) {
        return;
    }

    switch (command) {
        case ID_TRAY_PTT_MODE:
            context->state.pttMode = !context->state.pttMode;
            context->state.pttKeyDown = 0;

            if (context->state.pttMode) {
                sounds_play_ptt_mode(
                    context->paths.exePath,
                    sounds_get_directory(&context->state),
                    "ptt_mode_on.mp3",
                    context->state.enablePTTModeSounds,
                    context->state.soundVolume
                );
            } else {
                sounds_play_ptt_mode(
                    context->paths.exePath,
                    sounds_get_directory(&context->state),
                    "ptt_mode_off.mp3",
                    context->state.enablePTTModeSounds,
                    context->state.soundVolume
                );
            }

            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HK_PTT_MOUSE4:
            tray_menu_set_ptt_mouse(context, XBUTTON1);
            break;

        case ID_HK_PTT_MOUSE5:
            tray_menu_set_ptt_mouse(context, XBUTTON2);
            break;

        case ID_HK_PTT_TILDE:
            tray_menu_set_ptt_tilde(context);
            break;

        case ID_HK_PTT_CUSTOM:
            tray_menu_select_ptt(context, hWnd);
            break;

        case ID_HK_MIC_F9:
            context->state.vkToggle = VK_F9;
            context->state.modToggle = 0;
            context->state.toggleCustomSet = 0;

            context->state.vkMicChordFirst = 0;
            context->state.vkMicChordSecond = 0;
            context->state.micChordKeyDown = 0;

            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HK_MIC_F10:
            context->state.vkToggle = VK_F10;
            context->state.modToggle = 0;
            context->state.toggleCustomSet = 0;

            context->state.vkMicChordFirst = 0;
            context->state.vkMicChordSecond = 0;
            context->state.micChordKeyDown = 0;

            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HK_MIC_TILDE:
            context->state.vkToggle = VK_OEM_3;
            context->state.modToggle = 0;
            context->state.toggleCustomSet = 0;

            context->state.vkMicChordFirst = 0;
            context->state.vkMicChordSecond = 0;
            context->state.micChordKeyDown = 0;

            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HK_PTT_MODE_CHANGE:
            tray_menu_select_ptt_mode_hotkey(context, hWnd);
            break;

        case ID_HK_MIC_CUSTOM:
            tray_menu_select_mic_hotkey(context, hWnd);
            break;

        case ID_HK_PTT_MODE_RESET:
            context->state.vkPttModeToggle = 'P';
            context->state.modPttModeToggle = MOD_ALT;

            context->state.vkPttModeChordFirst = 0;
            context->state.vkPttModeChordSecond = 0;
            context->state.pttModeChordKeyDown = 0;

            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HK_QUICK_PTT_ASSIGN:
        case ID_HK_QUICK_PTT_CHANGE:
            passive_ptt_start_capture(hWnd);
            break;

        case ID_HK_QUICK_PTT_REMOVE:
            passive_ptt_clear();
            tray_menu_refresh(context);
            break;

        case ID_LANG_EN:
            tray_menu_set_language(context, LANG_EN);
            break;

        case ID_LANG_RU:
            tray_menu_set_language(context, LANG_RU);
            break;

        case ID_SETTINGS_ICON_AUTO:
            tray_menu_set_icon_mode(context, 0);
            break;

        case ID_SETTINGS_ICON_BLACK:
            tray_menu_set_icon_mode(context, 1);
            break;

        case ID_SETTINGS_ICON_WHITE:
            tray_menu_set_icon_mode(context, 2);
            break;

        case ID_SETTINGS_AUTOSTART:
            context->state.autoStart = !context->state.autoStart;

            startup_set(
                &context->paths,
                context->state.autoStart
            );

            tray_menu_save(context);
            break;

        case ID_SETTINGS_MIC_SOUNDS:
            context->state.enableMicSounds =
                !context->state.enableMicSounds;

            tray_menu_save(context);
            break;

        case ID_SETTINGS_PTT_SOUNDS:
            context->state.enablePTTSounds =
                !context->state.enablePTTSounds;

            tray_menu_save(context);
            break;

        case ID_SETTINGS_BALLOON:
            context->state.enableBalloonTips =
                !context->state.enableBalloonTips;

            tray_menu_save(context);

            if (context->state.enableBalloonTips) {
                tray_show_mic_balloon(
                    &context->state,
                    context->state.muted
                );
            }

            break;

        case ID_SETTINGS_ABOUT:
            tray_menu_show_about(context, hWnd);
            break;

        case ID_SOUND_VOLUME_0:
            context->state.soundVolume = 0;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_VOLUME_25:
            context->state.soundVolume = 25;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_VOLUME_50:
            context->state.soundVolume = 50;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_VOLUME_75:
            context->state.soundVolume = 75;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_VOLUME_100:
            context->state.soundVolume = 100;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_MIC:
            context->state.enableMicSounds =
                !context->state.enableMicSounds;

            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_PTT:
            context->state.enablePTTSounds =
                !context->state.enablePTTSounds;

            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_PTT_MODE:
            context->state.enablePTTModeSounds =
                !context->state.enablePTTModeSounds;

            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_SCHEME_CLASSIC:
            context->state.soundScheme = 0;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_SCHEME_MICPLUS:
            context->state.soundScheme = 1;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;
    }
}

void tray_menu_show(AppContext *context, HWND hWnd)
{
    POINT cursorPosition;

    char micHotkeyText[160];
    char pttHeaderText[160];
    char soundHeaderText[160];
    char quickPttText[160];
    char pttCustomText[160];
    char micCustomText[160];
	char pttModeText[160];
    char chordName[128];

    const char *micBase;
    const char *hotkeyStr;
    const char *mouse4Str;
    const char *mouse5Str;
    const char *assignText;
    const char *quickChangeText;
    const char *quickRemoveText;

    HMENU mainMenu;
    HMENU hotkeysMenu;
    HMENU settingsMenu;
    HMENU languageMenu;
    HMENU iconMenu;
    HMENU soundMenu;
    HMENU schemeMenu;
    HMENU pttMenu;
    HMENU micMenu;
    HMENU pttModeMenu;
    HMENU quickPttMenu;

    if (!context || !hWnd || !GetCursorPos(&cursorPosition)) {
        return;
    }

    mainMenu = CreatePopupMenu();
    hotkeysMenu = CreatePopupMenu();
    settingsMenu = CreatePopupMenu();
    languageMenu = CreatePopupMenu();
    iconMenu = CreatePopupMenu();
    soundMenu = CreatePopupMenu();
    schemeMenu = CreatePopupMenu();
    pttMenu = CreatePopupMenu();
    micMenu = CreatePopupMenu();
    pttModeMenu = CreatePopupMenu();
    quickPttMenu = CreatePopupMenu();

    if (!mainMenu ||
        !hotkeysMenu ||
        !settingsMenu ||
        !languageMenu ||
        !iconMenu ||
        !soundMenu ||
        !schemeMenu ||
        !pttMenu ||
        !micMenu ||
        !pttModeMenu ||
        !quickPttMenu) {

        if (mainMenu) DestroyMenu(mainMenu);
        if (hotkeysMenu) DestroyMenu(hotkeysMenu);
        if (settingsMenu) DestroyMenu(settingsMenu);
        if (languageMenu) DestroyMenu(languageMenu);
        if (iconMenu) DestroyMenu(iconMenu);
        if (soundMenu) DestroyMenu(soundMenu);
        if (schemeMenu) DestroyMenu(schemeMenu);
        if (pttMenu) DestroyMenu(pttMenu);
        if (micMenu) DestroyMenu(micMenu);
        if (pttModeMenu) DestroyMenu(pttModeMenu);
        if (quickPttMenu) DestroyMenu(quickPttMenu);

        return;
    }

    if (context->state.lang == LANG_RU) {
        mouse4Str = "Мышь 4";
        mouse5Str = "Мышь 5";
    } else {
        mouse4Str = "Mouse4";
        mouse5Str = "Mouse5";
    }

    if (context->state.pttMode) {
        if (context->state.pttButton == XBUTTON1) {
            snprintf(
                pttHeaderText,
                sizeof(pttHeaderText),
                context->state.lang == LANG_RU
                    ? "~ Рация (%s) ~"
                    : "~ Walkie-talkie (%s) ~",
                mouse4Str
            );
        } else if (context->state.pttButton == XBUTTON2) {
            snprintf(
                pttHeaderText,
                sizeof(pttHeaderText),
                context->state.lang == LANG_RU
                    ? "~ Рация (%s) ~"
                    : "~ Walkie-talkie (%s) ~",
                mouse5Str
            );
        } else if (context->state.vkPttCustom == VK_OEM_3) {
            snprintf(
                pttHeaderText,
                sizeof(pttHeaderText),
                context->state.lang == LANG_RU
                    ? "~ Рация (~) ~"
                    : "~ Walkie-talkie (~) ~"
            );
        } else if (context->state.vkPttCustom != 0) {
            hotkeyStr = format_hotkey(
                0,
                context->state.vkPttCustom
            );

            snprintf(
                pttHeaderText,
                sizeof(pttHeaderText),
                context->state.lang == LANG_RU
                    ? "~ Рация (%s) ~"
                    : "~ Walkie-talkie (%s) ~",
                hotkeyStr
            );
        } else {
            snprintf(
                pttHeaderText,
                sizeof(pttHeaderText),
                context->state.lang == LANG_RU
                    ? "~ Рация ~"
                    : "~ Walkie-talkie ~"
            );
        }
    } else {
        snprintf(
            pttHeaderText,
            sizeof(pttHeaderText),
            context->state.lang == LANG_RU
                ? "~ Рация: выкл ~"
                : "~ Walkie-talkie: off ~"
        );
    }

    snprintf(
        soundHeaderText,
        sizeof(soundHeaderText),
        context->state.lang == LANG_RU
            ? "~ Громкость: %d%% ~"
            : "~ Volume: %d%% ~",
        context->state.soundVolume
    );

    if (!context->state.muted) {
        micBase = context->state.lang == LANG_RU
            ? "Микрофон ВКЛ"
            : "Microphone ON";
    } else {
        micBase = context->state.lang == LANG_RU
            ? "Микрофон ВЫКЛ"
            : "Microphone OFF";
    }

    if (context->state.vkMicChordFirst != 0 &&
        context->state.vkMicChordSecond != 0) {

        key_build_chord_name(
            context->state.vkMicChordFirst,
            context->state.vkMicChordSecond,
            chordName,
            sizeof(chordName)
        );

        snprintf(
            micHotkeyText,
            sizeof(micHotkeyText),
            "%s (%s)",
            micBase,
            chordName
        );
    } else {
        hotkeyStr = format_hotkey(
            context->state.modToggle,
            context->state.vkToggle
        );

        snprintf(
            micHotkeyText,
            sizeof(micHotkeyText),
            "%s (%s)",
            micBase,
            hotkeyStr
        );
    }

    quickChangeText = context->state.lang == LANG_RU
        ? "Изменить…"
        : "Change…";

    quickRemoveText = context->state.lang == LANG_RU
        ? "Удалить"
        : "Remove";

    assignText = context->state.lang == LANG_RU
        ? "Назначить…"
        : "Assign…";

    if (context->state.pttButton == XBUTTON1) {
        hotkeyStr = mouse4Str;
    } else if (context->state.pttButton == XBUTTON2) {
        hotkeyStr = mouse5Str;
    } else if (context->state.vkPttCustom == VK_OEM_3) {
        hotkeyStr = "~";
    } else if (context->state.vkPttCustom != 0) {
        hotkeyStr = format_hotkey(
            0,
            context->state.vkPttCustom
        );
    } else {
        hotkeyStr = "";
    }

    snprintf(
        pttCustomText,
        sizeof(pttCustomText),
        context->state.lang == LANG_RU
            ? "Рация (%s) | Настроить…"
            : "Walkie-talkie (%s) | Edit",
        hotkeyStr
    );

    if (context->state.vkMicChordFirst != 0 &&
        context->state.vkMicChordSecond != 0) {

        key_build_chord_name(
            context->state.vkMicChordFirst,
            context->state.vkMicChordSecond,
            chordName,
            sizeof(chordName)
        );

        snprintf(
            micCustomText,
            sizeof(micCustomText),
            context->state.lang == LANG_RU
                ? "Микрофон (%s) | Настроить…"
                : "Microphone (%s) | Edit",
            chordName
        );
    } else {
        hotkeyStr = format_hotkey(
            context->state.modToggle,
            context->state.vkToggle
        );

        snprintf(
            micCustomText,
            sizeof(micCustomText),
            context->state.lang == LANG_RU
                ? "Микрофон (%s) | Настроить…"
                : "Microphone (%s) | Edit",
            hotkeyStr
        );
    }

    if (passive_ptt_is_enabled()) {
        hotkeyStr = format_hotkey(
            0,
            passive_ptt_get_double_vk()
        );

        snprintf(
            quickPttText,
            sizeof(quickPttText),
            context->state.lang == LANG_RU
                ? "Быстрый РТТ: двойной тап (%s)"
                : "Quick PTT: double tap (%s)",
            hotkeyStr
        );
    } else {
        snprintf(
            quickPttText,
            sizeof(quickPttText),
            context->state.lang == LANG_RU
                ? "Быстрый РТТ: <не назначено>"
                : "Quick PTT: double tap (off)"
				
        );
    }

    menu_append_utf8(
        mainMenu,
        MF_STRING | MF_GRAYED | MF_DISABLED,
        ID_TRAY_PTT_HEADER,
        pttHeaderText
    );

          if (context->state.vkPttModeChordFirst != 0 &&
        context->state.vkPttModeChordSecond != 0) {

        key_build_chord_name(
            context->state.vkPttModeChordFirst,
            context->state.vkPttModeChordSecond,
            chordName,
            sizeof(chordName)
        );

        hotkeyStr = chordName;
    } else {
        hotkeyStr = format_hotkey(
            context->state.modPttModeToggle,
            context->state.vkPttModeToggle
        );
    }

    snprintf(
        pttModeText,
        sizeof(pttModeText),
        context->state.lang == LANG_RU
            ? "Режим РТТ (%s)"
            : "PTT mode (%s)",
        hotkeyStr
    );

        menu_append_utf8(
        mainMenu,
        MF_STRING | (context->state.pttMode ? MF_CHECKED : 0),
        ID_TRAY_PTT_MODE,
        pttModeText
    );

    menu_append_utf8(
        mainMenu,
        MF_STRING | (!context->state.muted ? MF_CHECKED : 0),
        ID_TRAY_MIC_TOGGLE,
        micHotkeyText
    );

    menu_append_utf8(
        mainMenu,
        MF_SEPARATOR,
        0,
        NULL
    );

    menu_append_utf8(
        pttMenu,
        MF_STRING | (context->state.pttButton == XBUTTON1 &&
                     context->state.vkPttCustom == 0 ? MF_CHECKED : 0),
        ID_HK_PTT_MOUSE4,
        mouse4Str
    );

    menu_append_utf8(
        pttMenu,
        MF_STRING | (context->state.pttButton == XBUTTON2 &&
                     context->state.vkPttCustom == 0 ? MF_CHECKED : 0),
        ID_HK_PTT_MOUSE5,
        mouse5Str
    );

    menu_append_utf8(
        pttMenu,
        MF_STRING | (context->state.vkPttCustom == VK_OEM_3 ? MF_CHECKED : 0),
        ID_HK_PTT_TILDE,
        localization_get(context->state.lang, "MENU_TILDE")
    );

    menu_append_utf8(
        micMenu,
        MF_STRING | (context->state.modToggle == 0 &&
                     context->state.vkToggle == VK_F9 &&
                     context->state.vkMicChordFirst == 0 ? MF_CHECKED : 0),
        ID_HK_MIC_F9,
        "F9"
    );

    menu_append_utf8(
        micMenu,
        MF_STRING | (context->state.modToggle == 0 &&
                     context->state.vkToggle == VK_F10 &&
                     context->state.vkMicChordFirst == 0 ? MF_CHECKED : 0),
        ID_HK_MIC_F10,
        "F10"
    );

    menu_append_utf8(
        micMenu,
        MF_STRING | (context->state.modToggle == 0 &&
                     context->state.vkToggle == VK_OEM_3 &&
                     context->state.vkMicChordFirst == 0 ? MF_CHECKED : 0),
        ID_HK_MIC_TILDE,
        localization_get(context->state.lang, "MENU_TILDE")
    );

    menu_append_utf8(
        hotkeysMenu,
        MF_POPUP,
        (UINT_PTR)pttMenu,
        localization_get(context->state.lang, "MENU_PTT_BUTTON")
    );

    menu_append_utf8(
        hotkeysMenu,
        MF_POPUP,
        (UINT_PTR)micMenu,
        localization_get(context->state.lang, "MENU_MIC_BUTTON")
    );

    AppendMenuW(hotkeysMenu, MF_SEPARATOR, 0, NULL);

    menu_append_utf8(
        hotkeysMenu,
        MF_STRING,
        ID_HK_PTT_CUSTOM,
        pttCustomText
    );

    menu_append_utf8(
        hotkeysMenu,
        MF_STRING,
        ID_HK_MIC_CUSTOM,
        micCustomText
    );

    AppendMenuW(hotkeysMenu, MF_SEPARATOR, 0, NULL);

    {
        char pttModeMenuText[160];
        const char *pttModeResetText;

        pttModeResetText = context->state.lang == LANG_RU
            ? "Сбросить"
            : "Reset";

        if (context->state.vkPttModeChordFirst != 0 &&
            context->state.vkPttModeChordSecond != 0) {

            key_build_chord_name(
                context->state.vkPttModeChordFirst,
                context->state.vkPttModeChordSecond,
                chordName,
                sizeof(chordName)
            );

            snprintf(
                pttModeMenuText,
                sizeof(pttModeMenuText),
                context->state.lang == LANG_RU
                    ? "Режим РТТ (%s)"
                    : "PTT mode (%s)",
                chordName
            );
        } else {
            hotkeyStr = format_hotkey(
                context->state.modPttModeToggle,
                context->state.vkPttModeToggle
            );

            snprintf(
                pttModeMenuText,
                sizeof(pttModeMenuText),
                context->state.lang == LANG_RU
                    ? "Режим РТТ: настройка (%s)"
                    : "PTT mode: configure (%s)",
                hotkeyStr
            );
        }

        menu_append_utf8(
            hotkeysMenu,
            MF_POPUP,
            (UINT_PTR)pttModeMenu,
            pttModeMenuText
        );

        menu_append_utf8(
            pttModeMenu,
            MF_STRING,
            ID_HK_PTT_MODE_CHANGE,
            quickChangeText
        );

        menu_append_utf8(
            pttModeMenu,
            MF_STRING,
            ID_HK_PTT_MODE_RESET,
            pttModeResetText
        );
    }

    menu_append_utf8(
        hotkeysMenu,
        MF_POPUP,
        (UINT_PTR)quickPttMenu,
        quickPttText
    );

    menu_append_utf8(
        quickPttMenu,
        MF_STRING,
        passive_ptt_is_enabled()
            ? ID_HK_QUICK_PTT_CHANGE
            : ID_HK_QUICK_PTT_ASSIGN,
        passive_ptt_is_enabled()
            ? quickChangeText
            : assignText
    );

    menu_append_utf8(
        quickPttMenu,
        MF_STRING,
        ID_HK_QUICK_PTT_REMOVE,
        quickRemoveText
    );

    menu_insert_utf8(
        mainMenu,
        -1,
        MF_BYPOSITION | MF_POPUP,
        (UINT_PTR)hotkeysMenu,
        localization_get(context->state.lang, "MENU_HOTKEYS")
    );

    menu_append_utf8(
        languageMenu,
        MF_STRING | (context->state.lang == LANG_EN ? MF_CHECKED : 0),
        ID_LANG_EN,
        "English"
    );

    menu_append_utf8(
        languageMenu,
        MF_STRING | (context->state.lang == LANG_RU ? MF_CHECKED : 0),
        ID_LANG_RU,
        "Русский"
    );

    menu_append_utf8(
        settingsMenu,
        MF_POPUP,
        (UINT_PTR)languageMenu,
        localization_get(context->state.lang, "MENU_LANGUAGE")
    );

    menu_append_utf8(
        soundMenu,
        MF_STRING | MF_GRAYED | MF_DISABLED,
        ID_SOUND_HEADER,
        soundHeaderText
    );

    menu_append_utf8(
        schemeMenu,
        MF_STRING | (context->state.soundScheme == 0 ? MF_CHECKED : 0),
        ID_SOUND_SCHEME_CLASSIC,
        "Classic"
    );

    menu_append_utf8(
        schemeMenu,
        MF_STRING | (context->state.soundScheme == 1 ? MF_CHECKED : 0),
        ID_SOUND_SCHEME_MICPLUS,
        "MicPlus"
    );

    menu_append_utf8(
        soundMenu,
        MF_POPUP,
        (UINT_PTR)schemeMenu,
        context->state.lang == LANG_RU
            ? "Звуковая схема"
            : "Sound scheme"
    );

    AppendMenuW(soundMenu, MF_SEPARATOR, 0, NULL);

    menu_append_utf8(
        soundMenu,
        MF_STRING | (context->state.soundVolume == 0 ? MF_CHECKED : 0),
        ID_SOUND_VOLUME_0,
        context->state.lang == LANG_RU
            ? "Выкл."
            : "Off"
    );

    menu_append_utf8(
        soundMenu,
        MF_STRING | (context->state.soundVolume == 25 ? MF_CHECKED : 0),
        ID_SOUND_VOLUME_25,
        "25%"
    );

    menu_append_utf8(
        soundMenu,
        MF_STRING | (context->state.soundVolume == 50 ? MF_CHECKED : 0),
        ID_SOUND_VOLUME_50,
        "50%"
    );

    menu_append_utf8(
        soundMenu,
        MF_STRING | (context->state.soundVolume == 75 ? MF_CHECKED : 0),
        ID_SOUND_VOLUME_75,
        "75%"
    );

    menu_append_utf8(
        soundMenu,
        MF_STRING | (context->state.soundVolume == 100 ? MF_CHECKED : 0),
        ID_SOUND_VOLUME_100,
        "100%"
    );

    AppendMenuW(soundMenu, MF_SEPARATOR, 0, NULL);

    menu_append_utf8(
        soundMenu,
        MF_STRING | (context->state.enableMicSounds ? MF_CHECKED : 0),
        ID_SOUND_MIC,
        context->state.lang == LANG_RU
            ? "Сигналы микрофона"
            : "Mic sounds"
    );

    menu_append_utf8(
        soundMenu,
        MF_STRING | (context->state.enablePTTSounds ? MF_CHECKED : 0),
        ID_SOUND_PTT,
        context->state.lang == LANG_RU
            ? "Сигналы рации (РТТ)"
            : "PTT sounds"
    );

    menu_append_utf8(
        soundMenu,
        MF_STRING | (context->state.enablePTTModeSounds ? MF_CHECKED : 0),
        ID_SOUND_PTT_MODE,
        context->state.lang == LANG_RU
            ? "Сигналы режима РТТ"
            : "PTT mode sounds"
    );

    menu_append_utf8(
        settingsMenu,
        MF_POPUP,
        (UINT_PTR)soundMenu,
        context->state.lang == LANG_RU
            ? "Звуки и сигналы"
            : "Sound config"
    );

    menu_append_utf8(
        iconMenu,
        MF_STRING | (context->state.iconMode == 0 ? MF_CHECKED : 0),
        ID_SETTINGS_ICON_AUTO,
        context->state.lang == LANG_RU
            ? "Авто"
            : "Auto"
    );

    menu_append_utf8(
        iconMenu,
        MF_STRING | (context->state.iconMode == 1 ? MF_CHECKED : 0),
        ID_SETTINGS_ICON_BLACK,
        context->state.lang == LANG_RU
            ? "Белый"
            : "White"
    );

    menu_append_utf8(
        iconMenu,
        MF_STRING | (context->state.iconMode == 2 ? MF_CHECKED : 0),
        ID_SETTINGS_ICON_WHITE,
        context->state.lang == LANG_RU
            ? "Тёмный"
            : "Black"
    );

    menu_append_utf8(
        settingsMenu,
        MF_POPUP,
        (UINT_PTR)iconMenu,
        localization_get(context->state.lang, "MENU_ICON_COLOR")
    );

    menu_append_utf8(
        settingsMenu,
        MF_SEPARATOR,
        0,
        NULL
    );

    menu_append_utf8(
        settingsMenu,
        MF_STRING | (context->state.autoStart ? MF_CHECKED : 0),
        ID_SETTINGS_AUTOSTART,
        localization_get(context->state.lang, "MENU_AUTOSTART")
    );

    menu_append_utf8(
        settingsMenu,
        MF_STRING | (context->state.enableBalloonTips ? MF_CHECKED : 0),
        ID_SETTINGS_BALLOON,
        localization_get(context->state.lang, "MENU_BALLOON")
    );

    menu_append_utf8(
        settingsMenu,
        MF_SEPARATOR,
        0,
        NULL
    );

    menu_append_utf8(
        settingsMenu,
        MF_STRING,
        ID_SETTINGS_ABOUT,
        localization_get(context->state.lang, "MENU_ABOUT")
    );

    menu_insert_utf8(
        mainMenu,
        -1,
        MF_BYPOSITION | MF_POPUP,
        (UINT_PTR)settingsMenu,
        localization_get(context->state.lang, "MENU_SETTINGS")
    );

    menu_insert_utf8(
        mainMenu,
        -1,
        MF_BYPOSITION | MF_SEPARATOR,
        0,
        NULL
    );

    menu_insert_utf8(
        mainMenu,
        -1,
        MF_BYPOSITION | MF_STRING,
        ID_TRAY_EXIT,
        localization_get(context->state.lang, "MENU_EXIT")
    );

    SetForegroundWindow(hWnd);

    TrackPopupMenu(
        mainMenu,
        TPM_BOTTOMALIGN | TPM_LEFTALIGN,
        cursorPosition.x,
        cursorPosition.y,
        0,
        hWnd,
        NULL
    );

    SendMessageA(hWnd, WM_NULL, 0, 0);

    DestroyMenu(mainMenu);
}