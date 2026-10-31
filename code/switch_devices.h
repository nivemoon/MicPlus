#ifndef SWITCH_DEVICES_H
#define SWITCH_DEVICES_H

#include <windows.h>

#define SWITCH_DEVICES_MAX 16
#define SWITCH_DEVICES_NAME_LEN 128

typedef struct {
    wchar_t name[SWITCH_DEVICES_NAME_LEN];
    wchar_t id[128];
    BOOL isDefault;
} SwitchDevice;

typedef struct {
    SwitchDevice devices[SWITCH_DEVICES_MAX];
    int count;
} SwitchDeviceList;

/* Заполняет список устройств вывода (render) */
BOOL switch_devices_get_output(SwitchDeviceList *list);

/* Переключить устройство вывода по умолчанию по ID */
BOOL switch_devices_set_default(const wchar_t *deviceId);

/* Заполняет список устройств ввода (capture) */
BOOL switch_devices_get_input(SwitchDeviceList *list);

/* Назначить устройство ввода по умолчанию для связи (Discord, игры) */
BOOL switch_devices_set_default_input(const wchar_t *deviceId);

#endif