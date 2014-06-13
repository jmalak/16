#ifndef _DOSLIB_GFX_H_
#define _DOSLIB_GFX_H_
#include <iostream.h>
#include <stdlib.h>
#include "lib\lib_com.h"
#include "lib\x\modex.h"

#define NUM_COLORS	256 // number of colors in vga mode
//static lgq=NUM_COLORS/(1/8)
//static hgq=NUM_COLORS/(1/8)
#define BONK	    400
#define LGQ	     32
#define HGQ	     56
#define TILEWH			16
#define QUADWH			TILEWH/2
#define BUFFMX			3
/*byte SIZE = 80;	// Size =  40 = 1 across, 4 down
		     // Size =  80 = 2 across, 2 down
		     // Size = 160 = 4 across, 1 down*/
//#define VMEM	    0xA000 // = vga

#define SW	320
#define SH	240
//#define VW	560
//#define VH	416
#define VW	(SW+64)
#define VH	(SH+64)
#define BS (VH*2) // buffer space! not BULLSHIT
#define BH	BS+(TILEWH*BUFFMX) // buffer resolution

//void drawChar(int x, int y, int color, byte c);
//void drawText(int x, int y, int color, byte string);
void setvideo(/*byte mode, */short vq);
//void setvbuff(int vq);
//void updatevbuff();
void cls(byte color, byte *Where);
//void clearscr();
//void plotpixel(int x, int y, byte color, byte *Where);
//void plotpixelfast(int x, int y, byte color, byte *Where);
//void putPixel_X(int x, int y, byte color);
//void putColorBox_X(int x, int y, int w, int h, byte color);
//void vScroll(int rows);
//void scrolly(int bong);
//void wait_for_retrace(void); // Waits for vertical retrace
//void BlockMove();
//void eraseplayer(int x, int y);
//void drawplayer(int x, int y, int color);
short colortest();
short colorz();
void ssd(short svq);
//void pdump(int mult);
//void tulip();
short ding(int q);

#endif/*_DOSLIB_GFX_H_*/
