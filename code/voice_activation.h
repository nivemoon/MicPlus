#ifndef VOICE_ACTIVATION_H
#define VOICE_ACTIVATION_H

#include <windows.h>

typedef void (*VoiceActivationCallback)(BOOL active);

void voice_activation_set_callback(
    VoiceActivationCallback callback
);

void voice_activation_start(void);
void voice_activation_stop(void);

BOOL voice_activation_is_running(void);
BOOL voice_activation_is_speaking(void);

void voice_activation_set_threshold(double threshold);
void voice_activation_set_hangover_ms(DWORD ms);

#endif