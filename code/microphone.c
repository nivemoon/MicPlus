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
    char baseDirectory[MAX_PATH];
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
    char micCtlPath[MAX_PATH];
    char arguments[8];
    HINSTANCE result;

    if (!microphone_build_ctl_path(
            exePath,
            micCtlPath,
            sizeof(micCtlPath))) {
        return FALSE;
    }

    snprintf(
        arguments,
        sizeof(arguments),
        "%d",
        mute ? 1 : 0
    );

    result = ShellExecuteA(
        NULL,
        "open",
        micCtlPath,
        arguments,
        NULL,
        SW_HIDE
    );

    return (INT_PTR)result > 32;
}