#define _CRT_SECURE_NO_WARNINGS
#include "unicode.h"

#define IDM_AUDIO_DEVICE_BASE   3000
#define IDM_AUDIO_INPUT_BASE    3100
#define ID_ALIAS_RENAME 		3200
#define ID_AUDIO_HIDE_MIC 		3400
#define ID_TRAY_ICON_COLOR      2200
#define ID_TRAY_QUICK_PTT_HEADER 3500

#ifndef ID_KEEP_ALIVE_TOGGLE
#define ID_KEEP_ALIVE_TOGGLE 5460
#endif

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
#include "switch_devices.h"
#include <commctrl.h>
#include <shellapi.h>
#include <windows.h>
#include <stdio.h>
#include "hooks.h"
#include <string.h>
#include "alias.h"
#include "resource.h"
#include "sounds.h"
#include "voice_activation.h"
#include "alias_dialog.h"
#include <stdio.h>
#include "micplus_ini.h"

static BOOL tray_menu_is_windows_11(void)
{
    HKEY key;
    char buildText[32];
    DWORD size = sizeof(buildText);
    DWORD build = 0;

    if (RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            0,
            KEY_READ,
            &key) != ERROR_SUCCESS) {
        return FALSE;
    }

    if (RegQueryValueExA(
            key,
            "CurrentBuildNumber",
            NULL,
            NULL,
            (LPBYTE)buildText,
            &size) == ERROR_SUCCESS) {
        build = (DWORD)atoi(buildText);
    }

    RegCloseKey(key);

    return build >= 22000;
}

#ifndef ID_SETTINGS_AUDIO_ICON
#define ID_SETTINGS_AUDIO_ICON 5450
#endif

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

/* --- Forward declarations для DUSK-блока --- */
static const char *format_hotkey(UINT modifiers, UINT vk);
static void tray_menu_save(AppContext *context);
static void tray_menu_refresh(AppContext *context);

/* ===== Хоткеи DUSK: IDs ===== */
#define ID_HOTKEY_PANEL_DEFAULT   4100  /* D: панель аудиоустройств */
#define ID_HOTKEY_OUTPUT_DEFAULT  4101  /* U: переключить вывод */
#define ID_HOTKEY_LIST_DEFAULT    4102  /* S: список аудиоустройств */
#define ID_HOTKEY_INPUT_DEFAULT   4103  /* K: переключить ввод */

#define ID_HOTKEY_PANEL_SETUP     4110
#define ID_HOTKEY_OUTPUT_SETUP    4111
#define ID_HOTKEY_LIST_SETUP      4112
#define ID_HOTKEY_INPUT_SETUP     4113

#define ID_HOTKEY_PANEL_DISABLE   4120
#define ID_HOTKEY_OUTPUT_DISABLE  4121
#define ID_HOTKEY_LIST_DISABLE    4122
#define ID_HOTKEY_INPUT_DISABLE   4123

/* Дефолтные комбинации DUSK */
#define HOTKEY_DEFAULT_MODS (MOD_CONTROL | MOD_ALT | MOD_SHIFT)
#define HOTKEY_DEFAULT_VK_PANEL  'D'   /* Панель аудиоустройств */
#define HOTKEY_DEFAULT_VK_LIST   'U'   /* Список аудиоустройств */
#define HOTKEY_DEFAULT_VK_OUTPUT 'S'   /* Переключить вывод (звук) */
#define HOTKEY_DEFAULT_VK_INPUT  'K'   /* Переключить ввод (микрофон) */

//* ===== Добавление одного DUSK-подменю ===== */
static void add_dusk_submenu(
    HMENU parent,
    const char *title,
    UINT defaultMods,
    UINT defaultVk,
    UINT currentMods,
    UINT currentVk,
    UINT idDefault,
    UINT idSetup,
    UINT idDisable)
{
    HMENU sub;
    const char *comboText;
    char titleUtf8[128];

    sub = CreatePopupMenu();

    /* Строка 1: ВСЕГДА дефолтная комбинация; клик = сброс */
    comboText = format_hotkey(defaultMods, defaultVk);
    AppendMenuA(sub, MF_STRING, idDefault, comboText);

    AppendMenuW(sub, MF_SEPARATOR, 0, NULL);

    /* Строка 2: Изменить… */
    menu_append_utf8(sub, MF_STRING, idSetup, "Изменить…");

    /* Строка 3: Отключено */
    menu_append_utf8(
        sub,
        MF_STRING | (currentVk == 0 ? MF_CHECKED : 0),
        idDisable,
        "Отключено"
    );

    /* Заголовок: текущая комбинация справа */
    if (currentVk == 0) {
        _snprintf(titleUtf8, sizeof(titleUtf8),
            "%s\t(отключено)", title);
    } else if (currentMods == HOTKEY_DEFAULT_MODS &&
               currentVk >= 'A' && currentVk <= 'Z') {
        _snprintf(titleUtf8, sizeof(titleUtf8),
            "%s\t%c", title, (char)currentVk);
    } else {
        _snprintf(titleUtf8, sizeof(titleUtf8),
            "%s\t%s", title,
            format_hotkey(currentMods, currentVk));
    }

    menu_append_utf8(parent, MF_POPUP, (UINT_PTR)sub, titleUtf8);
}

