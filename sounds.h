#ifndef SOUNDS_H
#define SOUNDS_H

#include <windows.h>

#include "app_types.h"

void sounds_build_path(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    char *outPath,
    int outSize
);

BOOL sounds_play_file(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled,
    int volume
);

BOOL sounds_play_ptt(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled,
    int volume
);

BOOL sounds_play_ptt_mode(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled,
    int volume
);

BOOL sounds_play_mic(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled,
    int volume
);

const char *sounds_get_directory(
    const AppState *state);

#endif