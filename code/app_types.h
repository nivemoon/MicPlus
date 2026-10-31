#ifndef APP_TYPES_H
#define APP_TYPES_H
#define WM_APP_EMERGENCY (WM_APP + 103)
#define WM_APP_DUSK_PANEL  (WM_APP + 104)
#define WM_APP_DUSK_LIST   (WM_APP + 105)
#define WM_APP_DUSK_OUTPUT (WM_APP + 106)
#define WM_APP_DUSK_INPUT  (WM_APP + 107)
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define WM_APP_MIC_STATE (WM_APP + 100)
#define WM_APP_VAD_STATE (WM_APP + 102)
#include <windows.h>
#include <shellapi.h>
#include <stddef.h>

typedef enum {
    LANG_EN = 0,
    LANG_RU = 1
} APP_LANG;

typedef struct {
    char exePath[1024];      // источник
    char iniPath[1024];      // источник
    char startupPath[1200];  // 1024 + 46 запаса
    char appDataPath[1200];  // 1024 + 46 запаса
    char micctlPath[1320];   // 1200 + 11 запаса
} AppPaths;

typedef struct {
    int muted;
    int pttMode;
    UINT vkPttModeToggle;
    UINT modPttModeToggle;
    int soundMuted;
    
    int menuTheme;
    int iconMode;
    int useAudioIcon;
    UINT vkToggle;
    UINT modToggle;
    int toggleCustomSet;

    UINT vkMicChordFirst;
    UINT vkMicChordSecond;
    int micChordKeyDown;

    UINT vkPttModeChordFirst;
    UINT vkPttModeChordSecond;
    int pttModeChordKeyDown;

    UINT pttButton;
    UINT vkPttCustom;

    int pttKeyDown;
    int pttActive;
	int toggleKeyDown;
	
	int voiceActive;
    int pttButtonActive;
    int voiceActivationEnabled;

	UINT panelHotkeyMods;
	UINT panelHotkeyVk;
	UINT outputHotkeyMods;
	UINT outputHotkeyVk;
	UINT listHotkeyMods;
	UINT listHotkeyVk;
	UINT inputHotkeyMods;
	UINT inputHotkeyVk;

    APP_LANG lang;

    UINT vkSoundToggle;
    UINT modSoundToggle;

    int autoStart;
    int enableMicSounds;
    int enablePTTSounds;
    int enablePTTModeSounds;
    int enableBalloonTips;
    int soundVolume;
    int hideTrayIcon;
    int hideMicSelection;
    int keepHeadphonesAwake;
    int keepAliveBalloonShown;
    int leftClickAction;
    int middleClickAction;
    int scrollAction;
    int enableVolumeTickSound;
	int micSoundFromPtt;
	
	int voiceHangoverMs;   /* 300/500/1000/2000 */
	int voiceThresholdX1000; /* порог * 1000: 10/20/30/50 */
	
    char micSoundOn[600];
    char micSoundOff[600];
    char pttSoundOn[600];
    char pttSoundOff[600];

    HICON hIconOnLight;
    HICON hIconOnDark;
    HICON hIconOnCurrent;
    HICON hIconOffRed;
    HICON hIconAudio;

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