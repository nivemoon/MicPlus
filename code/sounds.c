#define _CRT_SECURE_NO_WARNINGS

#include "sounds.h"

#include <mmsystem.h>
#include <stdio.h>
#include <string.h>

#define SOUND_ALIAS "MICPLUS_SFX"

static int sound_alias_open = 0;

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

static void sound_close_alias(void)
{
    if (sound_alias_open) {
        mciSendStringA(
            "close " SOUND_ALIAS,
            NULL,
            0,
            NULL
        );

        sound_alias_open = 0;
    }
}

BOOL sounds_play_file(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled,
    int volume)
{
    char fullPath[MAX_PATH];
    char command[1024];
    int mciVolume;

    if (!enabled || volume <= 0) {
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

    sound_close_alias();

    {
    const char *deviceType;
    const char *extension;

    extension = strrchr(fullPath, '.');

    if (extension &&
        lstrcmpiA(extension, ".mp3") == 0) {
        deviceType = "mpegvideo";
    } else {
        deviceType = "waveaudio";
    }

    snprintf(
        command,
        sizeof(command),
        "open \"%s\" type %s alias " SOUND_ALIAS,
        fullPath,
        deviceType
    );
}

    {
    MCIERROR mciError;

    mciError = mciSendStringA(command, NULL, 0, NULL);

    if (mciError != 0) {
        char errorText[256];

        snprintf(
            errorText,
            sizeof(errorText),
            "MCI open failed.\nPath:\n%s\nError code: %lu",
            fullPath,
            (unsigned long)mciError
        );

        MessageBoxA(
            NULL,
            errorText,
            "MicPlus MCI error",
            MB_OK | MB_ICONERROR
        );

        return FALSE;
    }
}

    sound_alias_open = 1;

    if (volume > 100) {
        volume = 100;
    }

    mciVolume = volume * 10;

    snprintf(
        command,
        sizeof(command),
        "setaudio " SOUND_ALIAS " volume to %d",
        mciVolume
    );

    mciSendStringA(command, NULL, 0, NULL);

    if (mciSendStringA(
            "play " SOUND_ALIAS,
            NULL,
            0,
            NULL) != 0) {
        sound_close_alias();
        return FALSE;
    }

    return TRUE;
}

BOOL sounds_play_ptt(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled,
    int volume)
{
    return sounds_play_file(
        exePath,
        soundsDirectory,
        fileName,
        enabled,
        volume
    );
}

BOOL sounds_play_mic(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled,
    int volume)
{
    return sounds_play_file(
        exePath,
        soundsDirectory,
        fileName,
        enabled,
        volume
    );
}

BOOL sounds_play_ptt_mode(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled,
    int volume)
{
    return sounds_play_file(
        exePath,
        soundsDirectory,
        fileName,
        enabled,
        volume
    );
}

const char *sounds_get_directory(
    const AppState *state)
{
    if (state &&
        state->soundScheme == 1) {
        return "sounds\\micplus";
    }

    return "sounds\\classic";
}