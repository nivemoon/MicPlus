#ifndef SOUNDS_H
#define SOUNDS_H

#include <windows.h>

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
    int enabled
);

BOOL sounds_play_ptt(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled
);

BOOL sounds_play_mic(
    const char *exePath,
    const char *soundsDirectory,
    const char *fileName,
    int enabled
);

#endif