/* ===== Вставка блока DUSK в «Сочетания клавиш» ===== */
void tray_menu_add_dusk_block(HMENU hotkeysMenu, AppContext *ctx)
{
    /* Порядок: D → U → S → K */
    add_dusk_submenu(hotkeysMenu,
        ctx->state.lang == LANG_RU
            ? "Панель аудиоустройств"
            : "Audio device panel",
        HOTKEY_DEFAULT_MODS, HOTKEY_DEFAULT_VK_PANEL,
        ctx->state.panelHotkeyMods, ctx->state.panelHotkeyVk,
        ID_HOTKEY_PANEL_DEFAULT, ID_HOTKEY_PANEL_SETUP,
        ID_HOTKEY_PANEL_DISABLE);

    add_dusk_submenu(hotkeysMenu,
        ctx->state.lang == LANG_RU
            ? "Список аудиоустройств"
            : "Audio device list",
        HOTKEY_DEFAULT_MODS, HOTKEY_DEFAULT_VK_LIST,
        ctx->state.listHotkeyMods, ctx->state.listHotkeyVk,
        ID_HOTKEY_LIST_DEFAULT, ID_HOTKEY_LIST_SETUP,
        ID_HOTKEY_LIST_DISABLE);

    add_dusk_submenu(hotkeysMenu,
        ctx->state.lang == LANG_RU
            ? "Переключить вывод"
            : "Switch output",
        HOTKEY_DEFAULT_MODS, HOTKEY_DEFAULT_VK_OUTPUT,
        ctx->state.outputHotkeyMods, ctx->state.outputHotkeyVk,
        ID_HOTKEY_OUTPUT_DEFAULT, ID_HOTKEY_OUTPUT_SETUP,
        ID_HOTKEY_OUTPUT_DISABLE);

    add_dusk_submenu(hotkeysMenu,
        ctx->state.lang == LANG_RU
            ? "Переключить ввод"
            : "Switch input",
        HOTKEY_DEFAULT_MODS, HOTKEY_DEFAULT_VK_INPUT,
        ctx->state.inputHotkeyMods, ctx->state.inputHotkeyVk,
        ID_HOTKEY_INPUT_DEFAULT, ID_HOTKEY_INPUT_SETUP,
        ID_HOTKEY_INPUT_DISABLE);
}
static void tray_menu_handle_dusk_command(
    AppContext *context,
    HWND hWnd,
    UINT command)
{
        UINT modifiers;
    UINT virtualKey;
    UINT chordFirst;
    UINT chordSecond;
    
    switch (command) {
        /* --- Сброс на дефолт (клик по строке комбинации) --- */
        case ID_HOTKEY_PANEL_DEFAULT:
            context->state.panelHotkeyMods = HOTKEY_DEFAULT_MODS;
            context->state.panelHotkeyVk = HOTKEY_DEFAULT_VK_PANEL;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HOTKEY_LIST_DEFAULT:
            context->state.listHotkeyMods = HOTKEY_DEFAULT_MODS;
            context->state.listHotkeyVk = HOTKEY_DEFAULT_VK_LIST;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HOTKEY_OUTPUT_DEFAULT:
            context->state.outputHotkeyMods = HOTKEY_DEFAULT_MODS;
            context->state.outputHotkeyVk = HOTKEY_DEFAULT_VK_OUTPUT;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HOTKEY_INPUT_DEFAULT:
            context->state.inputHotkeyMods = HOTKEY_DEFAULT_MODS;
            context->state.inputHotkeyVk = HOTKEY_DEFAULT_VK_INPUT;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        /* --- Отключено --- */
        case ID_HOTKEY_PANEL_DISABLE:
            context->state.panelHotkeyMods = 0;
            context->state.panelHotkeyVk = 0;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HOTKEY_LIST_DISABLE:
            context->state.listHotkeyMods = 0;
            context->state.listHotkeyVk = 0;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HOTKEY_OUTPUT_DISABLE:
            context->state.outputHotkeyMods = 0;
            context->state.outputHotkeyVk = 0;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_HOTKEY_INPUT_DISABLE:
            context->state.inputHotkeyMods = 0;
            context->state.inputHotkeyVk = 0;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        /* --- Изменить… (capture как у микрофона) --- */
        case ID_HOTKEY_PANEL_SETUP:
        case ID_HOTKEY_LIST_SETUP:
        case ID_HOTKEY_OUTPUT_SETUP:
        case ID_HOTKEY_INPUT_SETUP:
            if (!key_capture_mic_hotkey(
                    hWnd,
                    context->state.lang,
                    &modifiers,
                    &virtualKey,
                    &chordFirst,
                    &chordSecond)) {
                return;
            }

            /* Аккорды для DUSK пока не поддерживаем */
            if (chordFirst != 0 && chordSecond != 0) {
                return;
            }

            switch (command) {
                case ID_HOTKEY_PANEL_SETUP:
                    context->state.panelHotkeyMods = modifiers;
                    context->state.panelHotkeyVk = virtualKey;
                    break;

                case ID_HOTKEY_LIST_SETUP:
                    context->state.listHotkeyMods = modifiers;
                    context->state.listHotkeyVk = virtualKey;
                    break;

                case ID_HOTKEY_OUTPUT_SETUP:
                    context->state.outputHotkeyMods = modifiers;
                    context->state.outputHotkeyVk = virtualKey;
                    break;

                case ID_HOTKEY_INPUT_SETUP:
                    context->state.inputHotkeyMods = modifiers;
                    context->state.inputHotkeyVk = virtualKey;
                    break;
            }

            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        default:
            break;
    }
}

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
                keyName = "";
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
    hooks_ptt_unlatch();   /* <-- добавить */

    context->state.pttButton = button;
    context->state.vkPttCustom = 0;
    context->state.pttKeyDown = 0;

    passive_ptt_clear();

    tray_menu_save(context);
    tray_menu_refresh(context);
}

