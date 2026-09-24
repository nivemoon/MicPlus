#include "unicode.h"
#define _CRT_SECURE_NO_WARNINGS

#include "tray_menu.h"

#include "key_capture.h"
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

#include <stdio.h>

#define MICPLUS_WEBSITE \
    "https://github.com/nivemoon/micplus"

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
        case VK_OEM_3: keyName = "~"; break;
        case VK_SPACE: keyName = "Space"; break;
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
    theme_update_icon(&context->state.hIconOnCurrent, context->state.hIconOnLight, context->state.hIconOnDark, context->state.iconMode);
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
    if (!key_capture_ptt(hWnd, context->state.lang, &virtualKey)) {
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
    if (!key_capture_hotkey(hWnd, context->state.lang, "MSG_TOGGLE_SELECT_TEXT", "MSG_TOGGLE_SELECT_TITLE", &modifiers, &virtualKey)) {
        return;
    }
    if (modifiers == MOD_CONTROL && virtualKey == VK_F9) {
        if (context->state.lang == LANG_RU) {
            MessageBoxW(hWnd, L"Ctrl+F9 зарезервирована для управления звуком.", L"MicPlus", MB_OK | MB_ICONWARNING);
        } else {
            MessageBoxA(hWnd, "Ctrl+F9 is reserved for sound control.", "MicPlus", MB_OK | MB_ICONWARNING);
        }
        return;
    }
    context->state.modToggle = modifiers;
    context->state.vkToggle = virtualKey;
    context->state.toggleCustomSet = 1;
    tray_menu_save(context);
    tray_menu_refresh(context);
}

