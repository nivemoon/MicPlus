#define _CRT_SECURE_NO_WARNINGS

#include "sounds.h"

#include <mmsystem.h>
#include <stdio.h>
#include <string.h>

void sounds_build_path(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    char *outPath,
    int outSize)
{
    char baseDirectory[MAX_PATH];
    const char *slash;

    if (!outPath || outSize <= 0) {
        return;
    }

    outPath[0] = '\0';

    if (!exePath ||
        !soundsDirectory ||
        !fileName ||
        !fileName[0]) {
        return;
    }

    lstrcpynA(
        baseDirectory,
        exePath,
        sizeof(baseDirectory)
    );

    slash = strrchr(
        baseDirectory,
        '\\'
    );

    if (slash) {
        baseDirectory[slash - baseDirectory + 1] = '\0';
    } else {
        baseDirectory[0] = '\0';
    }

    snprintf(
        outPath,
        outSize,
        "%s%s\\%s",
        baseDirectory,
        soundsDirectory,
        fileName
    );
}

BOOL sounds_play_file(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled)
{
    char fullPath[MAX_PATH];
    WCHAR widePath[MAX_PATH];

    if (!enabled) {
        return FALSE;
    }

    sounds_build_path(
        exePath,
        soundsDirectory,
        fileName,
        fullPath,
        sizeof(fullPath)
    );

    if (!fullPath[0]) {
        return FALSE;
    }

    if (MultiByteToWideChar(
            CP_ACP,
            0,
            fullPath,
            -1,
            widePath,
            ARRAYSIZE(widePath)) == 0) {
        return FALSE;
    }

    return PlaySoundW(
        widePath,
        NULL,
        SND_ASYNC |
        SND_FILENAME |
        SND_NODEFAULT
    );
}

BOOL sounds_play_ptt(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled)
{
    return sounds_play_file(
        exePath,
        soundsDirectory,
        fileName,
        enabled
    );
}

BOOL sounds_play_mic(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled)
{
    return sounds_play_file(
        exePath,
        soundsDirectory,
        fileName,
        enabled
    );
}