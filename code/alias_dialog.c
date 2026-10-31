#include "alias_dialog.h"
#include "device_icon.h"
#include "audio_device_type.h"
#include "alias.h"
#include "switch_devices.h"
#include "resource.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

static SwitchDeviceList g_outDevices;
static SwitchDeviceList g_inDevices;

#define ALIAS_FLAG_OUT 0x10000
#define ALIAS_FLAG_IN  0x20000

static void fill_combo(HWND hCombo) {
    int i, index;
    const wchar_t *alias;
    wchar_t label[300];

    ComboBox_ResetContent(hCombo);

    for (i = 0; i < g_outDevices.count; i++) {
        alias = alias_get(g_outDevices.devices[i].id);
        _snwprintf(label, 300, L"[вых] %s",
                   alias ? alias : g_outDevices.devices[i].name);
        label[299] = 0;
        index = (int)SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)label);
        SendMessageW(hCombo, CB_SETITEMDATA, index, ALIAS_FLAG_OUT | i);
    }

    for (i = 0; i < g_inDevices.count; i++) {
        alias = alias_get(g_inDevices.devices[i].id);
        _snwprintf(label, 300, L"[вх] %s",
                   alias ? alias : g_inDevices.devices[i].name);
        label[299] = 0;
        index = (int)SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)label);
        SendMessageW(hCombo, CB_SETITEMDATA, index, ALIAS_FLAG_IN | i);
    }

    if (g_outDevices.count + g_inDevices.count > 0) {
        SendMessageW(hCombo, CB_SETCURSEL, 0, 0);
    }
}

static const wchar_t *current_id(HWND hCombo, int *isInput) {
    int index, data;

    index = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
    if (index == CB_ERR) return NULL;

    data = (int)SendMessageW(hCombo, CB_GETITEMDATA, index, 0);

    if (data & ALIAS_FLAG_IN) {
        *isInput = 1;
        return g_inDevices.devices[data & 0xFFFF].id;
    }
    *isInput = 0;
    return g_outDevices.devices[data & 0xFFFF].id;
}

static void load_current(HWND hDlg) {
    HWND hCombo = GetDlgItem(hDlg, IDC_ALIAS_COMBO);
    HWND hEdit = GetDlgItem(hDlg, IDC_ALIAS_EDIT);
    const wchar_t *id, *alias;
    int isInput = 0;

    id = current_id(hCombo, &isInput);
    if (!id) return;

    alias = alias_get(id);
    SetWindowTextW(hEdit, alias ? alias : L"");
}

static INT_PTR CALLBACK proc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    (void)lp;
    HWND hCombo, hEdit;
    const wchar_t *id;
    wchar_t name[256];
    int isInput = 0;

    switch (msg) {
        case WM_INITDIALOG:
            SendMessageW(
                hDlg,
                WM_SETICON,
                ICON_SMALL,
                (LPARAM)LoadIconW(
                    GetModuleHandleW(NULL),
                    MAKEINTRESOURCEW(IDI_APP_ICON)
                )
            );

            if (switch_devices_get_output(&g_outDevices) &&
                switch_devices_get_input(&g_inDevices)) {
                fill_combo(GetDlgItem(hDlg, IDC_ALIAS_COMBO));
                load_current(hDlg);
            }
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDC_ALIAS_COMBO:
                    if (HIWORD(wp) == CBN_SELCHANGE)
                        load_current(hDlg);
                    return TRUE;

                case IDC_ALIAS_SAVE:
                    hCombo = GetDlgItem(hDlg, IDC_ALIAS_COMBO);
                    hEdit = GetDlgItem(hDlg, IDC_ALIAS_EDIT);
                    id = current_id(hCombo, &isInput);
                    if (!id) return TRUE;

                    GetWindowTextW(hEdit, name, 256);
                    if (name[0] == 0) {
                        alias_remove(id);
                    } else {
                        alias_set(id, name);
                    }

                    fill_combo(hCombo);
                    return TRUE;

                case IDC_ALIAS_RESET:
                    hCombo = GetDlgItem(hDlg, IDC_ALIAS_COMBO);
                    id = current_id(hCombo, &isInput);
                    if (!id) return TRUE;

                    alias_remove(id);
                    load_current(hDlg);
                    return TRUE;
                
				case IDC_ABOUT_CLOSE:
                    EndDialog(hDlg, 0);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    return TRUE;
				
            }
            break;
		        case WM_CLOSE:
            EndDialog(hDlg, 0);
            return TRUE;	
		
    }

    return FALSE;
}

void alias_dialog_show(HWND parent) {
    DialogBoxParamW(
        GetModuleHandleW(NULL),
        MAKEINTRESOURCEW(IDD_ALIAS),
        parent,
        proc,
        0
    );
}