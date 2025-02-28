// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include "config.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_video.h"
#include "z_zone.h"

#include "tables.h"
#include "doomkeys.h"

#include "doomgeneric.h"

#include <stdbool.h>
#include <stdlib.h>

#include <fcntl.h>

#include <stdarg.h>

#include <sys/types.h>

#include "miniz.h"
#include "core_vm_guest.h"
#include "stb_image_write.h"

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define PALETTE_PNG_LEN (256*3)

//#define CMAP256

struct FB_BitField
{
	uint32_t offset;			/* beginning of bitfield	*/
	uint32_t length;			/* length of bitfield		*/
};

struct FB_ScreenInfo
{
	uint32_t xres;			/* visible resolution		*/
	uint32_t yres;
	uint32_t xres_virtual;		/* virtual resolution		*/
	uint32_t yres_virtual;

	uint32_t bits_per_pixel;		/* guess what			*/
	
							/* >1 = FOURCC			*/
	struct FB_BitField red;		/* bitfield in s_Fb mem if true color, */
	struct FB_BitField green;	/* else only length is significant */
	struct FB_BitField blue;
	struct FB_BitField transp;	/* transparency			*/
};

static struct FB_ScreenInfo s_Fb;
int fb_scaling = 1; // numerator
int fb_down_scaling_x = 1; // denominator
int fb_down_scaling_y = 1; // denominator
int usemouse = 0;

struct color {
    uint32_t b:8;
    uint32_t g:8;
    uint32_t r:8;
    uint32_t a:8;
};

static struct color colors[256];

// r1 g1 b1 r2 g2 b2 ...
static uint8_t palette_png[PALETTE_PNG_LEN];

void I_GetEvent(void);

// The screen buffer; this is modified to draw things to the screen

byte *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = 2.0;
int mouse_threshold = 10;

// Gamma correction level to use

int usegamma = 0;

typedef struct
{
	byte r;
	byte g;
	byte b;
} col_t;

// Palette converted to RGB565

static uint16_t rgb565_palette[256];

void cmap_to_rgb565(uint16_t * out, uint8_t * in, int in_pixels)
{
    int i, j;
    struct color c;
    uint16_t r, g, b;

    for (i = 0; i < in_pixels; i++)
    {
        c = colors[*in]; 
        r = ((uint16_t)(c.r >> 3)) << 11;
        g = ((uint16_t)(c.g >> 2)) << 5;
        b = ((uint16_t)(c.b >> 3)) << 0;
        *out = (r | g | b);

        in++;
        for (j = 0; j < fb_scaling; j++) {
            out++;
        }
    }
}

uint32_t color_to_pixel(struct color c) {
    uint16_t r = (uint16_t)(c.r >> (8 - s_Fb.red.length));
    uint16_t g = (uint16_t)(c.g >> (8 - s_Fb.green.length));
    uint16_t b = (uint16_t)(c.b >> (8 - s_Fb.blue.length));
    uint32_t pix = r << s_Fb.red.offset;
    pix |= g << s_Fb.green.offset;
    pix |= b << s_Fb.blue.offset;
    return pix;
}

void cmap_to_fb(uint8_t * out, uint8_t * in, int in_pixels)
{
    int i, j, k;
    struct color c;
    uint32_t pix;
    uint16_t r, g, b;

    for (i = 0; i < in_pixels; i++)
    {
        c = colors[*in];  /* R:8 G:8 B:8 format! */
        pix = color_to_pixel(c);

        for (k = 0; k < fb_scaling; k++) {
            for (j = 0; j < s_Fb.bits_per_pixel/8; j++) {
                *out = (pix >> (j*8));
                out++;
            }
        }
        in++;
    }
}

#define INDEX(i, j) ((i)*SCREENWIDTH + (j))

