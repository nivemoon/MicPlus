#ifndef AUDIO_DEVICE_H
#define AUDIO_DEVICE_H

int audio_device_init(void);
void audio_device_shutdown(void);

int audio_device_get_mic_mute(void);
void audio_device_set_mic_mute(int mute);

#endif