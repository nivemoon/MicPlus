#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include "micplus_ini.h"

static char g_ini_path[MAX_PATH];
static char g_exe_ini[MAX_PATH];
static char g_appdata_ini[MAX_PATH];
static char g_exe_alias[MAX_PATH];
static char g_appdata_alias[MAX_PATH];
static int g_ini_deleted = 0;

static void BuildExeIniPath(void) {
    DWORD n = GetModuleFileNameA(NULL, g_exe_ini, MAX_PATH);
    while (n > 0 && g_exe_ini[n-1] != '\\') n--;
    g_exe_ini[n] = '\0';
    lstrcatA(g_exe_ini, "\\" INI_NAME);
}

static void BuildAppDataIniPath(void) {
    char appdata[MAX_PATH];
    g_appdata_ini[0] = '\0';
    if (FAILED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata)))
        return;
    lstrcatA(appdata, "\\MicPlus");
    CreateDirectoryA(appdata, NULL);          /* ок, если уже есть */
    lstrcatA(appdata, "\\" INI_NAME);
    lstrcpyA(g_appdata_ini, appdata);
}

static BOOL FileExists(const char *p) {
    return p[0] && GetFileAttributesA(p) != INVALID_FILE_ATTRIBUTES;
}

static void BuildExeAliasPath(void) {
    DWORD n = GetModuleFileNameA(NULL, g_exe_alias, MAX_PATH);
    while (n > 0 && g_exe_alias[n-1] != '\\') n--;
    g_exe_alias[n] = '\0';
    lstrcatA(g_exe_alias, "\\aliases.ini");
}

static void BuildAppDataAliasPath(void) {
    char appdata[MAX_PATH];
    g_appdata_alias[0] = '\0';
    if (FAILED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata)))
        return;
    lstrcatA(appdata, "\\MicPlus");
    CreateDirectoryA(appdata, NULL);
    lstrcatA(appdata, "\\aliases.ini");
    lstrcpyA(g_appdata_alias, appdata);
}

void IniInit(void) {
    BuildExeIniPath();
    BuildAppDataIniPath();
	BuildExeAliasPath();
    BuildAppDataAliasPath();

    if (FileExists(g_exe_ini)) {
        lstrcpyA(g_ini_path, g_exe_ini);
    } else if (FileExists(g_appdata_ini)) {
        lstrcpyA(g_ini_path, g_appdata_ini);
    } else {
        /* пробуем создать рядом с exe */
        HANDLE h = CreateFileA(g_exe_ini, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            CloseHandle(h);
            lstrcpyA(g_ini_path, g_exe_ini);
        } else {
            lstrcpyA(g_ini_path, g_appdata_ini);
        }
    }
}

const char* IniPath(void) {
    return g_ini_path;
}

int IniGetInt(const char *section, const char *key, int def) {
    return GetPrivateProfileIntA(section, key, def, g_ini_path);
}

void IniSetInt(const char *section, const char *key, int value) {
    char buf[32];
    wsprintfA(buf, "%d", value);
    WritePrivateProfileStringA(section, key, buf, g_ini_path);
}

void IniGetStr(const char *section, const char *key,
               const char *def, char *buf, int buf_size) {
    GetPrivateProfileStringA(section, key, def, buf, buf_size, g_ini_path);
}

void IniSetStr(const char *section, const char *key, const char *value) {
    WritePrivateProfileStringA(section, key, value, g_ini_path);
}

void IniDeleteEverywhere(void) {
    DeleteFileA(g_exe_ini);
    if (g_appdata_ini[0])
        DeleteFileA(g_appdata_ini);

    DeleteFileA(g_exe_alias);
    if (g_appdata_alias[0])
        DeleteFileA(g_appdata_alias);

    g_ini_deleted = 1;
}

void IniMarkDeleted(void) {
    g_ini_deleted = 1;
}

int IniIsDeleted(void) {
    return g_ini_deleted;
}