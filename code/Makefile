CC = gcc
WINDRES = windres

CFLAGS = -Wall -Wextra -O2 -mwindows -finput-charset=UTF-8 -fexec-charset=CP1251
LDFLAGS = -mwindows

TARGET = MicPlus.exe
RESOURCE = res.o

SOURCES = \
    micplus.c \
    app.c \
    paths.c \
    micplus_ini.c \
    startup.c \
    settings.c \
    theme.c \
    localization.c \
    key_names.c \
    sounds.c \
    microphone.c \
    audio_device.c \
    audio_device_type.c \
    audio_device_events.c \
    audio_icons.c \
    voice_activation.c \
    switch_devices.c \
    alias.c \
    alias_dialog.c \
    system_sound.c \
    tray.c \
    tray_menu.c \
    key_capture.c \
    multimedia.c \
    multimedia_window.c \
	keep_alive.c \
    hooks.c \
    passive_ptt.c \
    unicode.c \
    updater.c

OBJECTS = $(SOURCES:.c=.o)

LIBS = \
    -lshell32 \
    -lole32 \
    -loleaut32 \
    -luuid \
    -lcomctl32 \
    -lshlwapi \
    -lwinmm \
    -lversion \
    -ladvapi32 \
    -lwinhttp \
    -ldwmapi

all: $(TARGET)

$(TARGET): $(OBJECTS) $(RESOURCE)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(RESOURCE) $(LIBS)

res.o: res.rc resource.h
	$(WINDRES) res.rc -O coff -o res.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

settings_window.o: settings_window.c settings_window.h app_types.h
	$(CC) $(CFLAGS) -finput-charset=UTF-8 -fexec-charset=CP1251 -c settings_window.c -o $@

clean:
	del /Q *.o $(TARGET) 2>NUL