static void cmap_to_fb_downscale(uint32_t* out, uint8_t* in) {
    int w = s_Fb.xres;
    int h = s_Fb.yres;
    for (int i=0; i<h; ++i) {
        for (int j=0; j<w; ++j) {
            int in_i = i * fb_down_scaling_y;
            int in_j = j * fb_down_scaling_x;
            int in_i1 = MIN(SCREENHEIGHT, in_i + fb_down_scaling_y);
            int in_j1 = MIN(SCREENWIDTH, in_j + fb_down_scaling_x);
            uint32_t r = 0;
            uint32_t g = 0;
            uint32_t b = 0;
            uint32_t a = 0;
            uint32_t n = 0;
            for (int y=in_i; y<in_i1; ++y) {
                for (int x=in_j; x<in_j1; ++x) {
                    struct color c2 = colors[in[INDEX(y, x)]];
                    b += c2.b;
                    g += c2.g;
                    r += c2.r;
                    a += c2.a;
                    ++n;
                }
            }
            struct color c = {
                .b = b / n,
                .g = g / n,
                .r = r / n,
                .a = a / n,
            };
            out[i*w + j] = color_to_pixel(c);
        }
    }
}

static void cmap_to_fb_downscale_v2(uint8_t* out, uint8_t* in) {
    int w = s_Fb.xres;
    int h = s_Fb.yres;
    for (int i=0; i<h; ++i) {
        for (int j=0; j<w; ++j) {
            int in_i = i * fb_down_scaling_y;
            int in_j = j * fb_down_scaling_x;
            int in_i1 = MIN(SCREENHEIGHT, in_i + fb_down_scaling_y);
            int in_j1 = MIN(SCREENWIDTH, in_j + fb_down_scaling_x);
            uint32_t sum = 0;
            uint32_t n = 0;
            for (int y=in_i; y<in_i1; ++y) {
                for (int x=in_j; x<in_j1; ++x) {
                    sum += in[INDEX(y, x)];
                    ++n;
                }
            }
            uint8_t index = (uint8_t)(sum / n);
            out[i*w + j] = index;
        }
    }
}

static void cmap_to_fb_downscale_v3(uint8_t* out, uint8_t* in) {
    int w = s_Fb.xres;
    int h = s_Fb.yres;
    for (int i=0; i<h; ++i) {
        for (int j=0; j<w; ++j) {
            int in_i = i * fb_down_scaling_y;
            int in_j = j * fb_down_scaling_x;
            int in_i1 = MIN(SCREENHEIGHT, in_i + fb_down_scaling_y);
            int in_j1 = MIN(SCREENWIDTH, in_j + fb_down_scaling_x);
            uint32_t sum = in[INDEX(in_i, in_j)];
            uint32_t n = 1;
            uint8_t index = (uint8_t)(sum / n);
            out[i*w + j] = index;
        }
    }
}

static void cmap_to_fb_copy(uint8_t* out, uint8_t* in) {
    memcpy(out, in, SCREENWIDTH * SCREENHEIGHT);
    copy_out((uint64_t) out, (uint64_t) (DOOMGENERIC_RESX * DOOMGENERIC_RESY));
}

static void copy_out_cmap(uint8_t* in) {
    copy_out((uint64_t) in, (uint64_t) (DOOMGENERIC_RESX * DOOMGENERIC_RESY));
}

#define MAX_FRAMES 4

static void cmap_to_fb_compress(uint8_t* in) {
    ScreenBufferStream.next_in = in;
    ScreenBufferStream.avail_in = DOOMGENERIC_RESX * DOOMGENERIC_RESY;
    if (num_frames_written == MAX_FRAMES - 1) {
        mz_deflate(&ScreenBufferStream, MZ_FINISH);
        copy_out((uint64_t) CompressedScreenBuffer, (uint64_t) ScreenBufferStream.total_out);
        mz_deflateReset(&ScreenBufferStream);
        ScreenBufferStream.next_out = CompressedScreenBuffer;
        ScreenBufferStream.avail_out = COMPRESSOR_BUF_LEN;
        num_frames_written = 0;
    } else {
        mz_deflate(&ScreenBufferStream, MZ_NO_FLUSH);
        num_frames_written++;
    }
}

#undef INDEX

static void copy_to_host(void* context, void* data, int size) {
    copy_out((uint64_t) data, (uint64_t) size);
}