static INT_PTR CALLBACK tray_about_dialog_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    AppContext *context;
    context = (AppContext *)GetWindowLongPtrA(hDlg, DWLP_USER);

    switch (message) {
        case WM_INITDIALOG:
            SetWindowLongPtrA(hDlg, DWLP_USER, (LONG_PTR)lParam);
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
                    if (!context) return TRUE;

                    if (context->state.lang == LANG_RU) {
                        static const WCHAR russianPrompt[] =
                            L"Удалить файл настроек MicPlus?\n\n"
                            L"Все сохранённые настройки будут удалены.\n"
                            L"После удаления программа будет закрыта.\n"
                            L"При следующем запуске будут использованы "
                            L"настройки по умолчанию.\n\n"
                            L"Продолжить?";

                        if (MessageBoxW(hDlg, russianPrompt, L"MicPlus", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) {
                            return TRUE;
                        }
                    } else {
                        if (MessageBoxA(hDlg, "Delete the MicPlus settings file?\n\n"
                            "All saved settings will be deleted.\n"
                            "The program will close after deletion.\n"
                            "Default settings will be used next time.\n\n"
                            "Continue?", "MicPlus", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) {
                            return TRUE;
                        }
                    }

                    if (DeleteFileA(context->paths.iniPath)) {
                        EndDialog(hDlg, 1);
                        PostQuitMessage(0);
                    } else {
                        if (context->state.lang == LANG_RU) {
                            MessageBoxW(hDlg, L"Не удалось удалить файл настроек.", L"MicPlus", MB_OK | MB_ICONERROR);
                        } else {
                            MessageBoxA(hDlg, "The settings file could not be deleted.", "MicPlus", MB_OK | MB_ICONERROR);
                        }
                    }
                    return TRUE;
            }
            break;

        case WM_NOTIFY:
            if (lParam) {
                NMHDR *notificationHeader = (NMHDR *)lParam;
                if (notificationHeader->idFrom == IDC_ABOUT_LINK && notificationHeader->code == NM_CLICK) {
                    ShellExecuteA(hDlg, "open", MICPLUS_WEBSITE, NULL, NULL, SW_SHOWNORMAL);
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

    result = DialogBoxParamA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(IDD_ABOUT), hWnd, tray_about_dialog_proc, (LPARAM)context);

    if (result == -1) {
        char errorMessage[128];
        snprintf(errorMessage, sizeof(errorMessage), "DialogBoxParamA failed.\nError code: %lu", (unsigned long)GetLastError());
        MessageBoxA(hWnd, errorMessage, "MicPlus", MB_OK | MB_ICONERROR);
    }
}

void tray_menu_handle_command(AppContext *context, HWND hWnd, UINT command)
{
    if (!context) return;

    switch (command) {
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
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;
        case ID_HK_MIC_F10:
            context->state.vkToggle = VK_F10;
            context->state.modToggle = 0;
            context->state.toggleCustomSet = 0;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;
        case ID_HK_MIC_TILDE:
            context->state.vkToggle = VK_OEM_3;
            context->state.modToggle = 0;
            context->state.toggleCustomSet = 0;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;
        case ID_HK_MIC_CUSTOM:
            tray_menu_select_mic_hotkey(context, hWnd);
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
            startup_set(&context->paths, context->state.autoStart);
            tray_menu_save(context);
            break;
        case ID_SETTINGS_MIC_SOUNDS:
            context->state.enableMicSounds = !context->state.enableMicSounds;
            tray_menu_save(context);
            break;
        case ID_SETTINGS_PTT_SOUNDS:
            context->state.enablePTTSounds = !context->state.enablePTTSounds;
            tray_menu_save(context);
            break;
        case ID_SETTINGS_BALLOON:
            context->state.enableBalloonTips = !context->state.enableBalloonTips;
            tray_menu_save(context);
            if (context->state.enableBalloonTips) {
                tray_show_mic_balloon(&context->state, context->state.muted);
            }
            break;
        case ID_SETTINGS_ABOUT:
            tray_menu_show_about(context, hWnd);
            break;
    }
}

void tray_menu_show(AppContext *context, HWND hWnd)
{
    POINT cursorPosition;
    char micHotkeyText[128];
    char pttHotkeyText[128];
    const char *micBase;
    const char *pttBase;
    const char *hotkeyStr;
    const char *mouse4Str;
    const char *mouse5Str;

    HMENU mainMenu;
    HMENU hotkeysMenu;
    HMENU settingsMenu;
    HMENU languageMenu;
    HMENU iconMenu;
    HMENU pttMenu;
    HMENU micMenu;

    if (!context || !hWnd || !GetCursorPos(&cursorPosition)) {
        return;
    }

    mainMenu = CreatePopupMenu();
    hotkeysMenu = CreatePopupMenu();
    settingsMenu = CreatePopupMenu();
    languageMenu = CreatePopupMenu();
    iconMenu = CreatePopupMenu();
    pttMenu = CreatePopupMenu();
    micMenu = CreatePopupMenu();

    if (!mainMenu || !hotkeysMenu || !settingsMenu || !languageMenu || !iconMenu || !pttMenu || !micMenu) {
        if (mainMenu) DestroyMenu(mainMenu);
        if (hotkeysMenu) DestroyMenu(hotkeysMenu);
        if (settingsMenu) DestroyMenu(settingsMenu);
        if (languageMenu) DestroyMenu(languageMenu);
        if (iconMenu) DestroyMenu(iconMenu);
        if (pttMenu) DestroyMenu(pttMenu);
        if (micMenu) DestroyMenu(micMenu);
        return;
    }

    // Mouse labels based on language
    if (context->state.lang == LANG_RU) {
        mouse4Str = "Мышь 4";
        mouse5Str = "Мышь 5";
    } else {
        mouse4Str = "Mouse4";
        mouse5Str = "Mouse5";
    }

    // Build PTT text
    if (context->state.pttMode) {
        pttBase = context->state.lang == LANG_RU ? "Режим РТТ" : "PTT Mode";
    } else {
        pttBase = context->state.lang == LANG_RU ? "Включить РТТ" : "Enable PTT";
    }
    
    if (context->state.pttButton == XBUTTON1) {
        snprintf(pttHotkeyText, sizeof(pttHotkeyText), "%s (%s)", pttBase, mouse4Str);
    } else if (context->state.pttButton == XBUTTON2) {
        snprintf(pttHotkeyText, sizeof(pttHotkeyText), "%s (%s)", pttBase, mouse5Str);
    } else if (context->state.vkPttCustom == VK_OEM_3) {
        snprintf(pttHotkeyText, sizeof(pttHotkeyText), "%s (~)", pttBase);
    } else if (context->state.vkPttCustom != 0) {
        hotkeyStr = format_hotkey(0, context->state.vkPttCustom);
        snprintf(pttHotkeyText, sizeof(pttHotkeyText), "%s (%s)", pttBase, hotkeyStr);
    } else {
        snprintf(pttHotkeyText, sizeof(pttHotkeyText), "%s", pttBase);
    }

    // Build Mic text
    if (!context->state.muted) {
        micBase = context->state.lang == LANG_RU ? "Микрофон ВКЛ" : "Microphone ON";
    } else {
        micBase = context->state.lang == LANG_RU ? "Микрофон ВЫКЛ" : "Microphone OFF";
    }
    
    hotkeyStr = format_hotkey(context->state.modToggle, context->state.vkToggle);
    snprintf(micHotkeyText, sizeof(micHotkeyText), "%s (%s)", micBase, hotkeyStr);

    menu_append_utf8(mainMenu, MF_STRING | (context->state.pttMode ? MF_CHECKED : 0), ID_TRAY_PTT_MODE, pttHotkeyText);
    menu_append_utf8(mainMenu, MF_STRING | (!context->state.muted ? MF_CHECKED : 0), ID_TRAY_MIC_TOGGLE, micHotkeyText);
    menu_append_utf8(mainMenu, MF_SEPARATOR, 0, NULL);

    menu_append_utf8(pttMenu, MF_STRING | (context->state.pttButton == XBUTTON1 && context->state.vkPttCustom == 0 ? MF_CHECKED : 0), ID_HK_PTT_MOUSE4, mouse4Str);
    menu_append_utf8(pttMenu, MF_STRING | (context->state.pttButton == XBUTTON2 && context->state.vkPttCustom == 0 ? MF_CHECKED : 0), ID_HK_PTT_MOUSE5, mouse5Str);
    menu_append_utf8(pttMenu, MF_STRING | (context->state.vkPttCustom == VK_OEM_3 ? MF_CHECKED : 0), ID_HK_PTT_TILDE, localization_get(context->state.lang, "MENU_TILDE"));

    menu_append_utf8(micMenu, MF_STRING | (context->state.modToggle == 0 && context->state.vkToggle == VK_F9 ? MF_CHECKED : 0), ID_HK_MIC_F9, "F9");
    menu_append_utf8(micMenu, MF_STRING | (context->state.modToggle == 0 && context->state.vkToggle == VK_F10 ? MF_CHECKED : 0), ID_HK_MIC_F10, "F10");
    menu_append_utf8(micMenu, MF_STRING | (context->state.modToggle == 0 && context->state.vkToggle == VK_OEM_3 ? MF_CHECKED : 0), ID_HK_MIC_TILDE, localization_get(context->state.lang, "MENU_TILDE"));

    menu_append_utf8(hotkeysMenu, MF_POPUP, (UINT_PTR)pttMenu, localization_get(context->state.lang, "MENU_PTT_BUTTON"));
    menu_append_utf8(hotkeysMenu, MF_POPUP, (UINT_PTR)micMenu, localization_get(context->state.lang, "MENU_MIC_BUTTON"));
    menu_append_utf8(hotkeysMenu, MF_SEPARATOR, 0, NULL);
    menu_append_utf8(hotkeysMenu, MF_STRING, ID_HK_PTT_CUSTOM, localization_get(context->state.lang, "MENU_PTT_CUSTOM"));
    menu_append_utf8(hotkeysMenu, MF_STRING, ID_HK_MIC_CUSTOM, localization_get(context->state.lang, "MENU_MIC_CUSTOM"));

    menu_insert_utf8(mainMenu, -1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hotkeysMenu, localization_get(context->state.lang, "MENU_HOTKEYS"));

    menu_append_utf8(languageMenu, MF_STRING | (context->state.lang == LANG_EN ? MF_CHECKED : 0), ID_LANG_EN, "English");
    menu_append_utf8(languageMenu, MF_STRING | (context->state.lang == LANG_RU ? MF_CHECKED : 0), ID_LANG_RU, "Русский");

    menu_append_utf8(settingsMenu, MF_POPUP, (UINT_PTR)languageMenu, localization_get(context->state.lang, "MENU_LANGUAGE"));
    menu_append_utf8(iconMenu, MF_STRING | (context->state.iconMode == 0 ? MF_CHECKED : 0), ID_SETTINGS_ICON_AUTO, localization_get(context->state.lang, "ICON_AUTO"));
    menu_append_utf8(iconMenu, MF_STRING | (context->state.iconMode == 1 ? MF_CHECKED : 0), ID_SETTINGS_ICON_BLACK, localization_get(context->state.lang, "ICON_BLACK"));
    menu_append_utf8(iconMenu, MF_STRING | (context->state.iconMode == 2 ? MF_CHECKED : 0), ID_SETTINGS_ICON_WHITE, localization_get(context->state.lang, "ICON_WHITE"));

    menu_append_utf8(settingsMenu, MF_POPUP, (UINT_PTR)iconMenu, localization_get(context->state.lang, "MENU_ICON_COLOR"));
    menu_append_utf8(settingsMenu, MF_SEPARATOR, 0, NULL);
    menu_append_utf8(settingsMenu, MF_STRING | (context->state.autoStart ? MF_CHECKED : 0), ID_SETTINGS_AUTOSTART, localization_get(context->state.lang, "MENU_AUTOSTART"));
    menu_append_utf8(settingsMenu, MF_STRING | (context->state.enableMicSounds ? MF_CHECKED : 0), ID_SETTINGS_MIC_SOUNDS, localization_get(context->state.lang, "MENU_MIC_SOUNDS"));
    menu_append_utf8(settingsMenu, MF_STRING | (context->state.enablePTTSounds ? MF_CHECKED : 0), ID_SETTINGS_PTT_SOUNDS, localization_get(context->state.lang, "MENU_PTT_SOUNDS"));
    menu_append_utf8(settingsMenu, MF_STRING | (context->state.enableBalloonTips ? MF_CHECKED : 0), ID_SETTINGS_BALLOON, localization_get(context->state.lang, "MENU_BALLOON"));
    menu_append_utf8(settingsMenu, MF_SEPARATOR, 0, NULL);
    menu_append_utf8(settingsMenu, MF_STRING, ID_SETTINGS_ABOUT, localization_get(context->state.lang, "MENU_ABOUT"));

    menu_insert_utf8(mainMenu, -1, MF_BYPOSITION | MF_POPUP, (UINT_PTR)settingsMenu, localization_get(context->state.lang, "MENU_SETTINGS"));
    menu_insert_utf8(mainMenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    menu_insert_utf8(mainMenu, -1, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, localization_get(context->state.lang, "MENU_EXIT"));

    SetForegroundWindow(hWnd);
    TrackPopupMenu(mainMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, cursorPosition.x, cursorPosition.y, 0, hWnd, NULL);
    SendMessageA(hWnd, WM_NULL, 0, 0);
    DestroyMenu(mainMenu);
}