#define _CRT_SECURE_NO_WARNINGS

#include "multimedia.h"
#include "switch_devices.h"
#include "alias.h"

#include <stdio.h>

#define MM_OUTPUT_BASE  9200
#define MM_INPUT_BASE   9300

static void multimedia_add_devices(
    HMENU menu,
    SwitchDeviceList *devices,
    int idBase)
{
    int i;
    wchar_t text[SWITCH_DEVICES_NAME_LEN + 4];
    const wchar_t *alias;
    const wchar_t *label;

    for (i = 0; i < devices->count; i++) {
        UINT flags = MF_STRING;

        if (devices->devices[i].isDefault) {
            flags |= MF_CHECKED;
        }

        alias = alias_get(devices->devices[i].id);
        label = alias ? alias : devices->devices[i].name;

        _snwprintf(text, ARRAYSIZE(text), L"%s", label);
        text[ARRAYSIZE(text) - 1] = L'\0';

        AppendMenuW(menu, flags, idBase + i, text);
    }
}

void multimedia_show(HWND owner, int hideMicSelection)
{
    HMENU menu = CreatePopupMenu();
    SwitchDeviceList devices;
    POINT pt;
    UINT choice;

    if (!menu) {
        return;
    }

    if (switch_devices_get_output(&devices)) {
        multimedia_add_devices(menu, &devices, MM_OUTPUT_BASE);
    }

    if (!hideMicSelection &&
        switch_devices_get_input(&devices) &&
        devices.count > 0) {

        AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
        multimedia_add_devices(menu, &devices, MM_INPUT_BASE);
    }

    GetCursorPos(&pt);

    SetForegroundWindow(owner);

    choice = TrackPopupMenu(
        menu,
        TPM_RIGHTBUTTON | TPM_RETURNCMD,
        pt.x, pt.y,
        0, owner, NULL
    );

    DestroyMenu(menu);

    if (choice >= MM_OUTPUT_BASE &&
        choice < MM_OUTPUT_BASE + SWITCH_DEVICES_MAX) {

        if (switch_devices_get_output(&devices)) {
            int index = choice - MM_OUTPUT_BASE;

            if (index < devices.count) {
                switch_devices_set_default(
                    devices.devices[index].id
                );
            }
        }

        return;
    }

    if (choice >= MM_INPUT_BASE &&
        choice < MM_INPUT_BASE + SWITCH_DEVICES_MAX) {

        if (switch_devices_get_input(&devices)) {
            int index = choice - MM_INPUT_BASE;

            if (index < devices.count) {
                switch_devices_set_default_input(
                    devices.devices[index].id
                );
            }
        }
    }
}