static void cmap_to_png(uint8_t* colormap) {
    stbi_write_png_to_func(
        copy_to_host,
        NULL,
        DOOMGENERIC_RESX,
        DOOMGENERIC_RESY,
        1,
        colormap,
        0,
        palette_png,
        PALETTE_PNG_LEN
    );
}

void I_InitGraphics (void)
{
    int i;

	memset(&s_Fb, 0, sizeof(struct FB_ScreenInfo));
	s_Fb.xres = DOOMGENERIC_RESX;
	s_Fb.yres = DOOMGENERIC_RESY;
	s_Fb.xres_virtual = s_Fb.xres;
	s_Fb.yres_virtual = s_Fb.yres;
	s_Fb.bits_per_pixel = 32;

	s_Fb.blue.length = 8;
	s_Fb.green.length = 8;
	s_Fb.red.length = 8;
	s_Fb.transp.length = 8;

	s_Fb.blue.offset = 0;
	s_Fb.green.offset = 8;
	s_Fb.red.offset = 16;
	s_Fb.transp.offset = 24;
	

    printf("I_InitGraphics: framebuffer: x_res: %d, y_res: %d, x_virtual: %d, y_virtual: %d, bpp: %d\n",
            s_Fb.xres, s_Fb.yres, s_Fb.xres_virtual, s_Fb.yres_virtual, s_Fb.bits_per_pixel);

    printf("I_InitGraphics: framebuffer: RGBA: %d%d%d%d, red_off: %d, green_off: %d, blue_off: %d, transp_off: %d\n",
            s_Fb.red.length, s_Fb.green.length, s_Fb.blue.length, s_Fb.transp.length, s_Fb.red.offset, s_Fb.green.offset, s_Fb.blue.offset, s_Fb.transp.offset);

    printf("I_InitGraphics: DOOM screen size: w x h: %d x %d\n", SCREENWIDTH, SCREENHEIGHT);


    i = M_CheckParmWithArgs("-scaling", 1);
    if (i > 0) {
        i = atoi(myargv[i + 1]);
        fb_scaling = i;
        printf("I_InitGraphics: Scaling factor: %d\n", fb_scaling);
    } else {
        fb_down_scaling_x = MAX(1, SCREENWIDTH / s_Fb.xres);
        fb_down_scaling_y = MAX(1, SCREENHEIGHT / s_Fb.yres);
        fb_scaling = MAX(1, MIN(s_Fb.xres / SCREENWIDTH, s_Fb.yres / SCREENHEIGHT));
        printf(
            "I_InitGraphics: Auto-scaling ratio: x = %d / %d, y = %d / %d\n",
            fb_scaling,
            fb_down_scaling_x,
            fb_scaling,
            fb_down_scaling_y
        );
    }


    /* Allocate screen to draw to */
	I_VideoBuffer = (byte*)Z_Malloc (SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);  // For DOOM to draw on

	screenvisible = true;

    extern void I_InitInput(void);
    I_InitInput();
}

void I_ShutdownGraphics (void)
{
	Z_Free (I_VideoBuffer);
}

void I_StartFrame (void)
{

}

void I_StartTic (void)
{
	I_GetEvent();
}

void I_UpdateNoBlit (void)
{
}

//
// I_FinishUpdate
//

