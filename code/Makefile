CC = gcc
WINDRES = windres

CFLAGS = -Wall -Wextra -O2 -mwindows
LDFLAGS = -mwindows

TARGET = MicPlus.exe
RESOURCE = res.o

SOURCES = \
    micplus.c \
    app.c \
    paths.c \
    startup.c \
    settings.c \
    theme.c \
    localization.c \
    key_names.c \
    sounds.c \
    microphone.c \
    system_sound.c \
    tray.c \
    tray_menu.c \
    key_capture.c \
    hooks.c \
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
	-lwinhttp

all: $(TARGET)

$(TARGET): $(OBJECTS) $(RESOURCE)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(RESOURCE) $(LIBS)

res.o: res.rc resource.h
	$(WINDRES) res.rc -O coff -o res.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q *.o $(TARGET) 2>NUL
