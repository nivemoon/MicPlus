#define _CRT_SECURE_NO_WARNINGS

#include "multimedia.h"
#include "switch_devices.h"

#include <stdio.h>
#include <dwmapi.h>

#define IDC_MM_OUTPUT 2001
#define IDC_MM_INPUT  2002

static const wchar_t *MULTIMEDIA_CLASS = L"MicPlusMultimediaWnd";

/* Цвета тёмной темы */
static const COLORREF MM_COLOR_BG = RGB(43, 43, 43);
static const COLORREF MM_COLOR_LIST_BG = RGB(55, 55, 55);
static const COLORREF MM_COLOR_TEXT = RGB(230, 230, 230);

static HBRUSH g_bgBrush = NULL;
static HBRUSH g_listBrush = NULL;

static HWND g_outputList = NULL;
static HWND g_inputList = NULL;

static void mm_log(const char *what, unsigned long code)
{
    FILE *f;
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char *slash = strrchr(path, '\\');

    if (slash) {
        *slash = '\0';
    }

    lstrcatA(path, "\\micplus_mm.log");

    f = fopen(path, "a");
    if (f) {
        fprintf(f, "%s: 0x%08lX\n", what, code);
        fclose(f);
    }
}

static void multimedia_fill_list(
    HWND list,
    SwitchDeviceList *devices)
{
    int i;

    SendMessageW(list, LB_RESETCONTENT, 0, 0);

    for (i = 0; i < devices->count; i++) {
        wchar_t text[SWITCH_DEVICES_NAME_LEN + 4];

        if (devices->devices[i].isDefault) {
            _snwprintf(text, ARRAYSIZE(text), L"\x25CF  %s",
                devices->devices[i].name);
        } else {
            _snwprintf(text, ARRAYSIZE(text), L"     %s",
                devices->devices[i].name);
        }

        text[ARRAYSIZE(text) - 1] = L'\0';

        SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)text);
    }
}

static void multimedia_refresh(void)
{
    SwitchDeviceList devices;

    if (g_outputList) {
        if (switch_devices_get_output(&devices)) {
            multimedia_fill_list(g_outputList, &devices);
        }
    }

    if (g_inputList) {
        if (switch_devices_get_input(&devices)) {
            multimedia_fill_list(g_inputList, &devices);
        }
    }
}

static void multimedia_create_children(HWND hWnd)
{
    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HWND label;

    label = CreateWindowExW(0, L"STATIC", L"Вывод",
        WS_CHILD | WS_VISIBLE,
        14, 10, 200, 16,
        hWnd, NULL, GetModuleHandleA(NULL), NULL);
    SendMessageW(label, WM_SETFONT, (WPARAM)font, TRUE);

    g_outputList = CreateWindowExW(
        0, L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        14, 28, 232, 64,
        hWnd, (HMENU)IDC_MM_OUTPUT,
        GetModuleHandleA(NULL), NULL);
    SendMessageW(g_outputList, WM_SETFONT, (WPARAM)font, TRUE);

    CreateWindowExW(0, L"STATIC", NULL,
        WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        14, 98, 232, 2,
        hWnd, NULL, GetModuleHandleA(NULL), NULL);

    label = CreateWindowExW(0, L"STATIC", L"Ввод",
        WS_CHILD | WS_VISIBLE,
        14, 106, 200, 16,
        hWnd, NULL, GetModuleHandleA(NULL), NULL);
    SendMessageW(label, WM_SETFONT, (WPARAM)font, TRUE);

    g_inputList = CreateWindowExW(
        0, L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        14, 124, 232, 64,
        hWnd, (HMENU)IDC_MM_INPUT,
        GetModuleHandleA(NULL), NULL);
    SendMessageW(g_inputList, WM_SETFONT, (WPARAM)font, TRUE);
}