static void tray_menu_set_ptt_disable(AppContext *context)
{
    hooks_ptt_unlatch();      /* <-- добавить */

    context->state.pttButton = 0;
    context->state.vkPttCustom = 0;
    context->state.pttKeyDown = 0;

    passive_ptt_clear();

    tray_menu_save(context);
    tray_menu_refresh(context);
}

static void tray_menu_select_ptt(AppContext *context, HWND hWnd)
{
    UINT virtualKey;
    BOOL isDoubleClick = FALSE;
    BOOL wasDoubleClick =
        (context->state.pttButton == 0 &&
         context->state.vkPttCustom != 0);

if (!key_capture_ptt(
        hWnd,
        context->state.lang,
        &virtualKey,
        &isDoubleClick)) {
    return;
}

hooks_ptt_unlatch();          /* <-- добавить */

context->state.pttButton = 0;
context->state.vkPttCustom = virtualKey;
context->state.pttKeyDown = 0;

if (isDoubleClick) {
    passive_ptt_set_double_vk(virtualKey);
} else {
    passive_ptt_clear();
}

    /* сбросить старое состояние двойного клика,
       включая возможную активную передачу */
    if (wasDoubleClick) {
        passive_ptt_clear();
    }

    context->state.pttButton = 0;
    context->state.vkPttCustom = virtualKey;
    context->state.pttKeyDown = 0;

    if (isDoubleClick) {
        passive_ptt_set_double_vk(virtualKey);
    } else {
        passive_ptt_clear();
    }

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

    passive_ptt_clear();

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
    } else {
        context->state.vkPttModeChordFirst = 0;
        context->state.vkPttModeChordSecond = 0;
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
                        if (MessageBoxA(
                                hDlg,
                                "Delete the MicPlus settings file?\n\n"
                                "All saved settings will be deleted.\n"
                                "The program will close after deletion.\n"
                                "Default settings will be used next time.\n\n"
                                "Continue?",
                                "MicPlus",
                                MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2
                            ) != IDYES) {
                            return TRUE;
                        }
                    }

                    IniDeleteEverywhere();

                    EndDialog(hDlg, 1);
                    PostQuitMessage(0);

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

    if (command >= ID_HOTKEY_PANEL_DEFAULT &&
        command <= ID_HOTKEY_INPUT_DISABLE) {
        tray_menu_handle_dusk_command(context, hWnd, command);
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

        case 4700:
            hooks_set_voice_activation(
                !context->state.voiceActivationEnabled
            );
            tray_menu_refresh(context);
            break;

		case 4801:
			context->state.voiceHangoverMs = 300;
			voice_activation_set_hangover_ms(300);
			settings_save(&context->state, &context->paths);
			break;

case 4802:
    context->state.voiceHangoverMs = 500;
    voice_activation_set_hangover_ms(500);
    settings_save(&context->state, &context->paths);
    break;

case 4803:
    context->state.voiceHangoverMs = 1000;
    voice_activation_set_hangover_ms(1000);
    settings_save(&context->state, &context->paths);
    break;

case 4804:
    context->state.voiceHangoverMs = 2000;
    voice_activation_set_hangover_ms(2000);
    settings_save(&context->state, &context->paths);
    break;

case 4805:
    context->state.voiceThresholdX1000 = 10;
    voice_activation_set_threshold(0.01);
    settings_save(&context->state, &context->paths);
    break;

case 4806:
    context->state.voiceThresholdX1000 = 20;
    voice_activation_set_threshold(0.02);
    settings_save(&context->state, &context->paths);
    break;

case 4807:
    context->state.voiceThresholdX1000 = 30;
    voice_activation_set_threshold(0.03);
    settings_save(&context->state, &context->paths);
    break;

case 4808:
    context->state.voiceThresholdX1000 = 50;
    voice_activation_set_threshold(0.05);
    settings_save(&context->state, &context->paths);
    break;

        case ID_HK_PTT_MOUSE4:
            tray_menu_set_ptt_mouse(context, XBUTTON1);
            break;

        case ID_HK_PTT_MOUSE5:
            tray_menu_set_ptt_mouse(context, XBUTTON2);
            break;

        case ID_HK_PTT_TILDE:
            tray_menu_set_ptt_disable(context);
            break;

        case ID_HK_PTT_CUSTOM:
            tray_menu_select_ptt(context, hWnd);
            break;

        case ID_HK_PTT_MODE_DISABLE:
            context->state.vkPttModeToggle = 0;
            context->state.modPttModeToggle = 0;

            context->state.vkPttModeChordFirst = 0;
            context->state.vkPttModeChordSecond = 0;
            context->state.pttModeChordKeyDown = 0;

            tray_menu_save(context);
            tray_menu_refresh(context);
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

        case ID_ALIAS_RENAME:
            alias_dialog_show(hWnd);
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
            context->state.vkToggle = 0;
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

        case ID_SETTINGS_AUDIO_ICON:
            context->state.useAudioIcon =
                !context->state.useAudioIcon;

            tray_menu_save(context);
            tray_menu_refresh(context);
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

        case ID_SOUND_VOLUME_20:
            context->state.soundVolume = 20;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_VOLUME_40:
            context->state.soundVolume = 40;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_VOLUME_60:
            context->state.soundVolume = 60;
            tray_menu_save(context);
            tray_menu_refresh(context);
            break;

        case ID_SOUND_VOLUME_80:
            context->state.soundVolume = 80;
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
    }
}

