#ifndef APP_TYPES_H
#define APP_TYPES_H

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <shellapi.h>
#include <stddef.h>

typedef enum {
    LANG_EN = 0,
    LANG_RU = 1
} APP_LANG;

typedef struct {
    char exePath[MAX_PATH];
    char iniPath[MAX_PATH];
    char startupPath[MAX_PATH];
    char appDataPath[MAX_PATH];      
    char micctlPath[MAX_PATH]; 
} AppPaths;

typedef struct {
    int muted;
    int pttMode;
    int soundMuted;
    int iconMode;
    int menuTheme;

    UINT vkToggle;
    UINT modToggle;
    int toggleCustomSet;

    UINT pttButton;
    UINT vkPttCustom;

    int pttKeyDown;
    int toggleKeyDown;

    APP_LANG lang;

    UINT vkSoundToggle;
    UINT modSoundToggle;

    int autoStart;
    int enableMicSounds;
    int enablePTTSounds;
    int enableBalloonTips;

    int hideTrayIcon;      // НОВОЕ: скрывать значок из трея

    char micSoundOn[MAX_PATH];
    char micSoundOff[MAX_PATH];
    char pttSoundOn[MAX_PATH];
    char pttSoundOff[MAX_PATH];

    HICON hIconOnLight;
    HICON hIconOnDark;
    HICON hIconOnCurrent;
    HICON hIconOffRed;

    HINSTANCE hInst;
    HHOOK hMouseHook;
    HHOOK hKeyboardHook;

    NOTIFYICONDATAW nid;
    HWND hMainWnd;
} AppState;

typedef struct {
    AppPaths paths;
    AppState state;
} AppContext;

#endif