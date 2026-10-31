#define _CRT_SECURE_NO_WARNINGS

#include "microphone.h"

#include <shellapi.h>
#include <stdio.h>
#include <string.h>

static BOOL microphone_build_ctl_path(
    const char *exePath,
    char *outPath,
    int outSize)
{
    char baseDirectory[600];
    char *slash;

    if (!exePath ||
        !outPath ||
        outSize <= 0) {
        return FALSE;
    }

    outPath[0] = '\0';

    lstrcpynA(
        baseDirectory,
        exePath,
        sizeof(baseDirectory)
    );

    slash = strrchr(
        baseDirectory,
        '\\'
    );

    if (!slash) {
        return FALSE;
    }

    *(slash + 1) = '\0';

    snprintf(
        outPath,
        outSize,
        "%smicctl.exe",
        baseDirectory
    );

    return outPath[0] != '\0';
}

BOOL microphone_run_ctl(
    const char *exePath,
    int mute)
{
    char micCtlPath[1320];
    char commandLine[1340];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    if (!microphone_build_ctl_path(
            exePath,
            micCtlPath,
            sizeof(micCtlPath))) {
        return FALSE;
    }

    snprintf(
        commandLine,
        sizeof(commandLine),
        "\"%s\" %d",
        micCtlPath,
        mute ? 1 : 0
    );

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(
            NULL,
            commandLine,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi)) {
        return FALSE;
    }

    /* Не ждём завершения — процесс работает сам.
       Закрываем хендлы, чтобы не текли. */
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return TRUE;
}