void tray_menu_show(AppContext *context, HWND hWnd)
{
    POINT cursorPosition;

    char pttHeaderText[160];
    char soundHeaderText[160];

    char pttCustomText[160];
	char micCustomText[160];
	
   	char chordName[128];

    const char *hotkeyStr;
    const char *mouse4Str;
    const char *mouse5Str;

    HMENU mainMenu;
    HMENU hotkeysMenu;
    HMENU settingsMenu;
    HMENU languageMenu;
    HMENU iconMenu;
    HMENU soundMenu;
    HMENU pttMenu;
    HMENU micMenu;
    HMENU pttModeMenu;


    if (!context || !hWnd || !GetCursorPos(&cursorPosition)) {
        return;
    }

    mainMenu = CreatePopupMenu();
    hotkeysMenu = CreatePopupMenu();
    settingsMenu = CreatePopupMenu();
    languageMenu = CreatePopupMenu();
    iconMenu = CreatePopupMenu();
    soundMenu = CreatePopupMenu();
    pttMenu = CreatePopupMenu();
    micMenu = CreatePopupMenu();
    pttModeMenu = CreatePopupMenu();


    if (!mainMenu || !hotkeysMenu || !settingsMenu || !languageMenu ||
        !iconMenu || !soundMenu || !pttMenu || !micMenu ||
        !pttModeMenu) {

        if (mainMenu) DestroyMenu(mainMenu);
        if (hotkeysMenu) DestroyMenu(hotkeysMenu);
        if (settingsMenu) DestroyMenu(settingsMenu);
        if (languageMenu) DestroyMenu(languageMenu);
        if (iconMenu) DestroyMenu(iconMenu);
        if (soundMenu) DestroyMenu(soundMenu);
        if (pttMenu) DestroyMenu(pttMenu);
        if (micMenu) DestroyMenu(micMenu);
        if (pttModeMenu) DestroyMenu(pttModeMenu);


        return;
    }

    if (context->state.lang == LANG_RU) {
        mouse4Str = "Мышь 4";
        mouse5Str = "Мышь 5";
    } else {
        mouse4Str = "Mouse4";
        mouse5Str = "Mouse5";
    }

    /* --- Заголовок РТТ в главном меню (кнопка рации) --- */
    {
        char pttStatus[160];
                const char *buttonName = NULL;
        BOOL hasPart = FALSE;

        pttStatus[0] = '\0';

        if (!context->state.pttMode) {
            snprintf(
                pttHeaderText,
                sizeof(pttHeaderText),
                context->state.lang == LANG_RU
                    ? "~ Рация: выкл ~"
                    : "~ Walkie-talkie: off ~"
            );
        } else {
            if (passive_ptt_is_enabled() &&
                passive_ptt_get_double_vk() != 0) {
                buttonName = format_hotkey(
                    0,
                    passive_ptt_get_double_vk()
                );
            } else if (context->state.pttButton == XBUTTON1) {
                buttonName = mouse4Str;
            } else if (context->state.pttButton == XBUTTON2) {
                buttonName = mouse5Str;
            } else if (context->state.vkPttCustom == VK_OEM_3) {
                buttonName = "~";
            } else if (context->state.vkPttCustom != 0) {
                buttonName = format_hotkey(
                    0,
                    context->state.vkPttCustom
                );
            }

            if (buttonName) {
                snprintf(
                    pttStatus,
                    sizeof(pttStatus),
                    "%s",
                    buttonName
                );
                hasPart = TRUE;
            }

             if (hasPart) {
                snprintf(
                    pttHeaderText,
                    sizeof(pttHeaderText),
                    context->state.lang == LANG_RU
                        ? "~ Рация: %s ~"
                        : "~ Walkie-talkie: %s ~",
                    pttStatus
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
        }   /* ← закрывает else (РТТ включён) */
    }       /* ← закрывает блок «Заголовок РТТ» */
		
     /* --- Заголовки подменю хоткеев --- */
    {
        const char *pttHotkeyStr;
        const char *micHotkeyStr;
        char pttHotkeyBuf[64];

        if (context->state.pttButton == 0 &&
            context->state.vkPttCustom == 0) {
            pttHotkeyStr = localization_get(
                context->state.lang,
                "MENU_NO_HOTKEY"
            );
        } else if (context->state.vkPttCustom != 0) {
            hotkeyStr = format_hotkey(0, context->state.vkPttCustom);

            strncpy(
                pttHotkeyBuf,
                hotkeyStr,
                sizeof(pttHotkeyBuf) - 1
            );
            pttHotkeyBuf[sizeof(pttHotkeyBuf) - 1] = '\0';

            pttHotkeyStr = pttHotkeyBuf;
        } else {
            pttHotkeyStr = context->state.pttButton == XBUTTON1
                ? mouse4Str
                : mouse5Str;
        }

        if (context->state.vkMicChordFirst != 0 &&
            context->state.vkMicChordSecond != 0) {

            key_build_chord_name(
                context->state.vkMicChordFirst,
                context->state.vkMicChordSecond,
                chordName,
                sizeof(chordName)
            );

            micHotkeyStr = chordName;
        } else if (context->state.vkToggle == 0) {
            micHotkeyStr = localization_get(
                context->state.lang,
                "MENU_NO_HOTKEY"
            );
        } else {
            micHotkeyStr = format_hotkey(
                context->state.modToggle,
                context->state.vkToggle
            );
        }

        snprintf(pttCustomText, sizeof(pttCustomText),
            "%s\t%s",
            localization_get(context->state.lang, "MENU_PTT_BUTTON"),
            pttHotkeyStr);

        snprintf(micCustomText, sizeof(micCustomText),
            "%s\t%s",
            localization_get(context->state.lang, "MENU_MIC_BUTTON"),
            micHotkeyStr);
    }

/* --- Тексты переключателей с хоткеями --- */
char micToggleText[160];
char pttModeToggleText[160];

{
    char micKey[64];

    if (context->state.vkToggle == 0 &&
        context->state.vkMicChordFirst == 0) {
        micKey[0] = '\0';
    } else if (context->state.vkMicChordFirst != 0 &&
               context->state.vkMicChordSecond != 0) {
        key_build_chord_name(
            context->state.vkMicChordFirst,
            context->state.vkMicChordSecond,
            micKey,
            sizeof(micKey)
        );
    } else {
        strncpy(
            micKey,
            format_hotkey(
                context->state.modToggle,
                context->state.vkToggle
            ),
            sizeof(micKey) - 1
        );
        micKey[sizeof(micKey) - 1] = '\0';
    }

    snprintf(
        micToggleText,
        sizeof(micToggleText),
        context->state.lang == LANG_RU
            ? "Микрофон\t%s"
            : "Microphone\t%s",
        micKey
    );
}

{
    char pttModeKey[64];

    if (context->state.vkPttModeToggle == 0 &&
        context->state.vkPttModeChordFirst == 0) {
        pttModeKey[0] = '\0';
    } else if (context->state.vkPttModeChordFirst != 0 &&
               context->state.vkPttModeChordSecond != 0) {
        key_build_chord_name(
            context->state.vkPttModeChordFirst,
            context->state.vkPttModeChordSecond,
            pttModeKey,
            sizeof(pttModeKey)
        );
    } else {
        strncpy(
            pttModeKey,
            format_hotkey(
                context->state.modPttModeToggle,
                context->state.vkPttModeToggle
            ),
            sizeof(pttModeKey) - 1
        );
        pttModeKey[sizeof(pttModeKey) - 1] = '\0';
    }

    snprintf(
        pttModeToggleText,
        sizeof(pttModeToggleText),
        context->state.lang == LANG_RU
            ? "Режим РТТ\t%s"
            : "PTT mode\t%s",
        pttModeKey
    );
}

    /* --- Серая строка «Рация» в главном меню --- */
    menu_append_utf8(
        mainMenu,
        MF_STRING | MF_GRAYED | MF_DISABLED,
        ID_TRAY_PTT_HEADER,
        pttHeaderText
    );

    AppendMenuW(mainMenu, MF_SEPARATOR, 0, NULL);

menu_append_utf8(
    mainMenu,
    MF_STRING | (!context->state.muted ? MF_CHECKED : 0),
    ID_TRAY_MIC_TOGGLE,
    micToggleText
);

menu_append_utf8(
    mainMenu,
    MF_STRING | (context->state.pttMode ? MF_CHECKED : 0),
    ID_TRAY_PTT_MODE,
    pttModeToggleText
);

    /* --- Подменю PTT --- */
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
        MF_STRING | (context->state.pttButton == 0 &&
                     context->state.vkPttCustom == 0 ? MF_CHECKED : 0),
        ID_HK_PTT_TILDE,
        localization_get(context->state.lang, "MENU_NO_HOTKEY")
    );

    menu_append_utf8(
        pttMenu,
        MF_STRING,
        ID_HK_PTT_CUSTOM,
        localization_get(context->state.lang, "MENU_PTT_CUSTOM")
    );

    /* --- Подменю микрофона --- */

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
                     context->state.vkToggle == 0 &&
                     context->state.vkMicChordFirst == 0 ? MF_CHECKED : 0),
        ID_HK_MIC_TILDE,
        localization_get(context->state.lang, "MENU_NO_HOTKEY")
    );

    menu_append_utf8(
        micMenu,
        MF_STRING,
        ID_HK_MIC_CUSTOM,
        localization_get(context->state.lang, "MENU_MIC_CUSTOM")
    );

    AppendMenuW(pttMenu, MF_SEPARATOR, 0, NULL);

    menu_append_utf8(
        pttModeMenu,
        MF_STRING | (context->state.vkPttModeToggle == 'P' &&
                     context->state.modPttModeToggle == MOD_ALT &&
                     context->state.vkPttModeChordFirst == 0
                         ? MF_CHECKED : 0),
        ID_HK_PTT_MODE_RESET,
        "Alt+P"
    );

    AppendMenuW(pttModeMenu, MF_SEPARATOR, 0, NULL);

    menu_append_utf8(
        pttModeMenu,
        MF_STRING | (context->state.vkPttModeToggle == 0 &&
                     context->state.vkPttModeChordFirst == 0
                         ? MF_CHECKED : 0),
        ID_HK_PTT_MODE_DISABLE,
        localization_get(context->state.lang, "MENU_NO_HOTKEY")
    );

    menu_append_utf8(
        pttModeMenu,
        MF_STRING,
        ID_HK_PTT_MODE_CHANGE,
        context->state.lang == LANG_RU
            ? "Назначить…"
            : "Assign…"
    );

    menu_append_utf8(
        pttMenu,
        MF_POPUP,
        (UINT_PTR)pttModeMenu,
        context->state.lang == LANG_RU
            ? "Режим РТТ"
            : "PTT mode"
    );

    menu_append_utf8(
        hotkeysMenu,
        MF_POPUP,
        (UINT_PTR)pttMenu,
        pttCustomText
    );

    menu_append_utf8(
        hotkeysMenu,
        MF_POPUP,
        (UINT_PTR)micMenu,
        micCustomText
    );

    /* --- Хоткеи DUSK --- */
    AppendMenuW(hotkeysMenu, MF_SEPARATOR, 0, NULL);
    tray_menu_add_dusk_block(hotkeysMenu, context);

