#include <stdio.h>

#include "m_argv.h"

#include "doomgeneric.h"

uint8_t* DG_ScreenBuffer = 0;
uint8_t* CompressedScreenBuffer = 0;
int num_frames_written = 0;

void M_FindResponseFile(void);
void D_DoomMain (void);


void doomgeneric_Create(int argc, char **argv)
{
	// save arguments
    myargc = argc;
    myargv = argv;

	M_FindResponseFile();

    const int n = PAYLOAD_LEN;
	DG_ScreenBuffer = malloc(n);
	CompressedScreenBuffer = malloc(COMPRESSOR_BUF_LEN);

	DG_Init();

	D_DoomMain ();
}

