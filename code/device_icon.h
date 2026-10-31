#ifndef DEVICE_ICON_H
#define DEVICE_ICON_H

#include <windows.h>
#include "audio_device_type.h"

AudioDeviceType device_icon_get_override(const wchar_t *deviceId);
void device_icon_set_override(const wchar_t *deviceId, const wchar_t *typeName);

HICON device_icon_load(const wchar_t *deviceId, AudioDeviceType detected, int active);
HICON device_icon_load_ptt(const wchar_t *deviceId, AudioDeviceType detected);

#endif