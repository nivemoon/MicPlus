#ifndef MICPLUS_INI_H
#define MICPLUS_INI_H

#define INI_NAME "micplus.ini"

void IniInit(void);                 /* вызвать один раз при старте */
const char* IniPath(void);          /* путь активного ini */

int  IniGetInt(const char *section, const char *key, int def);
void IniSetInt(const char *section, const char *key, int value);

/* строка: buf размером buf_size */
void IniGetStr(const char *section, const char *key,
               const char *def, char *buf, int buf_size);
void IniSetStr(const char *section, const char *key, const char *value);

void IniDeleteEverywhere(void);     /* удалить ini и aliases.ini везде */
void IniMarkDeleted(void);          /* запретить пересоздание до перезапуска */
int  IniIsDeleted(void);            /* 1 — настройки были удалены */

#endif
