#ifndef AUDIO_DEVICE_TYPE_H
#define AUDIO_DEVICE_TYPE_H

typedef enum {
    AUDIO_TYPE_UNKNOWN = 0,
    AUDIO_TYPE_HEADPHONES,
    AUDIO_TYPE_AIRBUDS,
    AUDIO_TYPE_SPEAKERS
} AudioDeviceType;

AudioDeviceType audio_device_type_detect(void);

BOOL audio_device_output_active(void);

#endif