/* --- Активация голосом --- */
menu_append_utf8(
    mainMenu,
    MF_STRING | (context->state.voiceActivationEnabled ? MF_CHECKED : 0),
    4700,
    context->state.lang == LANG_RU
        ? "Активация голосом\tV+U"
        : "Voice activation\tV+U"
);

 
AppendMenuW(mainMenu, MF_SEPARATOR, 0, NULL);

/* --- Аудиоустройства --- */
{
    SwitchDeviceList outDevices;
    SwitchDeviceList inDevices;
    HMENU mediaMenu;
    int i;
    UINT flags;
    const wchar_t *alias;
    const wchar_t *label;

    mediaMenu = CreatePopupMenu();

    if (switch_devices_get_output(&outDevices)) {
        for (i = 0; i < outDevices.count; i++) {
            flags = MF_STRING;
            if (outDevices.devices[i].isDefault) {
                flags |= MF_CHECKED;
            }

            alias = alias_get(outDevices.devices[i].id);
            label = alias ? alias : outDevices.devices[i].name;

            AppendMenuW(
                mediaMenu,
                flags,
                IDM_AUDIO_DEVICE_BASE + i,
                label
            );
        }
    }

    if (switch_devices_get_input(&inDevices) &&
        inDevices.count > 0) {

        AppendMenuW(mediaMenu, MF_SEPARATOR, 0, NULL);

        for (i = 0; i < inDevices.count; i++) {
            flags = MF_STRING;
            if (inDevices.devices[i].isDefault) {
                flags |= MF_CHECKED;
            }

            alias = alias_get(inDevices.devices[i].id);
            label = alias ? alias : inDevices.devices[i].name;

            AppendMenuW(
                mediaMenu,
                flags,
                IDM_AUDIO_INPUT_BASE + i,
                label
            );
        }
    }

    AppendMenuW(mediaMenu, MF_SEPARATOR, 0, NULL);

    AppendMenuW(
        mediaMenu,
        MF_STRING,
        ID_ALIAS_RENAME,
        context->state.lang == LANG_RU
            ? L"Переименовать устройства…"
            : L"Rename devices…"
    );

    AppendMenuW(
        mediaMenu,
        MF_STRING |
        (context->state.hideMicSelection ? MF_CHECKED : 0),
        ID_AUDIO_HIDE_MIC,
        context->state.lang == LANG_RU
            ? L"Скрыть микрофоны (Левый клик)"
            : L"Hide microphones (Left click)"
    );

    AppendMenuW(
        mediaMenu,
        MF_STRING |
        (context->state.keepHeadphonesAwake ? MF_CHECKED : 0),
        ID_KEEP_ALIVE_TOGGLE,
        context->state.lang == LANG_RU
            ? L"Bluetooth: без спящего режима"
            : L"Bluetooth: no standby"
    );

    menu_insert_utf8(
        mainMenu,
        -1,
        MF_POPUP | MF_BYPOSITION,
        (UINT_PTR)mediaMenu,
        context->state.lang == LANG_RU
            ? "Аудиоустройства"
            : "Audio devices"
    );
}

