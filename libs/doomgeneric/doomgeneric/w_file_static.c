#include <stdio.h>
#include <string.h>

#include "m_misc.h"
#include "w_file.h"
#include "z_zone.h"

// Embedded `doom1.wad` as `roms_doom1_wad`.
#include "doom1_wad.c"

long ROM_SIZE = sizeof(roms_doom1_wad);

#define MIN(a,b) ((a)<(b)?(a):(b))

typedef struct
{
    wad_file_t wad;
    FILE *fstream;
} stdc_wad_file_t;

extern wad_file_class_t static_wad_file;

static wad_file_t *wad_open(char *path)
{
    if (strcmp(path, "doom1.wad")) {
        return NULL;
    }

    wad_file_t* wad = Z_Malloc(sizeof(wad_file_t), PU_STATIC, 0);
    wad->file_class = &static_wad_file;
    wad->mapped = (unsigned char*) roms_doom1_wad;
    wad->length = sizeof(roms_doom1_wad);
    fprintf(stderr, "wad_open %s = %p\n", path, wad);
    return wad;
}

static void wad_close(wad_file_t *wad)
{
    fprintf(stderr, "wad_close %p\n", wad);
    Z_Free(wad);
}

// Read data from the specified position in the file into the
// provided buffer.  Returns the number of bytes read.

size_t wad_read(wad_file_t *wad, unsigned int offset,
                   void *buffer, size_t buffer_len)
{
    fprintf(stderr, "wad_read %p: offset=%u len=%lu\n", wad, offset, buffer_len);
    char* buf = (char*) buffer;
    const size_t n = MIN(offset + buffer_len, wad->length);
    for (unsigned int i=offset; i<n; ++i) {
        buf[i - offset] = wad->mapped[i];
    }
    return (n >= offset) ? (n - offset) : MIN(n, buffer_len);
}


wad_file_class_t static_wad_file =
{
    wad_open,
    wad_close,
    wad_read,
};
