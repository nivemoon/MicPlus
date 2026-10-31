#define _CRT_SECURE_NO_WARNINGS

#include "sounds.h"
#include <stdlib.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>

#define SOUND_ALIAS "MICPLUS_SFX"

static CRITICAL_SECTION g_soundLock;
static BOOL g_soundLockInit = FALSE;
static int sound_alias_open = 0;
static char g_currentFile[MAX_PATH] = "";

static void sound_init_lock(void)
{
    if (!g_soundLockInit) {
        InitializeCriticalSection(&g_soundLock);
        g_soundLockInit = TRUE;
    }
}

const char *sounds_get_directory(const AppState *state)
{
    (void)state;

    return "sounds";
}

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
        g_currentFile[0] = '\0';
    }
}

static BOOL sounds_play_file_sync(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled,
    int volume)
{
    char fullPath[MAX_PATH];
    char command[1024];
    int mciVolume;
    BOOL result = FALSE;

    EnterCriticalSection(&g_soundLock);

    do {
        if (!enabled || volume <= 0) {
            break;
        }

        sounds_build_path(
            exePath,
            soundsDirectory,
            fileName,
            fullPath,
            sizeof(fullPath)
        );

        if (!fullPath[0]) {
            break;
        }


        /* Другой файл (или alias закрыт) — переоткрываем. */
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
            MCIERROR err = mciSendStringA(command, NULL, 0, NULL);

            if (err != 0) {
                char dbg[2048];

                snprintf(
                    dbg,
                    sizeof(dbg),
                    "exe=[%s]\nfile=[%s]\nfull=[%s]",
                    exePath ? exePath : "(null)",
                    fileName ? fileName : "(null)",
                    fullPath
                );

                MessageBoxA(NULL, dbg, "MCI OPEN FAIL", MB_OK);
                break;
            }
        }

        sound_alias_open = 1;
        lstrcpynA(g_currentFile, fullPath, sizeof(g_currentFile));

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
            break;
        }

        result = TRUE;
    } while (0);

    LeaveCriticalSection(&g_soundLock);
    return result;
}

typedef struct {
    char exePath[MAX_PATH];
    char soundsDirectory[MAX_PATH];
    char fileName[256];
    int enabled;
    int volume;
} SoundPlayParams;

static DWORD WINAPI sounds_play_thread(LPVOID param)
{
    SoundPlayParams *p = (SoundPlayParams *)param;

    sounds_play_file_sync(
        p->exePath,
        p->soundsDirectory,
        p->fileName,
        p->enabled,
        p->volume
    );

    free(p);

    return 0;
}

BOOL sounds_play_file(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled,
    int volume)
{
    SoundPlayParams *p;
    HANDLE thread;

    sound_init_lock();

    if (!enabled || volume <= 0) {
        return FALSE;
    }

    p = (SoundPlayParams *)malloc(sizeof(SoundPlayParams));

    if (!p) {
        return FALSE;
    }

    strncpy(p->exePath, exePath ? exePath : "", MAX_PATH - 1);
    p->exePath[MAX_PATH - 1] = '\0';

    strncpy(
        p->soundsDirectory,
        soundsDirectory ? soundsDirectory : "",
        MAX_PATH - 1
    );
    p->soundsDirectory[MAX_PATH - 1] = '\0';

    strncpy(
        p->fileName,
        fileName ? fileName : "",
        sizeof(p->fileName) - 1
    );
    p->fileName[sizeof(p->fileName) - 1] = '\0';

    p->enabled = enabled;
    p->volume = volume;

    /* Не ждём предыдущий поток — просто отсоединяем его.
       Доступ к MCI сериализуется критической секцией. */
    thread = CreateThread(
        NULL,
        0,
        sounds_play_thread,
        p,
        0,
        NULL
    );

    if (thread) {
        CloseHandle(thread);
    } else {
        free(p);
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

BOOL sounds_play_volume_tick(
    const char *exePath,
    const char *soundsDir,
    const char *fileName,
    int enabled,
    int volume)
{
    (void)volume;

    /* Фиксированная громкость щелчка — всегда одинаково */
    return sounds_play_file(
        exePath,
        soundsDir,
        fileName,
        enabled,
        20
    );
}