void I_FinishUpdate (void)
{
    int y;
    int x_offset, y_offset, x_offset_end;
    unsigned char *line_in, *line_out;

    /* Offsets in case FB is bigger than DOOM */
    /* 600 = s_Fb heigt, 200 screenheight */
    /* 600 = s_Fb heigt, 200 screenheight */
    /* 2048 =s_Fb width, 320 screenwidth */
    y_offset     = (((s_Fb.yres - (SCREENHEIGHT * fb_scaling / fb_down_scaling_y)) * s_Fb.bits_per_pixel/8)) / 2;
    x_offset     = (((s_Fb.xres - (SCREENWIDTH  * fb_scaling / fb_down_scaling_x)) * s_Fb.bits_per_pixel/8)) / 2; // XXX: siglent FB hack: /4 instead of /2, since it seems to handle the resolution in a funny way
    //x_offset     = 0;
    x_offset_end = ((s_Fb.xres - (SCREENWIDTH  * fb_scaling / fb_down_scaling_x)) * s_Fb.bits_per_pixel/8) - x_offset;

    /* DRAW SCREEN */
    line_in  = (unsigned char *) I_VideoBuffer;
    line_out = (unsigned char *) DG_ScreenBuffer;

    y = SCREENHEIGHT;

    if (fb_scaling == 1 && (fb_down_scaling_x > 1 || fb_down_scaling_y > 1)) {
        //cmap_to_fb_downscale(DG_ScreenBuffer, I_VideoBuffer);
        cmap_to_fb_downscale_v3((uint8_t*)DG_ScreenBuffer, I_VideoBuffer);
    } else if (fb_scaling == 1 && fb_down_scaling_x == 1 && fb_down_scaling_y == 1) {
        //cmap_to_fb_copy((uint8_t*)DG_ScreenBuffer, I_VideoBuffer);
        //cmap_to_fb_compress(I_VideoBuffer);
        cmap_to_png(I_VideoBuffer);
    } else {
        while (y--)
        {
            int i;
            for (i = 0; i < fb_scaling; i++) {
                line_out += x_offset;
                #ifdef CMAP256
                for (fb_scaling == 1) {
                    memcpy(line_out, line_in, SCREENWIDTH); /* fb_width is bigger than Doom SCREENWIDTH... */
                } else {
                    //XXX FIXME fb_scaling support!
                }
                #else
                //cmap_to_rgb565((void*)line_out, (void*)line_in, SCREENWIDTH);
                cmap_to_fb((void*)line_out, (void*)line_in, SCREENWIDTH);
                #endif
                line_out += (SCREENWIDTH * fb_scaling * (s_Fb.bits_per_pixel/8)) + x_offset_end;
            }
            line_in += SCREENWIDTH;
        }
    }

	DG_DrawFrame();
}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
#define GFX_RGB565(r, g, b)			((((r & 0xF8) >> 3) << 11) | (((g & 0xFC) >> 2) << 5) | ((b & 0xF8) >> 3))
#define GFX_RGB565_R(color)			((0xF800 & color) >> 11)
#define GFX_RGB565_G(color)			((0x07E0 & color) >> 5)
#define GFX_RGB565_B(color)			(0x001F & color)

void I_SetPalette (byte* palette)
{
    //for (i=0; i<256; ++i ) {
    //    colors[i].a = 0;
    //    colors[i].r = gammatable[usegamma][*palette++];
    //    colors[i].g = gammatable[usegamma][*palette++];
    //    colors[i].b = gammatable[usegamma][*palette++];
    //}
    for (int i=0; i<256; ++i) {
        uint8_t red = gammatable[usegamma][*palette++];
        uint8_t green = gammatable[usegamma][*palette++];
        uint8_t blue = gammatable[usegamma][*palette++];
        palette_png[3*i + 0] = red;
        palette_png[3*i + 1] = green;
        palette_png[3*i + 2] = blue;
    }
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex (int r, int g, int b)
{
    int best, best_diff, diff;
    int i;
    col_t color;

    printf("I_GetPaletteIndex\n");

    best = 0;
    best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
    	color.r = GFX_RGB565_R(rgb565_palette[i]);
    	color.g = GFX_RGB565_G(rgb565_palette[i]);
    	color.b = GFX_RGB565_B(rgb565_palette[i]);

        diff = (r - color.r) * (r - color.r)
             + (g - color.g) * (g - color.g)
             + (b - color.b) * (b - color.b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}

void I_BeginRead (void)
{
}

void I_EndRead (void)
{
}

void I_SetWindowTitle (char *title)
{
	DG_SetWindowTitle(title);
}

void I_GraphicsCheckCommandLine (void)
{
}

void I_SetGrabMouseCallback (grabmouse_callback_t func)
{
}

void I_EnableLoadingDisk(void)
{
}

void I_BindVideoVariables (void)
{
}

void I_DisplayFPSDots (boolean dots_on)
{
}

void I_CheckIsScreensaver (void)
{
}
