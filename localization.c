#include "localization.h"

#include <windows.h>

const char *localization_get(
    APP_LANG language,
    const char *id)
{
    if (!id) {
        return "";
    }

    if (language == LANG_RU) {
        if (lstrcmpA(id, "MENU_PTT_MODE") == 0)
            return "Включить РТТ";

        if (lstrcmpA(id, "MENU_MIC") == 0)
            return "Включить микрофон";

        if (lstrcmpA(id, "MENU_SOUND") == 0)
            return "Глобальный звук";

        if (lstrcmpA(id, "MENU_HOTKEYS") == 0)
            return "Сочетания клавиш";

        if (lstrcmpA(id, "MENU_SETTINGS") == 0)
            return "Настройки";

        if (lstrcmpA(id, "MENU_LANGUAGE") == 0)
            return "Язык программы";

        if (lstrcmpA(id, "MENU_AUTOSTART") == 0)
            return "Автозапуск с Windows";

        if (lstrcmpA(id, "MENU_MIC_SOUNDS") == 0)
            return "Сигналы микрофона (ВКЛ/ВЫКЛ)";

        if (lstrcmpA(id, "MENU_PTT_SOUNDS") == 0)
            return "Сигналы в режиме рации (РТТ)";

        if (lstrcmpA(id, "MENU_BALLOON") == 0)
            return "Показывать уведомления";

        if (lstrcmpA(id, "MENU_HIDE_TRAY") == 0)
            return "Скрыть значок из трея";

        if (lstrcmpA(id, "MENU_EXIT") == 0)
            return "Выход";

        if (lstrcmpA(id, "MENU_PTT_BUTTON") == 0)
            return "Кнопка режима РТТ";

        if (lstrcmpA(id, "MENU_MIC_BUTTON") == 0)
            return "Кнопка микрофона";

        if (lstrcmpA(id, "MENU_PTT_CUSTOM") == 0)
            return "Режим РТТ: Назначить...";

        if (lstrcmpA(id, "MENU_MIC_CUSTOM") == 0)
            return "Микрофон: Назначить...";

        if (lstrcmpA(id, "MENU_ABOUT") == 0)
            return "О программе";

        if (lstrcmpA(id, "MENU_MOUSE4") == 0)
            return "Мышь 4";

        if (lstrcmpA(id, "MENU_MOUSE5") == 0)
            return "Мышь 5";

        if (lstrcmpA(id, "MENU_TILDE") == 0)
            return "Тильда (~)";

        if (lstrcmpA(id, "MENU_ICON_COLOR") == 0)
            return "Цвет иконки приложения";

        if (lstrcmpA(id, "ICON_AUTO") == 0)
            return "Система";

        if (lstrcmpA(id, "ICON_BLACK") == 0)
            return "Белый";

        if (lstrcmpA(id, "ICON_WHITE") == 0)
            return "Чёрный";

        if (lstrcmpA(id, "MSG_TOGGLE_SELECT_TITLE") == 0)
            return "MicPlus - Хоткей микрофона";

        if (lstrcmpA(id, "MSG_TOGGLE_SELECT_TEXT") == 0)
            return "Закройте это окно, затем нажмите нужное сочетание "
                   "(модификаторы + клавиша).\n"
                   "ESC в основном окне отменяет выбор.";

        if (lstrcmpA(id, "MSG_SOUND_SELECT_TITLE") == 0)
            return "MicPlus - Хоткей звука системы";

        if (lstrcmpA(id, "MSG_SOUND_SELECT_TEXT") == 0)
            return "Закройте это окно, затем нажмите нужное сочетание "
                   "(модификаторы + клавиша) для звука.\n"
                   "ESC в основном окне отменяет выбор.";

        if (lstrcmpA(id, "MSG_PTT_SELECT_TITLE") == 0)
            return "MicPlus - Выбор PTT-клавиши";

        if (lstrcmpA(id, "MSG_PTT_SELECT_TEXT") == 0)
            return "Закройте это окно, затем нажмите нужную PTT-клавишу.\n"
                   "ESC в основном окне отменяет выбор.";

        if (lstrcmpA(id, "MSG_SELECTED_HK") == 0)
            return "Выбран хоткей: %s";

        if (lstrcmpA(id, "MSG_SELECTED_PTT") == 0)
            return "Выбрана PTT-клавиша: %s";

        if (lstrcmpA(id, "MSG_ERR_TOGGLE_ENTER_ESC") == 0)
            return "Enter/Esc нельзя использовать в одиночку.\n"
                   "Добавьте Ctrl/Alt/Shift/Win или выберите другую клавишу.";

        if (lstrcmpA(id, "MSG_ERR_PTT_ENTER_ESC") == 0)
            return "Enter/Esc нельзя использовать для PTT.\n"
                   "Выберите другую клавишу.";

        if (lstrcmpA(id, "MSG_ERR_HOOK") == 0)
            return "Не удалось установить низкоуровневые хуки.";

        if (lstrcmpA(id, "MSG_ERR_HOTKEY") == 0)
            return "Не удалось зарегистрировать хоткей.";

        if (lstrcmpA(id, "TOOLTIP_FORMAT_ON") == 0)
            return "MicPlus: ВКЛ (РТТ %s, Мик %s, Звук %s)";

        if (lstrcmpA(id, "TOOLTIP_FORMAT_OFF") == 0)
            return "MicPlus: ВЫКЛ (РТТ %s, Мик %s, Звук %s)";

        if (lstrcmpA(id, "BALLOON_MIC_ON_TITLE") == 0)
            return "MicPlus";

        if (lstrcmpA(id, "BALLOON_MIC_ON_TEXT") == 0)
            return "Микрофон ВКЛ";

        if (lstrcmpA(id, "BALLOON_MIC_OFF_TITLE") == 0)
            return "MicPlus";

        if (lstrcmpA(id, "BALLOON_MIC_OFF_TEXT") == 0)
            return "Микрофон ВЫКЛ";

    } else {
        if (lstrcmpA(id, "MENU_PTT_MODE") == 0)
            return "PTT mode";

        if (lstrcmpA(id, "MENU_MIC") == 0)
            return "Microphone";

        if (lstrcmpA(id, "MENU_SOUND") == 0)
            return "Global sound";

        if (lstrcmpA(id, "MENU_HOTKEYS") == 0)
            return "Hotkeys";

        if (lstrcmpA(id, "MENU_SETTINGS") == 0)
            return "Settings";

        if (lstrcmpA(id, "MENU_LANGUAGE") == 0)
            return "Language";

        if (lstrcmpA(id, "MENU_AUTOSTART") == 0)
            return "Start MicPlus with Windows";

        if (lstrcmpA(id, "MENU_MIC_SOUNDS") == 0)
            return "Microphone sounds (mute/unmute)";

        if (lstrcmpA(id, "MENU_PTT_SOUNDS") == 0)
            return "PTT sounds (push-to-talk)";

        if (lstrcmpA(id, "MENU_BALLOON") == 0)
            return "Microphone notifications (balloon)";

        if (lstrcmpA(id, "MENU_HIDE_TRAY") == 0)
            return "Hide tray icon";

        if (lstrcmpA(id, "MENU_EXIT") == 0)
            return "Exit";

        if (lstrcmpA(id, "MENU_PTT_BUTTON") == 0)
            return "PTT Button";

        if (lstrcmpA(id, "MENU_MIC_BUTTON") == 0)
            return "Mic Button";

        if (lstrcmpA(id, "MENU_PTT_CUSTOM") == 0)
            return "PTT: Custom...";

        if (lstrcmpA(id, "MENU_MIC_CUSTOM") == 0)
            return "Mic: Custom...";

        if (lstrcmpA(id, "MENU_ABOUT") == 0)
            return "About";

        if (lstrcmpA(id, "MENU_MOUSE4") == 0)
            return "Mouse 4";

        if (lstrcmpA(id, "MENU_MOUSE5") == 0)
            return "Mouse 5";

        if (lstrcmpA(id, "MENU_TILDE") == 0)
            return "Tilde (~)";

        if (lstrcmpA(id, "MENU_ICON_COLOR") == 0)
            return "Icon color";

        if (lstrcmpA(id, "ICON_AUTO") == 0)
            return "Auto";

        if (lstrcmpA(id, "ICON_BLACK") == 0)
            return "Black";

        if (lstrcmpA(id, "ICON_WHITE") == 0)
            return "White";

        if (lstrcmpA(id, "MENU_THEME") == 0)
            return "Menu theme";

        if (lstrcmpA(id, "THEME_DARK") == 0)
            return "Dark";

        if (lstrcmpA(id, "THEME_LIGHT") == 0)
            return "Light";

        if (lstrcmpA(id, "MSG_TOGGLE_SELECT_TITLE") == 0)
            return "MicPlus - Mic hotkey";

        if (lstrcmpA(id, "MSG_TOGGLE_SELECT_TEXT") == 0)
            return "Close this window, then press modifiers+key "
                   "in the main window.\n"
                   "Press Esc in the main window to cancel.";

        if (lstrcmpA(id, "MSG_SOUND_SELECT_TITLE") == 0)
            return "MicPlus - Global sound hotkey";

        if (lstrcmpA(id, "MSG_SOUND_SELECT_TEXT") == 0)
            return "Close this window, then press modifiers+key "
                   "for sound.\n"
                   "Press Esc in the main window to cancel.";

        if (lstrcmpA(id, "MSG_PTT_SELECT_TITLE") == 0)
            return "MicPlus - Select PTT key";

        if (lstrcmpA(id, "MSG_PTT_SELECT_TEXT") == 0)
            return "Close this window, then press the desired PTT key.\n"
                   "Press Esc in the main window to cancel.";

        if (lstrcmpA(id, "MSG_SELECTED_HK") == 0)
            return "Selected hotkey: %s";

        if (lstrcmpA(id, "MSG_SELECTED_PTT") == 0)
            return "Selected PTT key: %s";

        if (lstrcmpA(id, "MSG_ERR_TOGGLE_ENTER_ESC") == 0)
            return "Enter/Esc cannot be used alone.\n"
                   "Please include Ctrl/Alt/Shift/Win or pick another key.";

        if (lstrcmpA(id, "MSG_ERR_PTT_ENTER_ESC") == 0)
            return "Enter/Esc cannot be used for PTT.\n"
                   "Pick another key.";

        if (lstrcmpA(id, "MSG_ERR_HOOK") == 0)
            return "Failed to set low-level hooks.";

        if (lstrcmpA(id, "MSG_ERR_HOTKEY") == 0)
            return "Failed to register hotkey.";

        if (lstrcmpA(id, "TOOLTIP_FORMAT_ON") == 0)
            return "MicPlus: ON (PTT %s, Mic %s, Sound %s)";

        if (lstrcmpA(id, "TOOLTIP_FORMAT_OFF") == 0)
            return "MicPlus: OFF (PTT %s, Mic %s, Sound %s)";

        if (lstrcmpA(id, "BALLOON_MIC_ON_TITLE") == 0)
            return "MicPlus";

        if (lstrcmpA(id, "BALLOON_MIC_ON_TEXT") == 0)
            return "Microphone ON";

        if (lstrcmpA(id, "BALLOON_MIC_OFF_TITLE") == 0)
            return "MicPlus";

        if (lstrcmpA(id, "BALLOON_MIC_OFF_TEXT") == 0)
            return "Microphone OFF";
    }

    return id;
}