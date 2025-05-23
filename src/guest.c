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
#include "miniz.h"

POLKAVM_MIN_STACK_SIZE(16 * 4096);

static long ext_stdout(long buffer, size_t length) {
    corevm_yield_console_data(1, buffer, length);
    return length;
}

static char * ARGV[4] = {"./doom", "-iwad", "doom1.wad", "-timedemo"};

#define FRAMES_PER_SEC TICRATE

#define SAMPLE_RATE 44100
#define CHANNELS 2

#define DIV_CEIL(x, y) (((x) + ((y) - 1)) / (y))
#define MAX_SAMPLES_PER_FRAME DIV_CEIL(SAMPLE_RATE, FRAMES_PER_SEC)
#define AUDIO_BUFFER_LEN (MAX_SAMPLES_PER_FRAME * sizeof(int16_t) * CHANNELS)

uint64_t _pvm_start() {
    struct CoreVmVideoMode video_mode = {
        .width = DOOMGENERIC_RESX,
        .height = DOOMGENERIC_RESY,
        .refresh_rate = FRAMES_PER_SEC,
        .format = COREVM_VIDEO_RGB88_INDEXED8,
    };
    corevm_video_mode(&video_mode);
    struct CoreVmAudioMode audio_mode = {
        .sample_rate = SAMPLE_RATE,
        .channels = CHANNELS,
        .sample_format = COREVM_AUDIO_S16LE,
    };
    corevm_audio_mode(&audio_mode);
    doomgeneric_Create(4, ARGV);
    while (1) {
        doomgeneric_Tick();
    }
    return 0;
}

POLKAVM_EXPORT(uint64_t, _pvm_start);

static void flush_stdio() {
    fflush(stdout);
    fflush(stderr);
}

__attribute__((noreturn)) void abort(void) {
    fprintf(stderr, "\nabort() called!\n");
    flush_stdio();

    POLKAVM_TRAP();
}

#define MEMORY_SIZE (16 * 1024 * 1024)

static unsigned char _MEMORY[MEMORY_SIZE];
static unsigned long _SBRK = 0;

void *sbrk(intptr_t inc) {
    if (_SBRK + inc > MEMORY_SIZE) {
        errno = ENOMEM;
        return -1;
    }

    void * p = (void *)(_MEMORY + _SBRK);
    _SBRK += inc;
    return p;
}

long __syscall_cp(long n) {
    if (n == SYS_close) {
        return 0;
    } else {
        fprintf(stderr, "WARN: unhandled syscall(0): %li\n", n);
        flush_stdio();
        return -ENOSYS;
    }
}

long __syscall1(long n, long a0) {
    switch (n) {
        case SYS_close:
        case SYS_unlink:
        {
            return 0;
        }
        default:
        {
            fprintf(stderr, "WARN: unhandled syscall(1): %li\n", n);
            flush_stdio();
            return -ENOSYS;
        }
    }
}

long __syscall2(long n, long a0, long a1) {
    switch (n) {
        case SYS_mkdir:
        case SYS_rename:
        {
            return 0;
        }
        default:
        {
            fprintf(stderr, "WARN: unhandled syscall(2): %li\n", n);
            flush_stdio();
            return -ENOSYS;
        }
    }
}

#define FD_STDOUT 1
#define FD_STDERR 2
#define FD_MIDI 3
#define FD_DUMMY 10

static char * MIDI = 0;
static long MIDI_OFFSET = 0;
static long MIDI_SIZE = 0;
static long MIDI_CAPACITY = 0;

static long write_midi(long buffer, size_t length) {
    long tail = MIDI_OFFSET + length;
    if (tail > MIDI_CAPACITY) {
        if (MIDI_CAPACITY == 0) {
            MIDI_CAPACITY = 1024;
        }

        while (tail > MIDI_CAPACITY) {
            MIDI_CAPACITY = MIDI_CAPACITY * 2;
        }
        MIDI = realloc(MIDI, MIDI_CAPACITY);
    }

    memcpy(MIDI + MIDI_OFFSET, buffer, length);
    MIDI_OFFSET += length;
    if (MIDI_OFFSET > MIDI_SIZE) {
        MIDI_SIZE = MIDI_OFFSET;
    }

    return length;
}

