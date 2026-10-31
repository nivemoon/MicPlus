#include "alias.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>

static char g_iniPathA[MAX_PATH];
static wchar_t g_iniPathW[MAX_PATH];

void alias_set_path(
    const char *exe_path,
    const char *appdata_path)
{
    char exeDir[MAX_PATH];
    char candidate[MAX_PATH + 32];
    FILE *test;
    char *lastSlash;

    g_iniPathA[0] = '\0';

    /* Попытка 1: рядом с exe */
    if (exe_path && exe_path[0]) {
        strncpy(exeDir, exe_path, MAX_PATH - 1);
        exeDir[MAX_PATH - 1] = '\0';

        lastSlash = strrchr(exeDir, '\\');
        if (lastSlash) {
            *lastSlash = '\0';

            snprintf(
                candidate,
                sizeof(candidate),
                "%s\\aliases.ini",
                exeDir
            );

            test = fopen(candidate, "a");
            if (test) {
                fclose(test);
                strncpy(g_iniPathA, candidate, MAX_PATH - 1);
                g_iniPathA[MAX_PATH - 1] = '\0';
            }
        }
    }

    /* Попытка 2: AppData\MicPlus */
    if (g_iniPathA[0] == '\0' &&
        appdata_path && appdata_path[0]) {

        CreateDirectoryA(appdata_path, NULL);

        snprintf(
            g_iniPathA,
            sizeof(g_iniPathA),
            "%s\\aliases.ini",
            appdata_path
        );
    }

    MultiByteToWideChar(
        CP_ACP,
        0,
        g_iniPathA,
        -1,
        g_iniPathW,
        MAX_PATH
    );
}

const wchar_t *alias_get(const wchar_t *device_id) {
    static wchar_t value[256];
    char idA[256];
    char valueA[512];

    WideCharToMultiByte(
        CP_ACP, 0, device_id, -1,
        idA, sizeof(idA), NULL, NULL
    );

    if (GetPrivateProfileStringA(
            "Aliases",
            idA,
            "",
            valueA,
            sizeof(valueA),
            g_iniPathA
        ) == 0) {
        return NULL;
    }

    MultiByteToWideChar(
        CP_ACP, 0, valueA, -1,
        value, 256
    );

    return value;
}

int alias_set(const wchar_t *device_id, const wchar_t *name) {
    char idA[256];
    char nameA[512];

    WideCharToMultiByte(
        CP_ACP, 0, device_id, -1,
        idA, sizeof(idA), NULL, NULL
    );

    WideCharToMultiByte(
        CP_ACP, 0, name, -1,
        nameA, sizeof(nameA), NULL, NULL
    );

    return WritePrivateProfileStringA(
        "Aliases",
        idA,
        nameA,
        g_iniPathA
    ) != 0;
}

int alias_remove(const wchar_t *device_id) {
    char idA[256];

    WideCharToMultiByte(
        CP_ACP, 0, device_id, -1,
        idA, sizeof(idA), NULL, NULL
    );

    return WritePrivateProfileStringA(
        "Aliases",
        idA,
        NULL,
        g_iniPathA
    ) != 0;
}