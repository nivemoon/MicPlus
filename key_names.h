#ifndef KEY_NAMES_H
#define KEY_NAMES_H

#include <windows.h>

void key_get_display_name(
    UINT virtualKey,
    char *out,
    int outSize
);

void key_build_hotkey_name(
    UINT modifiers,
    UINT virtualKey,
    char *out,
    int outSize
);

void key_get_ptt_label(
    UINT pttButton,
    UINT customVirtualKey,
    char *out,
    int outSize
);

#endif