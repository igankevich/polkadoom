#include <stdio.h>

#include "m_argv.h"

#include "doomgeneric.h"

uint8_t* DG_ScreenBuffer = 0;
uint8_t* CompressedScreenBuffer = 0;
mz_stream ScreenBufferStream = {0};
int num_frames_written = 0;

void M_FindResponseFile(void);
void D_DoomMain (void);


void doomgeneric_Create(int argc, char **argv)
{
	// save arguments
    myargc = argc;
    myargv = argv;

	M_FindResponseFile();

    const int n = PALETTE_PNG_LEN + DOOMGENERIC_RESX * DOOMGENERIC_RESY;
	DG_ScreenBuffer = malloc(n);
	CompressedScreenBuffer = malloc(COMPRESSOR_BUF_LEN);

    memset(&ScreenBufferStream, 0, sizeof(mz_stream));
    ScreenBufferStream.next_out = CompressedScreenBuffer;
    ScreenBufferStream.avail_out = COMPRESSOR_BUF_LEN;
    mz_deflateInit(&ScreenBufferStream, MZ_DEFAULT_COMPRESSION);

	DG_Init();

	D_DoomMain ();
}

