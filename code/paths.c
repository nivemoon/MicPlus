#define _CRT_SECURE_NO_WARNINGS

#include "paths.h"

#include <shlobj.h>
#include <stdio.h>
#include <string.h>

BOOL paths_get_config_file(
    char *outPath,
    size_t bufferSize)
{
    char appData[1024];     // строки 13, 88, 116
	char folderPath[1200];  // строка 14

    if (!outPath || bufferSize == 0) {
        return FALSE;
    }

    outPath[0] = '\0';

    if (SHGetFolderPathA(
            NULL,
            CSIDL_APPDATA,
            NULL,
            SHGFP_TYPE_CURRENT,
            appData) != S_OK) {
        return FALSE;
    }

    snprintf(
        folderPath,
        sizeof(folderPath),
        "%s\\MicPlus",
        appData
    );

    CreateDirectoryA(
        folderPath,
        NULL
    );

    snprintf(
        outPath,
        bufferSize,
        "%s\\micplus.ini",
        folderPath
    );

    return TRUE;
}

void paths_build_config_fallback(
    AppPaths *paths)
{
    char *lastSlash;

    if (!paths) {
        return;
    }

    lstrcpynA(
        paths->iniPath,
        paths->exePath,
        sizeof(paths->iniPath)
    );

    lastSlash = strrchr(
        paths->iniPath,
        '\\'
    );

    if (lastSlash) {
        *(lastSlash + 1) = '\0';

        lstrcatA(
            paths->iniPath,
            "micplus.ini"
        );
    } else {
        paths->iniPath[0] = '\0';
    }
}

void paths_build_startup(
    AppPaths *paths)
{
    char appData[1024];

    if (!paths) {
        return;
    }

    paths->startupPath[0] = '\0';

    if (SHGetFolderPathA(
            NULL,
            CSIDL_APPDATA,
            NULL,
            SHGFP_TYPE_CURRENT,
            appData) != S_OK) {
        return;
    }

    snprintf(
        paths->startupPath,
        sizeof(paths->startupPath),
        "%s\\Microsoft\\Windows\\Start Menu\\Programs\\Startup",
        appData
    );
}

void paths_build(
    AppPaths *paths)
{
    char appData[1024];
    
    if (!paths) {
        return;
    }

    ZeroMemory(
        paths,
        sizeof(*paths)
    );

    GetModuleFileNameA(
        NULL,
        paths->exePath,
        sizeof(paths->exePath)
    );

    // Получить AppData
    if (SHGetFolderPathA(
            NULL,
            CSIDL_APPDATA,
            NULL,
            SHGFP_TYPE_CURRENT,
            appData) == S_OK) {
        
        // %APPDATA%\MicPlus
        snprintf(
            paths->appDataPath,
            sizeof(paths->appDataPath),
            "%s\\MicPlus",
            appData
        );
        
        // Создать папку
        CreateDirectoryA(
            paths->appDataPath,
            NULL
        );
        
        // micctlPath = %APPDATA%\MicPlus\micctl.exe
        snprintf(
            paths->micctlPath,
            sizeof(paths->micctlPath),
            "%s\\micctl.exe",
            paths->appDataPath
        );
    }

    if (!paths_get_config_file(
            paths->iniPath,
            sizeof(paths->iniPath))) {
        paths_build_config_fallback(paths);
    }

    paths_build_startup(paths);
}


