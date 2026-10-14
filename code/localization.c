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
            return "Системные уведомления";

        if (lstrcmpA(id, "MENU_HIDE_TRAY") == 0)
            return "Скрыть значок из трея";

        if (lstrcmpA(id, "MENU_EXIT") == 0)
            return "Выход";

        if (lstrcmpA(id, "MENU_PTT_BUTTON") == 0)
            return "Кнопка Рации (РТТ)";

        if (lstrcmpA(id, "MENU_MIC_BUTTON") == 0)
            return "Кнопка микрофона";

        if (lstrcmpA(id, "MENU_PTT_CUSTOM") == 0)
            return "Кнопка РТТ: Назначить...";

        if (lstrcmpA(id, "MENU_MIC_CUSTOM") == 0)
            return "Микрофон: Назначить...";

        if (lstrcmpA(id, "MENU_PASSIVE_CUSTOM") == 0)
            return "Двойной клик: Назначить...";

        if (lstrcmpA(id, "MENU_ABOUT") == 0)
            return "О программе";

        if (lstrcmpA(id, "MENU_MOUSE4") == 0)
            return "Мышь 4";

        if (lstrcmpA(id, "MENU_MOUSE5") == 0)
            return "Мышь 5";

        if (lstrcmpA(id, "MENU_TILDE") == 0)
            return "Тильда (~)";

        if (lstrcmpA(id, "MENU_ICON_COLOR") == 0)
            return "Цвет значка MicPlus";

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
                   "(клавиша, две клавиши, модификаторы + клавиша).\n"
                   "ESC в основном окне отменяет выбор.";

        if (lstrcmpA(id, "MSG_SOUND_SELECT_TITLE") == 0)
            return "MicPlus - Хоткей звука системы";

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
            return "Не удалось зарегистрировать горячую клавишу.";

        if (lstrcmpA(id, "TOOLTIP_FORMAT_ON") == 0)
            return "MicPlus: PTT %s, Микрофон %s, Звук %s";

        if (lstrcmpA(id, "TOOLTIP_FORMAT_OFF") == 0)
            return "MicPlus: PTT %s, Микрофон %s, Звук %s";

        if (lstrcmpA(id, "BALLOON_MIC_ON_TITLE") == 0)
            return "MicPlus";

        if (lstrcmpA(id, "BALLOON_MIC_ON_TEXT") == 0)
            return "Микрофон включён";

        if (lstrcmpA(id, "BALLOON_MIC_OFF_TITLE") == 0)
            return "MicPlus";

        if (lstrcmpA(id, "BALLOON_MIC_OFF_TEXT") == 0)
            return "Микрофон выключен";

        if (lstrcmpA(id, "BALLOON_PTT_TITLE") == 0)
            return "MicPlus";

        if (lstrcmpA(id, "BALLOON_PTT_ON_TEXT") == 0)
            return "Режим РТТ включён";

        if (lstrcmpA(id, "BALLOON_PTT_OFF_TEXT") == 0)
            return "Режим РТТ выключен";

        if (lstrcmpA(id, "MENU_PTT_MODE_CUSTOM") == 0)
            return "Режим РТТ (Alt + P) | Ред.";

        if (lstrcmpA(id, "MSG_PTT_MODE_SELECT_TITLE") == 0)
            return "MicPlus - Сочетание режима РТТ";

        if (lstrcmpA(id, "MSG_PTT_MODE_SELECT_TEXT") == 0)
            return "Закройте это окно, затем нажмите нужное сочетание.\n"
                   "ESC отменяет выбор и оставляет текущее сочетание без изменений.";
    }

    if (language == LANG_EN) {
        if (lstrcmpA(id, "MENU_PTT_MODE") == 0)
            return "Enable Push-to-Talk";

        if (lstrcmpA(id, "MENU_MIC") == 0)
            return "Enable Microphone";

        if (lstrcmpA(id, "MENU_SOUND") == 0)
            return "System Sound";

        if (lstrcmpA(id, "MENU_HOTKEYS") == 0)
            return "Keyboard shortcuts";

        if (lstrcmpA(id, "MENU_SETTINGS") == 0)
            return "Settings";

        if (lstrcmpA(id, "MENU_LANGUAGE") == 0)
            return "Language";

        if (lstrcmpA(id, "MENU_AUTOSTART") == 0)
            return "Start with Windows";

        if (lstrcmpA(id, "MENU_MIC_SOUNDS") == 0)
            return "Microphone Sounds (On/Off)";

        if (lstrcmpA(id, "MENU_PTT_SOUNDS") == 0)
            return "Push-to-Talk Sounds";

        if (lstrcmpA(id, "MENU_BALLOON") == 0)
            return "Show Notifications";

        if (lstrcmpA(id, "MENU_HIDE_TRAY") == 0)
            return "Hide Tray Icon";

        if (lstrcmpA(id, "MENU_EXIT") == 0)
            return "Exit";

        if (lstrcmpA(id, "MENU_PTT_BUTTON") == 0)
            return "PTT button";

        if (lstrcmpA(id, "MENU_MIC_BUTTON") == 0)
            return "Microphone button";

        if (lstrcmpA(id, "MENU_PTT_CUSTOM") == 0)
            return "PTT button: Assign...";

        if (lstrcmpA(id, "MENU_MIC_CUSTOM") == 0)
            return "Microphone: Assign...";

        if (lstrcmpA(id, "MENU_PASSIVE_CUSTOM") == 0)
            return "Passive Push-to-Talk: Assign...";

        if (lstrcmpA(id, "MENU_ABOUT") == 0)
            return "About";

        if (lstrcmpA(id, "MENU_MOUSE4") == 0)
            return "Mouse 4";

        if (lstrcmpA(id, "MENU_MOUSE5") == 0)
            return "Mouse 5";

        if (lstrcmpA(id, "MENU_TILDE") == 0)
            return "Tilde (~)";

        if (lstrcmpA(id, "MENU_ICON_COLOR") == 0)
            return "App Icon Color";

        if (lstrcmpA(id, "ICON_AUTO") == 0)
            return "System";

        if (lstrcmpA(id, "ICON_BLACK") == 0)
            return "White";

        if (lstrcmpA(id, "ICON_WHITE") == 0)
            return "Black";

        if (lstrcmpA(id, "MSG_TOGGLE_SELECT_TITLE") == 0)
            return "MicPlus - Microphone Hotkey";

        if (lstrcmpA(id, "MSG_TOGGLE_SELECT_TEXT") == 0)
            return "Close this window, then press the combination you want "
                   "(a key, two keys, or modifiers plus a key).\n"
                   "Press ESC in the main window to cancel.";

        if (lstrcmpA(id, "MSG_SOUND_SELECT_TITLE") == 0)
            return "MicPlus - System Sound Hotkey";

        if (lstrcmpA(id, "MSG_PTT_SELECT_TITLE") == 0)
            return "MicPlus - Select Push-to-Talk Key";

        if (lstrcmpA(id, "MSG_PTT_SELECT_TEXT") == 0)
            return "Close this window, then press the Push-to-Talk key you want.\n"
                   "Press ESC in the main window to cancel.";

        if (lstrcmpA(id, "MSG_SELECTED_HK") == 0)
            return "Hotkey selected: %s";

        if (lstrcmpA(id, "MSG_SELECTED_PTT") == 0)
            return "Push-to-Talk key selected: %s";

        if (lstrcmpA(id, "MSG_ERR_TOGGLE_ENTER_ESC") == 0)
            return "Enter or Esc cannot be used on their own.\n"
                   "Add Ctrl, Alt, Shift, or Win, or choose another key.";

        if (lstrcmpA(id, "MSG_ERR_PTT_ENTER_ESC") == 0)
            return "Enter or Esc cannot be used for Push-to-Talk.\n"
                   "Choose another key.";

        if (lstrcmpA(id, "MSG_ERR_HOOK") == 0)
            return "Failed to set low-level hooks.";

        if (lstrcmpA(id, "MSG_ERR_HOTKEY") == 0)
            return "Failed to register hotkey.";

        if (lstrcmpA(id, "TOOLTIP_FORMAT_ON") == 0)
            return "MicPlus: PTT %s, Microphone %s, Sound %s";

        if (lstrcmpA(id, "TOOLTIP_FORMAT_OFF") == 0)
            return "MicPlus: PTT %s, Microphone %s, Sound %s";

        if (lstrcmpA(id, "BALLOON_MIC_ON_TITLE") == 0)
            return "MicPlus";

        if (lstrcmpA(id, "BALLOON_MIC_ON_TEXT") == 0)
            return "Microphone on";

        if (lstrcmpA(id, "BALLOON_MIC_OFF_TITLE") == 0)
            return "MicPlus";

        if (lstrcmpA(id, "BALLOON_MIC_OFF_TEXT") == 0)
            return "Microphone off";

        if (lstrcmpA(id, "BALLOON_PTT_TITLE") == 0)
            return "MicPlus";

        if (lstrcmpA(id, "BALLOON_PTT_ON_TEXT") == 0)
            return "PTT mode on";

        if (lstrcmpA(id, "BALLOON_PTT_OFF_TEXT") == 0)
            return "PTT mode off";

        if (lstrcmpA(id, "MENU_PTT_MODE_CUSTOM") == 0)
            return "PTT mode (Alt + P) | Edit";

        if (lstrcmpA(id, "MSG_PTT_MODE_SELECT_TITLE") == 0)
            return "MicPlus - PTT Mode Hotkey";

        if (lstrcmpA(id, "MSG_PTT_MODE_SELECT_TEXT") == 0)
            return "Close this window, then press the desired hotkey.\n"
                   "ESC cancels and keeps the current hotkey unchanged.";
    }

    return id;
}