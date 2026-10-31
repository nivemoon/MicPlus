#ifndef AUDIO_ICONS_H
#define AUDIO_ICONS_H

#include <windows.h>
#include "audio_device_type.h"

HICON audio_icon_load(AudioDeviceType type, int active);

HICON audio_icon_load_ptt(AudioDeviceType type);

HICON audio_icon_load_ptt_off(void);

#endif