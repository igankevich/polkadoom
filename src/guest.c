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
#include "core_vm_guest.h"
#include "../libs/doomgeneric/doomgeneric/doomgeneric.h"
#include "../libs/SDL/src/audio/SDL_audio_c.h"
#include "../libs/SDL/src/audio/SDL_sysaudio.h"

POLKAVM_MIN_STACK_SIZE(16 * 4096);

static long ext_stdout(long buffer, size_t length) {
    // TODO
    //copy_out(buffer, length);
    return length;
}

static void ext_output_audio(long buffer, size_t length) {}

/*
struct KeyPress {
    int tic;
    unsigned char key;
    unsigned char is_pressed;
}

static const unsigned int MAX_KEY_PRESSES = 32;
static struct KeyPress key_presses[MAX_KEY_PRESSES];
static unsigned int key_presses_write_index = 0;
static unsigned int key_presses_read_index = 0;
*/

static char * ARGV[4] = {"./doom", "-iwad", "doom1.wad", "-timedemo"};

uint64_t ext_main() {
    doomgeneric_Create(4, ARGV);
    //doomgeneric_Tick();
    //while (1) {
    //    doomgeneric_Tick();
    //}
    return 0;
}

POLKAVM_EXPORT(uint64_t, ext_main);

void DG_Init(void) {}

void DG_DrawFrame(void) {
    copy_out(
        (uint64_t) DG_ScreenBuffer,
        (uint64_t) (DOOMGENERIC_RESX * DOOMGENERIC_RESY)
    );
}

int DG_GetKey(int * is_pressed, unsigned char * key) {
    return 0;
}

void DG_SetWindowTitle(const char * title) {}

static uint32_t SUBSAMPLES = 0;

void DG_SleepMs(uint32_t ms) {
    SUBSAMPLES += ms;
}

uint32_t DG_GetTicksMs(void) {
    // Make sure there's always forward progress.
    SUBSAMPLES += 1;
    return SUBSAMPLES;
}

void SDL_Delay(uint32_t ms) {
    DG_SleepMs(ms);
}

Uint32 SDL_GetTicks(void) {
    return SUBSAMPLES;
}

static SDL_bool AUDIO_Init(SDL_AudioDriverImpl *impl)
{
    return SDL_FALSE;
}

AudioBootStrap DUMMYAUDIO_bootstrap = {
    "doom", "DOOM audio", AUDIO_Init, SDL_FALSE
};

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
            fprintf(stderr, "sys_lseek\n");
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
            fprintf(stderr, "sys_readv\n");
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