/* --- Настройки голосового режима --- */
{
    HMENU vadMenu = CreatePopupMenu();

    AppendMenuA(vadMenu, MF_STRING | MF_GRAYED, 0, "Задержка выключения:");

    AppendMenuA(vadMenu,
        MF_STRING | (context->state.voiceHangoverMs == 300 ? MF_CHECKED : 0),
        4801, "300 мс");
    AppendMenuA(vadMenu,
        MF_STRING | (context->state.voiceHangoverMs == 500 ? MF_CHECKED : 0),
        4802, "500 мс");
    AppendMenuA(vadMenu,
        MF_STRING | (context->state.voiceHangoverMs == 1000 ? MF_CHECKED : 0),
        4803, "1000 мс");
    AppendMenuA(vadMenu,
        MF_STRING | (context->state.voiceHangoverMs == 2000 ? MF_CHECKED : 0),
        4804, "2000 мс");

    AppendMenuA(vadMenu, MF_SEPARATOR, 0, NULL);

    AppendMenuA(vadMenu, MF_STRING | MF_GRAYED, 0, "Порог срабатывания:");

    AppendMenuA(vadMenu,
        MF_STRING | (context->state.voiceThresholdX1000 == 10 ? MF_CHECKED : 0),
        4805, "1% (чувствительно)");
    AppendMenuA(vadMenu,
        MF_STRING | (context->state.voiceThresholdX1000 == 20 ? MF_CHECKED : 0),
        4806, "2%");
    AppendMenuA(vadMenu,
        MF_STRING | (context->state.voiceThresholdX1000 == 30 ? MF_CHECKED : 0),
        4807, "3%");
    AppendMenuA(vadMenu,
        MF_STRING | (context->state.voiceThresholdX1000 == 50 ? MF_CHECKED : 0),
        4808, "5% (строго)");

    menu_append_utf8(
        mainMenu,
        MF_POPUP,
        (UINT_PTR)vadMenu,
        context->state.lang == LANG_RU
            ? "Голосовой режим"
            : "Voice mode settings"
    );
}

