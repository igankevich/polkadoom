#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <SDL.h>

#include "polkavm_guest.h"
#include "corevm_guest.h"
#include "../libs/doomgeneric/doomgeneric/doomgeneric.h"
#include "../libs/doomgeneric/doomgeneric/i_timer.h"
#include "../libs/SDL/src/audio/SDL_audio_c.h"
#include "../libs/SDL/src/audio/SDL_sysaudio.h"

static char * ARGV[4] = {"./doom", "-iwad", "doom1.wad", "-timedemo"};

// Original frame rate is 35
#define FRAMES_PER_SEC TICRATE

#define SAMPLE_RATE (44100/2)
#define CHANNELS 1

#define DIV_CEIL(x, y) (((x) + ((y) - 1)) / (y))
#define MAX_SAMPLES_PER_FRAME DIV_CEIL(SAMPLE_RATE, FRAMES_PER_SEC)
#define AUDIO_BUFFER_LEN (MAX_SAMPLES_PER_FRAME * sizeof(int16_t) * CHANNELS)

int main() {
    struct CoreVmVideoMode video_mode = {
        .width = DOOMGENERIC_RESX,
        .height = DOOMGENERIC_RESY,
        .refresh_rate = FRAMES_PER_SEC,
        .format = COREVM_VIDEO_RGB88_INDEXED8,
    };
    corevm_video_mode(&video_mode);
    #if !defined(DOOM_NO_AUDIO)
    struct CoreVmAudioMode audio_mode = {
        .sample_rate = SAMPLE_RATE,
        .channels = CHANNELS,
        .sample_format = COREVM_AUDIO_S16LE,
    };
    corevm_audio_mode(&audio_mode);
    #endif
    doomgeneric_Create(4, ARGV);
    while (1) {
        doomgeneric_Tick();
    }
    return 0;
}

POLKAVM_EXPORT(int, main);

static inline size_t next_power_of_2(size_t x) {
    size_t n = 1;
    while (n < x) {
        n <<= 1;
    }
    return n;
}

static SDL_AudioDevice * AUDIO_DEVICE = NULL;

static int AUDIO_OpenDevice(SDL_AudioDevice *this, const char *devname) {
    this->spec.samples = next_power_of_2(SAMPLE_RATE / FRAMES_PER_SEC);
    this->spec.format = AUDIO_S16;
    this->spec.freq = SAMPLE_RATE;
    this->spec.channels = CHANNELS;

    SDL_CalculateAudioSpec(&this->spec);

    printf("Opened audio device: buffer size = %lu\n", (unsigned long)this->spec.size);
    AUDIO_DEVICE = this;
    return 0;
}

static void AUDIO_CloseDevice(SDL_AudioDevice *this) {}

static SDL_bool AUDIO_Init(SDL_AudioDriverImpl *impl)
{
    impl->OpenDevice = AUDIO_OpenDevice;
    impl->CloseDevice = AUDIO_CloseDevice;
    impl->OnlyHasDefaultOutputDevice = SDL_TRUE;
    impl->ProvidesOwnCallbackThread = SDL_TRUE;
    return SDL_TRUE;
}

AudioBootStrap DUMMYAUDIO_bootstrap = {
    "doom", "DOOM audio", AUDIO_Init, SDL_FALSE
};

// This is based on SDL's Haiku backend.
void doom_get_audio(void * stream, size_t len)
{
    SDL_AudioDevice * audio = AUDIO_DEVICE;
    SDL_AudioCallback callback = audio->callbackspec.callback;
    SDL_LockMutex(audio->mixer_lock);

    if (!SDL_AtomicGet(&audio->enabled) || SDL_AtomicGet(&audio->paused)) {
        if (audio->stream) {
            SDL_AudioStreamClear(audio->stream);
        }
        SDL_memset(stream, audio->spec.silence, len);
    } else {
        SDL_assert(audio->spec.size == len);

        if (audio->stream == NULL) {  /* no conversion necessary. */
            callback(audio->callbackspec.userdata, (Uint8 *) stream, len);
        } else {  /* streaming/converting */
            const int stream_len = audio->callbackspec.size;
            const int ilen = (int) len;
            while (SDL_AudioStreamAvailable(audio->stream) < ilen) {
                callback(audio->callbackspec.userdata, audio->work_buffer, stream_len);
                if (SDL_AudioStreamPut(audio->stream, audio->work_buffer, stream_len) == -1) {
                    SDL_AudioStreamClear(audio->stream);
                    SDL_AtomicSet(&audio->enabled, 0);
                    break;
                }
            }

            const int got = SDL_AudioStreamGet(audio->stream, stream, ilen);
            SDL_assert((got < 0) || (got == ilen));
            if (got != ilen) {
                SDL_memset(stream, audio->spec.silence, len);
            }
        }
    }

    SDL_UnlockMutex(audio->mixer_lock);
}

void DG_Init(void) {}

// Current in-game time in nanoseconds.
static size_t timestamp = 0;
// Current frame number;
static size_t frame_number = 0;
static char audio_buffer[AUDIO_BUFFER_LEN];

static void pump_audio();

void DG_DrawFrame(void) {
    #if !defined(DOOM_NO_AUDIO)
    pump_audio();
    #endif
    ++frame_number;
    timestamp = frame_number * 1000000000UL / FRAMES_PER_SEC;
}

static void pump_audio() {
    size_t start_sample = frame_number * SAMPLE_RATE / FRAMES_PER_SEC;
    size_t end_sample = (frame_number + 1) * SAMPLE_RATE / FRAMES_PER_SEC;
    size_t buffer_len = (end_sample - start_sample) * sizeof(int16_t) * CHANNELS;
    doom_get_audio(audio_buffer, buffer_len);
    corevm_yield_audio_samples(audio_buffer, buffer_len);
}

void DG_SleepMs(uint32_t ms) {
    timestamp += ((size_t)ms) * 1000UL;
}

uint32_t DG_GetTicksMs(void) {
    return timestamp / 1000UL;
}

void SDL_Delay(Uint32 ms) {
    DG_SleepMs(ms);
}

Uint32 SDL_GetTicks(void) {
    return DG_GetTicksMs();
}

int DG_GetKey(int * is_pressed, unsigned char * key) {
    return 0;
}

void DG_SetWindowTitle(const char * title) {}
