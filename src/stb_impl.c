static void* png_compress(unsigned char *data, int data_len, int *out_len, int quality); 

#include "miniz.h"

#define STBIW_ZLIB_COMPRESS png_compress
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

// https://github.com/nothings/stb/issues/113
static void* png_compress(unsigned char *data, int data_len, int *out_len, int quality) {
    mz_ulong buflen = mz_compressBound(data_len);
    unsigned char* zlib = STBIW_MALLOC(buflen);
    int ret = mz_compress2(zlib, &buflen, data, data_len, MZ_BEST_SPEED);
    if (ret != MZ_OK) {
        return NULL;
    }
    *out_len = (int)buflen;
    return zlib;
}