/* --- Горячие клавиши --- */
menu_insert_utf8(
    mainMenu,
    -1,
    MF_BYPOSITION | MF_POPUP,
    (UINT_PTR)hotkeysMenu,
    localization_get(context->state.lang, "MENU_HOTKEYS")
);

/* --- Настройки --- */
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

snprintf(
    soundHeaderText,
    sizeof(soundHeaderText),
    context->state.lang == LANG_RU
        ? "Громкость звуков: %d%%"
        : "Sound volume: %d%%",
    context->state.soundVolume
);

menu_append_utf8(
    soundMenu,
    MF_STRING | MF_GRAYED | MF_DISABLED,
    ID_SOUND_HEADER,
    soundHeaderText
);

menu_append_utf8(
    soundMenu,
    MF_STRING | (context->state.soundVolume == 20 ? MF_CHECKED : 0),
    ID_SOUND_VOLUME_20,
    ">> 20%"
);

menu_append_utf8(
    soundMenu,
    MF_STRING | (context->state.soundVolume == 40 ? MF_CHECKED : 0),
    ID_SOUND_VOLUME_40,
    ">>> 40%"
);

menu_append_utf8(
    soundMenu,
    MF_STRING | (context->state.soundVolume == 60 ? MF_CHECKED : 0),
    ID_SOUND_VOLUME_60,
    ">>>> 60%"
);

