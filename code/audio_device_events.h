#ifndef AUDIO_DEVICE_EVENTS_H
#define AUDIO_DEVICE_EVENTS_H

#include <windows.h>

#define WM_APP_DEVICE_CHANGED (WM_APP + 200)

BOOL audio_device_events_start(HWND hWnd);
void audio_device_events_stop(void);

#endif