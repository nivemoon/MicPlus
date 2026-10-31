#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "voice_activation.h"
#define VA_DEBUG 0

static FILE *va_debug_open(void)
{
#if VA_DEBUG
    return fopen("va_debug.txt", "a");
#else
    return NULL;
#endif
}

#define VA_SAMPLE_RATE     16000
#define VA_CHANNELS        1
#define VA_BITS_PER_SAMPLE 16
#define VA_BUFFER_MS       30
#define VA_BUFFER_COUNT    3

static VoiceActivationCallback g_callback;
static HWAVEIN g_waveIn;
static WAVEHDR g_headers[VA_BUFFER_COUNT];
static SHORT g_buffers[VA_BUFFER_COUNT]
    [VA_SAMPLE_RATE * VA_BUFFER_MS / 1000];

static BOOL g_running;
static BOOL g_speaking;
static DWORD g_silenceMs;
static double g_threshold = 0.02;
static double g_releaseThreshold = 0.008;
static DWORD g_hangoverMs = 500;

static CRITICAL_SECTION g_lock;

static void va_emit(BOOL active)
{
    if (g_callback) {
        g_callback(active);
    }
}

static void va_process_buffer(const WAVEHDR *header)
{
    const SHORT *samples = (const SHORT *)header->lpData;
    UINT sampleCount = header->dwBytesRecorded / sizeof(SHORT);
    double sum = 0.0;
    double rms;
    UINT i;

    if (sampleCount == 0) {
        return;
    }

    for (i = 0; i < sampleCount; i++) {
        double s = samples[i] / 32768.0;
        sum += s * s;
    }

    rms = sqrt(sum / sampleCount);

    {
        static UINT logCounter = 0;

        if ((logCounter++ % 10) == 0) {
            FILE *log = va_debug_open();
            if (log) {
                fprintf(log, "rms=%.5f\n", rms);
                fclose(log);
            }
        }
    }

    EnterCriticalSection(&g_lock);

     if (!g_speaking && rms >= g_threshold) {
        g_speaking = TRUE;
        g_silenceMs = 0;
        va_emit(TRUE);
    } else if (g_speaking) {
        if (rms < g_releaseThreshold) {
            g_silenceMs += VA_BUFFER_MS;

            if (g_silenceMs >= g_hangoverMs) {
                g_speaking = FALSE;
                g_silenceMs = 0;
                va_emit(FALSE);
            }
        } else {
            g_silenceMs = 0;
        }
    }

    LeaveCriticalSection(&g_lock);
}

static void CALLBACK va_wave_in_proc(
    HWAVEIN waveIn,
    UINT message,
    DWORD_PTR instance,
    DWORD_PTR param1,
    DWORD_PTR param2)
{
    WAVEHDR *header;
    UINT index;
    UINT i;

    (void)waveIn;
    (void)instance;
    (void)param2;

    {
        FILE *log = va_debug_open();
        if (log) {
            fprintf(log, "wave callback: msg=%u\n", (unsigned)message);
            fclose(log);
        }
    }

    if (message != WIM_DATA) {
        return;
    }

    header = (WAVEHDR *)param1;

    va_process_buffer(header);

    for (i = 0; i < VA_BUFFER_COUNT; i++) {
        if (&g_headers[i] == header) {
            index = i;
            break;
        }
    }

    if (i < VA_BUFFER_COUNT && g_running) {
        waveInAddBuffer(g_waveIn, &g_headers[index], sizeof(WAVEHDR));
    }
}

void voice_activation_set_callback(
    VoiceActivationCallback callback)
{
    g_callback = callback;
}

void voice_activation_set_threshold(double threshold)
{
    g_threshold = threshold;
}

void voice_activation_set_hangover_ms(DWORD ms)
{
    g_hangoverMs = ms;
}

BOOL voice_activation_is_running(void)
{
    return g_running;
}

BOOL voice_activation_is_speaking(void)
{
    return g_speaking;
}

void voice_activation_start(void)
{
    WAVEFORMATEX format;
    UINT i;

    if (g_running) {
        return;
    }

    {
        FILE *log = va_debug_open();
        if (log) {
            fprintf(log, "va_start called\n");
            fclose(log);
        }
    }

    InitializeCriticalSection(&g_lock);

    ZeroMemory(&format, sizeof(format));
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = VA_CHANNELS;
    format.nSamplesPerSec = VA_SAMPLE_RATE;
    format.wBitsPerSample = VA_BITS_PER_SAMPLE;
    format.nBlockAlign =
        format.nChannels * format.wBitsPerSample / 8;
    format.nAvgBytesPerSec =
        format.nSamplesPerSec * format.nBlockAlign;

       if (waveInOpen(
            &g_waveIn,
            WAVE_MAPPER,
            &format,
            (DWORD_PTR)va_wave_in_proc,
            0,
            CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {

        {
            FILE *log = va_debug_open();
            if (log) {
                fprintf(log, "waveInOpen FAILED\n");
                fclose(log);
            }
        }

        DeleteCriticalSection(&g_lock);
        return;
    }

    {
        FILE *log = va_debug_open();
        if (log) {
            fprintf(log, "waveInOpen OK\n");
            fclose(log);
        }
    }

     for (i = 0; i < VA_BUFFER_COUNT; i++) {
        MMRESULT mr;

        ZeroMemory(&g_headers[i], sizeof(WAVEHDR));
        g_headers[i].lpData = (LPSTR)g_buffers[i];
        g_headers[i].dwBufferLength = sizeof(g_buffers[i]);

        mr = waveInPrepareHeader(g_waveIn, &g_headers[i], sizeof(WAVEHDR));

        {
            FILE *log = va_debug_open();
            if (log) {
                fprintf(log, "prepare[%u]=%u\n", (unsigned)i, (unsigned)mr);
                fclose(log);
            }
        }

        mr = waveInAddBuffer(g_waveIn, &g_headers[i], sizeof(WAVEHDR));

        {
            FILE *log = va_debug_open();
            if (log) {
                fprintf(log, "add[%u]=%u\n", (unsigned)i, (unsigned)mr);
                fclose(log);
            }
        }
    }

    {
        MMRESULT mr = waveInStart(g_waveIn);
        FILE *log = va_debug_open();

        if (log) {
            fprintf(log, "waveInStart=%u\n", (unsigned)mr);
            fclose(log);
        }
    }

    g_running = TRUE;
}

void voice_activation_stop(void)
{
    UINT i;

    if (!g_running) {
        return;
    }

    g_running = FALSE;

    waveInReset(g_waveIn);

    for (i = 0; i < VA_BUFFER_COUNT; i++) {
        waveInUnprepareHeader(g_waveIn, &g_headers[i], sizeof(WAVEHDR));
    }

    waveInClose(g_waveIn);
    g_waveIn = NULL;

    if (g_speaking) {
        g_speaking = FALSE;
        va_emit(FALSE);
    }

    DeleteCriticalSection(&g_lock);
}