menu_append_utf8(
    soundMenu,
    MF_STRING | (context->state.soundVolume == 80 ? MF_CHECKED : 0),
    ID_SOUND_VOLUME_80,
    ">>>>> 80%"
);

menu_append_utf8(
    soundMenu,
    MF_STRING | (context->state.soundVolume == 100 ? MF_CHECKED : 0),
    ID_SOUND_VOLUME_100,
    ">>>>>> 100%"
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

/* --- Поведение значка --- */
{
    HMENU trayIconMenu = CreatePopupMenu();
    HMENU leftClickMenu = CreatePopupMenu();
    HMENU middleClickMenu = CreatePopupMenu();
    HMENU iconModeMenu = CreatePopupMenu();
    BOOL isWindows11 = tray_menu_is_windows_11();

    menu_append_utf8(
        leftClickMenu,
        MF_STRING |
        (context->state.leftClickAction == 0 ? MF_CHECKED : 0),
        ID_MOUSE_LEFT_PANEL,
        context->state.lang == LANG_RU
            ? "Переключатель"
            : "Switcher"
    );

    menu_append_utf8(
        leftClickMenu,
        MF_STRING |
        (context->state.leftClickAction == 1 ? MF_CHECKED : 0),
        ID_MOUSE_LEFT_MENU,
        context->state.lang == LANG_RU
            ? "Главное меню"
            : "Main menu"
    );

    menu_append_utf8(
        leftClickMenu,
        MF_STRING |
        (context->state.leftClickAction == 2 ? MF_CHECKED : 0),
        ID_MOUSE_LEFT_NONE,
        context->state.lang == LANG_RU
            ? "Нет действия"
            : "No action"
    );

    menu_append_utf8(
        middleClickMenu,
        MF_STRING |
        (context->state.middleClickAction == 0 ? MF_CHECKED : 0),
        ID_MOUSE_MIDDLE_MIC,
        context->state.lang == LANG_RU
            ? "Микрофон (ВКЛ/ВЫКЛ)"
            : "Microphone (On/Off)"
    );

    menu_append_utf8(
        middleClickMenu,
        MF_STRING |
        (context->state.middleClickAction == 1 ? MF_CHECKED : 0),
        ID_MOUSE_MIDDLE_PANEL,
        context->state.lang == LANG_RU
            ? "Панель аудиоустройств"
            : "Audio device panel"
    );

    menu_append_utf8(
        middleClickMenu,
        MF_STRING |
        (context->state.middleClickAction == 2 ? MF_CHECKED : 0),
        ID_MOUSE_MIDDLE_NONE,
        context->state.lang == LANG_RU
            ? "Нет действия"
            : "No action"
    );

    menu_append_utf8(
        trayIconMenu,
        MF_POPUP,
        (UINT_PTR)leftClickMenu,
        context->state.lang == LANG_RU
            ? "Левый клик"
            : "Left click"
    );

        menu_append_utf8(
        trayIconMenu,
        MF_POPUP | (isWindows11 ? MF_GRAYED : 0),
        (UINT_PTR)middleClickMenu,
        isWindows11
            ? (context->state.lang == LANG_RU
                  ? "Колесо = ЛКМ"
                  : "Middle = LMB")
            : (context->state.lang == LANG_RU
                  ? "Колёсико"
                  : "Middle")
    );

    menu_append_utf8(
        iconModeMenu,
        MF_STRING |
        (context->state.iconMode == 0 ? MF_CHECKED : 0),
        ID_SETTINGS_ICON_AUTO,
        context->state.lang == LANG_RU
            ? "Авто"
            : "Auto"
    );

    menu_append_utf8(
        iconModeMenu,
        MF_STRING |
        (context->state.iconMode == 1 ? MF_CHECKED : 0),
        ID_SETTINGS_ICON_BLACK,
        context->state.lang == LANG_RU
            ? "Белый"
            : "White"
    );

    menu_append_utf8(
        iconModeMenu,
        MF_STRING |
        (context->state.iconMode == 2 ? MF_CHECKED : 0),
        ID_SETTINGS_ICON_WHITE,
        context->state.lang == LANG_RU
            ? "Тёмный"
            : "Black"
    );

    menu_append_utf8(
        trayIconMenu,
        MF_POPUP,
        (UINT_PTR)iconModeMenu,
        context->state.lang == LANG_RU
            ? "Цвет значка"
            : "App icon color"
    );

    menu_append_utf8(
        settingsMenu,
        MF_POPUP,
        (UINT_PTR)trayIconMenu,
        context->state.lang == LANG_RU
            ? "Настройки значка"
            : "Icon settings"
    );
}

menu_append_utf8(
    settingsMenu,
    MF_STRING | (context->state.useAudioIcon ? MF_CHECKED : 0),
    ID_SETTINGS_AUDIO_ICON,
    localization_get(context->state.lang, "MENU_AUDIO_ICON")
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

AppendMenuW(mainMenu, MF_SEPARATOR, 0, NULL);

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