static LRESULT CALLBACK multimedia_proc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {
        case WM_CREATE:
            if (!g_bgBrush) {
                g_bgBrush = CreateSolidBrush(MM_COLOR_BG);
                g_listBrush = CreateSolidBrush(MM_COLOR_LIST_BG);
            }

            multimedia_create_children(hWnd);
            multimedia_refresh();
            return 0;

        case WM_CTLCOLORSTATIC:
            SetTextColor((HDC)wParam, MM_COLOR_TEXT);
            SetBkColor((HDC)wParam, MM_COLOR_BG);
            return (LRESULT)g_bgBrush;

        case WM_CTLCOLORLISTBOX:
            SetTextColor((HDC)wParam, MM_COLOR_TEXT);
            SetBkColor((HDC)wParam, MM_COLOR_LIST_BG);
            return (LRESULT)g_listBrush;

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            int notification = HIWORD(wParam);
            int index;
            SwitchDeviceList devices;

            if (notification != LBN_SELCHANGE) {
                break;
            }

            if (id == IDC_MM_OUTPUT && g_outputList) {
                index = (int)SendMessageW(g_outputList, LB_GETCURSEL, 0, 0);

                if (index >= 0 &&
                    switch_devices_get_output(&devices) &&
                    index < devices.count) {

                    if (!switch_devices_set_default(
                            devices.devices[index].id)) {
                        mm_log("switch_devices_set_default",
                            (unsigned long)GetLastError());
                    }

                    multimedia_refresh();
                }
                return 0;
            }

            if (id == IDC_MM_INPUT && g_inputList) {
                index = (int)SendMessageW(g_inputList, LB_GETCURSEL, 0, 0);

                if (index >= 0 &&
                    switch_devices_get_input(&devices) &&
                    index < devices.count) {

                    if (!switch_devices_set_default_input(
                            devices.devices[index].id)) {
                        mm_log("switch_devices_set_default_input",
                            (unsigned long)GetLastError());
                    }

                    multimedia_refresh();
                }
                return 0;
            }

            break;
        }

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                ShowWindow(hWnd, SW_HIDE);
            }
            return 0;

         case WM_DESTROY:
            g_outputList = NULL;
            g_inputList = NULL;
            return 0;

        default:
            return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

void multimedia_window_show(HWND owner)
{
    static BOOL registered = FALSE;
    static HWND window = NULL;

    WNDCLASSW wc;
    RECT workArea;
    POINT cursor;
    HMONITOR monitor;
    MONITORINFO mi;
    int x, y;
    int width = 260;
    int height = 224;

    if (!registered) {
        ZeroMemory(&wc, sizeof(wc));

        wc.lpfnWndProc = multimedia_proc;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = MULTIMEDIA_CLASS;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(MM_COLOR_BG);

        if (!RegisterClassW(&wc)) {
            mm_log("RegisterClassW", (unsigned long)GetLastError());
            return;
        }

        registered = TRUE;
    }

    if (!window || !IsWindow(window)) {
        GetCursorPos(&cursor);

        monitor = MonitorFromPoint(cursor, MONITOR_DEFAULTTONEAREST);
        mi.cbSize = sizeof(mi);

        if (monitor && GetMonitorInfoW(monitor, &mi)) {
            workArea = mi.rcWork;
        } else {
            SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
        }

        x = cursor.x - width / 2;
        y = cursor.y - height - 8;

        if (x < workArea.left) {
            x = workArea.left;
        }
        if (x + width > workArea.right) {
            x = workArea.right - width;
        }
        if (y < workArea.top) {
            y = workArea.top;
        }
        if (y + height > workArea.bottom) {
            y = workArea.bottom - height;
        }

        window = CreateWindowExW(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            MULTIMEDIA_CLASS,
            L"Мультимедиа",
            WS_POPUP,
            x, y, width, height,
            owner,
            NULL,
            GetModuleHandleA(NULL),
            NULL
        );

        if (!window) {
            mm_log("CreateWindowExW", (unsigned long)GetLastError());
            return;
        }
    }

    if (IsWindowVisible(window)) {
        ShowWindow(window, SW_HIDE);
    } else {
        multimedia_refresh();
        SetForegroundWindow(window);
        ShowWindow(window, SW_SHOW);
    }
}