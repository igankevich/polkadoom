#include <stdio.h>

#include "m_argv.h"

#include "doomgeneric.h"

uint32_t* DG_ScreenBuffer = 0;

void M_FindResponseFile(void);
void D_DoomMain (void);


void doomgeneric_Create(int argc, char **argv)
{
	// save arguments
    myargc = argc;
    myargv = argv;

	M_FindResponseFile();

	DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY);
    const int n = DOOMGENERIC_RESX * DOOMGENERIC_RESY;
    for (int i=0; i<n; ++i) {
        DG_ScreenBuffer[i] = 0;
    }

	DG_Init();

	D_DoomMain ();
}