static long write_dummy(long buffer, size_t length) {
    return length;
}

long __syscall3(long n, long a0, long a1, long a2) {
    switch (n) {
        case SYS_open:
        {
            const char * filename = a0;
            fprintf(stderr, "\nsys_open: '%s'\n", filename);
            flush_stdio();

            if (!strcmp(filename, "/tmp/doom.mid")) {
                MIDI_OFFSET = 0;
                return FD_MIDI;
            } else if (!strcmp(filename, "./.savegame/temp.dsg")) {
                return FD_DUMMY;
            } else {
                return -ENOENT;
            }
        }
        case SYS_lseek:
        {
            long * offset = 0;
            long * size = 0;
            if (a0 == FD_MIDI) {
                offset = &MIDI_OFFSET;
                size = &MIDI_SIZE;
            } else if (a0 == FD_DUMMY) {
                return 0;
            } else {
                fprintf(stderr, "WARN: lseek on an unknown FD: %li\n", a0);
                flush_stdio();

                return -EBADF;
            }

            switch (a2) {
                case SEEK_SET:
                    *offset = a1;
                    break;
                case SEEK_CUR:
                    *offset += a1;
                    break;
                case SEEK_END:
                    *offset = *size + a1;
                    break;
                default:
                    return -EINVAL;
            }

            return *offset;
        }
        case SYS_ioctl:
        {
            return -ENOSYS;
        }
        case SYS_readv:
        {
            long * fd_offset = 0;
            long * fd_size = 0;
            if (a0 == FD_MIDI) {
                fd_offset = &MIDI_OFFSET;
                fd_size = &MIDI_SIZE;
            } else if (a0 == FD_DUMMY) {
                return 0;
            } else {
                fprintf(stderr, "WARN: read from an unknown FD: %li\n", a0);
                flush_stdio();

                return -EBADF;
            }

            const struct iovec *iov = (const struct iovec *)a1;
            long bytes_read = 0;
            for (long i = 0; i < a2; ++i) {
                long remaining = *fd_size - *fd_offset;
                long length = iov[i].iov_len;
                if (remaining < length) {
                    length = remaining;
                }
                if (length == 0) {
                    break;
                }

                memcpy(iov[i].iov_base, MIDI + *fd_offset, length);
                *fd_offset += length;
                bytes_read += length;
            }

            return bytes_read;
        }
        case SYS_writev:
        {
            long (*write)(long, size_t) = NULL;
            if (a0 == FD_STDOUT || a0 == FD_STDERR) {
                write = ext_stdout;
            } else if (a0 == FD_MIDI) {
                write = write_midi;
            } else if (a0 == FD_DUMMY) {
                write = write_dummy;
            } else {
                fprintf(stderr, "WARN: write into an unknown FD: %li\n", a0);
                flush_stdio();

                return -EBADF;
            }

            const struct iovec *iov = (const struct iovec *)a1;
            long bytes_written = 0;
            for (long i = 0; i < a2; ++i) {
                long result = write(iov[i].iov_base, iov[i].iov_len);
                if (result < 0) {
                    return result;
                }
                bytes_written += result;
            }

            return bytes_written;
        }
        default:
        {
            fprintf(stderr, "WARN: unhandled syscall(3): %li\n", n);
            flush_stdio();

            return -ENOSYS;
        }
    }
}

long __syscall4(long n, long a0, long a1, long a2, long a3) {
    fprintf(stderr, "WARN: unhandled syscall(4): %li\n", n);
    flush_stdio();

    return -ENOSYS;
}

long __syscall6(long n, long a0, long a1, long a2, long a3, long a4, long a5) {
    fprintf(stderr, "WARN: unhandled syscall(6): %li\n", n);
    flush_stdio();

    return -ENOSYS;
}

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
    pump_audio();
    ++frame_number;
    timestamp = frame_number * 1000000000UL / FRAMES_PER_SEC;
}

#pragma GCC push_options
#pragma GCC optimize ("O3")

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

#pragma GCC pop_options

int DG_GetKey(int * is_pressed, unsigned char * key) {
    return 0;
}

void DG_SetWindowTitle(const char * title) {}
