#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include <stdlib.h>
#include <stdint.h>

#include "miniz.h"

#define DOOMGENERIC_RESX 320
#define DOOMGENERIC_RESY 200
#define FRAME_LEN (DOOMGENERIC_RESX*DOOMGENERIC_RESY)
#define PALETTE_PNG_LEN (256*3)
#define PAYLOAD_LEN (1 + PALETTE_PNG_LEN + FRAME_LEN)
#define COMPRESSOR_BUF_LEN (10 * DOOMGENERIC_RESX * DOOMGENERIC_RESY)


extern uint8_t* DG_ScreenBuffer;
extern uint8_t* CompressedScreenBuffer;
extern mz_stream ScreenBufferStream;
extern int num_frames_written;

void doomgeneric_Create(int argc, char **argv);
void doomgeneric_Tick();


//Implement below functions for your platform
void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char * title);

#endif //DOOM_GENERIC
