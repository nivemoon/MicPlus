#ifndef ALIAS_H
#define ALIAS_H

#include <windows.h>

/* Указать путь к основному ini (вызывается один раз при старте) */
void alias_set_path(
    const char *exe_path,
    const char *appdata_path
);

/* Получить псевдоним по ID устройства.
   Возвращает NULL, если псевдонима нет. */
const wchar_t *alias_get(const wchar_t *device_id);

/* Сохранить (или перезаписать) псевдоним */
int alias_set(const wchar_t *device_id, const wchar_t *name);

/* Удалить псевдоним (вернуть системное имя) */
int alias_remove(const wchar_t *device_id);

#endif