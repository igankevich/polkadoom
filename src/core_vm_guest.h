#ifndef CORE_VM_GUEST_H
#define CORE_VM_GUEST_H

#include <stdint.h>

#include "polkavm_guest.h"

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

POLKAVM_IMPORT_V2(200, void, copy_out, uint64_t, uint64_t);
POLKAVM_IMPORT_V2(201, uint64_t, mem_alloc, uint64_t);
POLKAVM_IMPORT_V2(202, void, mem_free, uint64_t, uint64_t);

#endif
