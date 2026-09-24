#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <windows.h>

BOOL microphone_run_ctl(
    const char *exePath,
    int mute
);

#endif