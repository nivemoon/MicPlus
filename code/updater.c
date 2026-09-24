#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include "resource.h"
#include "unicode.h"

#pragma comment(lib, "winhttp")

#define APP_VERSION_MAJOR 1
#define APP_VERSION_MINOR 5
#define APP_VERSION_PATCH 0

static int parse_version_from_json(const wchar_t *json, int *major, int *minor, int *patch)
{
    const wchar_t *tag_name = wcsstr(json, L"\"tag_name\":");
    if (!tag_name) return 0;
    
    tag_name += 12;
    while (*tag_name == L' ' || *tag_name == L'"' || *tag_name == L'\t' || *tag_name == L'\n' || *tag_name == L'\r') tag_name++;
    
    const wchar_t *v = tag_name;
    
    if (*v == L'v' || *v == L'V') v++;
    
    if (swscanf_s(v, L"%d.%d.%d", major, minor, patch) != 3) {
        if (swscanf_s(v, L"%d.%d", major, minor) == 2) {
            *patch = 0;
            return 1;
        } else {
            return 0;
        }
    }
    
    return 1;
}

static int fetch_latest_version(int *major, int *minor, int *patch)
{
    HINTERNET session = NULL;
    HINTERNET connect = NULL;
    HINTERNET request = NULL;
    char utf8_buffer[16384] = {0};
    wchar_t utf16_buffer[16384] = {0};
    DWORD bytesRead = 0;
    int result = 0;
    DWORD totalBytes = 0;
    DWORD availableBytes = 0;
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    int utf16_len;
    
    session = WinHttpOpen(L"MicPlus/1.5", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!session) goto cleanup;
    
    DWORD timeout = 30000;
    WinHttpSetTimeouts(session, timeout, timeout, timeout, timeout);
    
    connect = WinHttpConnect(session, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connect) goto cleanup;
    
    request = WinHttpOpenRequest(connect, L"GET", L"/repos/nivemoon/micplus/releases/latest", NULL, NULL, NULL, WINHTTP_FLAG_SECURE);
    if (!request) goto cleanup;
    
    if (!WinHttpAddRequestHeaders(request, L"User-Agent: MicPlus/1.5", -1, WINHTTP_ADDREQ_FLAG_ADD)) goto cleanup;
    if (!WinHttpAddRequestHeaders(request, L"Accept: application/vnd.github+json", -1, WINHTTP_ADDREQ_FLAG_ADD)) goto cleanup;
    
    if (!WinHttpSendRequest(request, NULL, 0, NULL, 0, 0, 0)) goto cleanup;
    if (!WinHttpReceiveResponse(request, NULL)) goto cleanup;
    
    if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &statusCode, &statusCodeSize, NULL)) goto cleanup;
    
    if (statusCode != 200) goto cleanup;
    
    while (WinHttpQueryDataAvailable(request, &availableBytes) && availableBytes > 0) {
        if (!WinHttpReadData(request, utf8_buffer + totalBytes, availableBytes, &bytesRead)) {
            break;
        }
        totalBytes += bytesRead;
        if (totalBytes >= sizeof(utf8_buffer) - 1) break;
    }
    utf8_buffer[totalBytes] = '\0';
    
    utf16_len = MultiByteToWideChar(CP_UTF8, 0, utf8_buffer, -1, utf16_buffer, sizeof(utf16_buffer) / sizeof(wchar_t));
    if (utf16_len == 0) goto cleanup;
    
    if (parse_version_from_json(utf16_buffer, major, minor, patch)) {
        result = 1;
    }
    
cleanup:
    if (request) WinHttpCloseHandle(request);
    if (connect) WinHttpCloseHandle(connect);
    if (session) WinHttpCloseHandle(session);
    
    return result;
}

void updater_check_and_show(HWND hWnd)
{
    int latestMajor = 0, latestMinor = 0, latestPatch = 0;
    
    if (!fetch_latest_version(&latestMajor, &latestMinor, &latestPatch)) {
        MessageBoxW(hWnd, L"Не удалось проверить обновления.", L"MicPlus", MB_OK | MB_ICONWARNING);
        return;
    }
    
    if (latestMajor > APP_VERSION_MAJOR || 
        (latestMajor == APP_VERSION_MAJOR && latestMinor > APP_VERSION_MINOR) ||
        (latestMajor == APP_VERSION_MAJOR && latestMinor == APP_VERSION_MINOR && latestPatch > APP_VERSION_PATCH)) {
        
        wchar_t msg[256];
        swprintf_s(msg, 256, L"Доступна версия: %d.%d.%d. Скачать?", latestMajor, latestMinor, latestPatch);
        
        int ret = MessageBoxW(hWnd, msg, L"MicPlus", MB_YESNO | MB_ICONINFORMATION);
        if (ret == IDYES) {
            ShellExecuteW(hWnd, L"open", L"https://github.com/nivemoon/micplus/releases/latest", NULL, NULL, SW_SHOWNORMAL);
        }
    } else {
        MessageBoxW(hWnd, L"У вас последняя версия.", L"MicPlus", MB_OK | MB_ICONINFORMATION);
    }
}
