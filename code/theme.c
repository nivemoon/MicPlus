#include "theme.h"
#include <windows.h>
#include <winreg.h>

int theme_is_system_light(void)
{
    HKEY key;
    DWORD value = 1;
    DWORD valueSize = sizeof(value);

    if (RegOpenKeyExA(
            HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            0,
            KEY_READ,
            &key) != ERROR_SUCCESS) {
        return 1;
    }

    if (RegGetValueA(
            key,
            NULL,
            "SystemUsesLightTheme",
            RRF_RT_REG_DWORD,
            NULL,
            &value,
            &valueSize) != ERROR_SUCCESS) {
        RegCloseKey(key);
        return 1;
    }

    RegCloseKey(key);

    return value != 0;
}

void theme_update_icon(
    HICON *current,
    HICON iconLight,
    HICON iconDark,
    int iconMode)
{
    if (!current) {
        return;
    }

    switch (iconMode) {
        case 1:
            *current = iconDark;
            break;

        case 2:
            *current = iconLight;
            break;

        case 0:
        default:
            *current = theme_is_system_light()
                ? iconLight
                : iconDark;
            break;
    }
}

// НОВОЕ: определение цвета для меню
int theme_is_menu_dark(int menuTheme)
{
    // 0=система, 1=тёмная, 2=светлая
    switch (menuTheme) {
        case 1:
            return 1;  // тёмная
        case 2:
            return 0;  // светлая
        case 0:
        default:
            // система — инвертируем: если система светлая, меню светлое (0)
            // если система тёмная, меню тёмное (1)
            return !theme_is_system_light();
    }
}