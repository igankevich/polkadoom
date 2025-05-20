#ifndef COREVM_GUEST_H
#define COREVM_GUEST_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "polkavm_guest.h"

// Sanity checks.
static_assert(sizeof(size_t) <= sizeof(uint64_t), "`size_t` is too large");
static_assert(sizeof(void*) <= sizeof(uint64_t), "`void*` is too large");

struct PolkaVM_Metadata_V2 {
    uint8_t version;
    uint32_t flags;
    uint32_t symbol_length;
    const char * symbol;
    uint8_t input_regs;
    uint8_t output_regs;
    uint8_t has_index;
    uint32_t index;
} __attribute__ ((packed));

#define POLKAVM_IMPORT_V2(arg_index, arg_return_ty, fn_name, ...) \
static struct PolkaVM_Metadata_V2 POLKAVM_JOIN(fn_name, __IMPORT_METADATA) __attribute__ ((section(".polkavm_metadata"))) = { \
    .version = 2, \
    .flags = 0, \
    .symbol_length = sizeof(#fn_name) - 1, \
    .symbol = #fn_name, \
    .input_regs = POLKAVM_COUNT_REGS(__VA_ARGS__), \
    .output_regs = POLKAVM_COUNT_REGS(arg_return_ty), \
    .has_index = 1, \
    .index = arg_index \
}; \
static arg_return_ty __attribute__ ((naked)) fn_name(POLKAVM_IMPORT_ARGS_IMPL(__VA_ARGS__)) { \
    __asm__( \
        POLKAVM_IMPORT_DEF() \
        "ret\n" \
        : \
        : \
          [metadata] "i" (&POLKAVM_JOIN(fn_name, __IMPORT_METADATA)) \
        : "memory" \
    ); \
}

POLKAVM_IMPORT_V2(0, uint64_t, corevm_gas);
POLKAVM_IMPORT_V2(1, uint64_t, corevm_alloc, uint64_t);
POLKAVM_IMPORT_V2(2, void, corevm_free, uint64_t, uint64_t);
POLKAVM_IMPORT_V2(3, uint64_t, corevm_yield_console_data, uint64_t, uint64_t, uint64_t);
POLKAVM_IMPORT_V2(4, uint64_t, corevm_yield_video_frame_impl, uint64_t, uint64_t);
POLKAVM_IMPORT_V2(5, void, corevm_video_mode_impl, uint64_t, uint64_t, uint64_t, uint64_t);

#ifndef COREVM_PRINTF_BUFFER_LEN
#define COREVM_PRINTF_BUFFER_LEN 4096
#endif

#define corevm_printf_impl(stream, format, ...) \
    { \
        char buffer[COREVM_PRINTF_BUFFER_LEN]; \
        int n = snprintf(buffer, COREVM_PRINTF_BUFFER_LEN, format, ##__VA_ARGS__); \
        if (n > 0) { \
            if (n == COREVM_PRINTF_BUFFER_LEN) { \
                n = COREVM_PRINTF_BUFFER_LEN - 1; \
            } \
            buffer[n] = 0; \
            while (true) { \
                uint64_t ret = corevm_yield_console_data(stream, (uint64_t)buffer, (uint64_t)(n + 1)); \
                if (ret == 0) { \
                    break; \
                } \
            } \
        } \
    }

#define corevm_printf(format, ...) corevm_printf_impl(1, format, ##__VA_ARGS__)
#define corevm_eprintf(format, ...) corevm_printf_impl(2, format, ##__VA_ARGS__)

inline static void corevm_yield_video_frame(const void* frame, size_t frame_len) {
    while (1) {
        uint64_t ret = corevm_yield_video_frame_impl((uint64_t) frame, (uint64_t) frame_len);
        if (ret == 0) {
            break;
        }
    }
}

enum CoreVmVideoFrameFormat {
    COREVM_VIDEO_RGB88_INDEXED8 = 1
};

struct CoreVmVideoMode {
    uint32_t width;
    uint32_t height;
    uint16_t refresh_rate;
    enum CoreVmVideoFrameFormat format;
};

inline static void corevm_video_mode(const struct CoreVmVideoMode* mode) {
    corevm_video_mode_impl(
        (uint64_t) mode->width,
        (uint64_t) mode->height,
        (uint64_t) mode->refresh_rate,
        (uint64_t) mode->format
    );
}

#endif
