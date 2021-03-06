#include <dos.h>
#include <string.h>
#include <mem.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib\modex16.h"


byte far* VGA=(byte far*) 0xA0000000; 	/* this points to video memory. */

static void fadePalette(sbyte fade, sbyte start, word iter, byte *palette);
static byte tmppal[PAL_SIZE];
static struct pcxHeader {
    byte id;
    byte version;
    byte encoding;
    byte bpp;
    word xmin;
    word ymin;
    word xmax;
    word ymax;
    word hres;
    word vres;
    byte pal16[48];
    byte res1;
    word bpplane;
    word palType;
    word hScreenSize;
    word vScreenSize;
    byte padding[54];
};


static void
vgaSetMode(byte mode)
{
  union REGS regs;

  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}


/* -========================= Entry  Points ==========================- */
void
modexEnter() {
    word i;
    dword far*ptr=(dword far*)VGA;      /* used for faster screen clearing */
    word CRTParms[] = {
	0x0d06,		/* vertical total */
	0x3e07,		/* overflow (bit 8 of vertical counts) */
	0x4109,		/* cell height (2 to double-scan */
	0xea10,		/* v sync start */
	0xac11,		/* v sync end and protect cr0-cr7 */
	0xdf12,		/* vertical displayed */
	0x0014,		/* turn off dword mode */
	0xe715,		/* v blank start */
	0x0616,		/* v blank end */
	0xe317		/* turn on byte mode */
    };
    int CRTParmCount = sizeof(CRTParms) / sizeof(CRTParms[0]);

    /* TODO save current video mode and palette */
    vgaSetMode(VGA_256_COLOR_MODE);

    /* disable chain4 mode */
    outpw(SC_INDEX, 0x0604);

    /* synchronous reset while setting Misc Output */
    outpw(SC_INDEX, 0x0100);

    /* select 25 MHz dot clock & 60 Hz scanning rate */
    outp(MISC_OUTPUT, 0xe3);

    /* undo reset (restart sequencer) */
    outpw(SC_INDEX, 0x0300);

    /* reprogram the CRT controller */
    outp(CRTC_INDEX, 0x11); /* VSync End reg contains register write prot */
    outp(CRTC_DATA, 0x7f);  /* get current write protect on varios regs */

    /* send the CRTParms */
    for(i=0; i<CRTParmCount; i++) {
	outpw(CRTC_INDEX, CRTParms[i]);
    }

    /* clear video memory */
    outpw(SC_INDEX, 0x0f02);
    for(i=0; i<0x8000; i++) {
	ptr[i] = 0x0000;
    }
}

int old_mode;

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// setvideo() - This function Manages the video modes					  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
void setvideo(/*byte mode, */short vq){
		union REGS in, out;

		if(!vq){ // deinit the video
				// change to the video mode we were in before we switched to mode 13h
				in.h.ah = 0x00;
				in.h.al = old_mode;
				int86(0x10, &in, &out);

		}else if(vq==1){ // init the video
				// get old video mode
				in.h.ah = 0xf;
				int86(0x10, &in, &out);
				old_mode = out.h.al;
				// enter mode
				modexEnter();
		}
}

page_t
modexDefaultPage() {
    page_t page;

    /* default page values */
    page.data = VGA;
    page.dx = 0;
    page.dy = 0;
    page.width = SCREEN_WIDTH;
    page.height = SCREEN_HEIGHT;

    return page;
}

/* returns the next page in contiguous memory
 * the next page will be the same size as p, by default
 */
page_t
modexNextPage(page_t *p) {
    page_t result;

    result.data = p->data + (p->width/4)*p->height;  /* compute the offset */
    result.dx = 0;
    result.dy = 0;
    result.width = p->width;
    result.height = p->height;

    return result;
}


void
modexShowPage(page_t *page) {
    word high_address;
    word low_address;
    word offset;
    byte crtcOffset;

    /* calculate offset */
    offset = (word) page->data;
    offset += page->dy * (page->width >> 2 );
    offset += page->dx >> 2;

    /* calculate crtcOffset according to virtual width */
    crtcOffset = page->width >> 3;

    high_address = HIGH_ADDRESS | (offset & 0xff00);
    low_address  = LOW_ADDRESS  | (offset << 8);

    /* wait for appropriate timing and then program CRTC */
    while ((inp(INPUT_STATUS_1) & DISPLAY_ENABLE));
    outpw(CRTC_INDEX, high_address);
    outpw(CRTC_INDEX, low_address);
    outp(CRTC_INDEX, 0x13);
    outp(CRTC_DATA, crtcOffset);

    /*  wait for one retrace */
    while (!(inp(INPUT_STATUS_1) & VRETRACE)); 

    /* do PEL panning here */
    outp(AC_INDEX, 0x33);
    outp(AC_INDEX, (page->dx & 0x03) << 1);
}


void
modexPanPage(page_t *page, int dx, int dy) {
    page->dx = dx;
    page->dy = dy;
}


void
modexSelectPlane(byte plane) {
    outp(SC_INDEX, MAP_MASK);          /* select plane */
    outp(SC_DATA,  plane);
}


void
modexClearRegion(page_t *page, int x, int y, int w, int h, byte  color) {
    word pageOff = (word) page->data;
    word xoff=x/4;       /* xoffset that begins each row */
    word scanCount=w/4;  /* number of iterations per row (excluding right clip)*/
    word poffset = pageOff + y*(page->width/4) + xoff; /* starting offset */
    word nextRow = page->width/4-scanCount-1;  /* loc of next row */
    byte lclip[] = {0x0f, 0x0e, 0x0c, 0x08};  /* clips for rectangles not on 4s */
    byte rclip[] = {0x00, 0x01, 0x03, 0x07};
    byte left = lclip[x&0x03];
    byte right = rclip[(x+w)&0x03];

    /* handle the case which requires an extra group */
    if((x & 0x03) && !((x+w) & 0x03)) {
      right=0x0f;
    }

    __asm {
		MOV AX, SCREEN_SEG      ; go to the VGA memory
		MOV ES, AX
		MOV DI, poffset		; go to the first pixel
		MOV DX, SC_INDEX	; point to the map mask
		MOV AL, MAP_MASK
		OUT DX, AL
		INC DX
		MOV AL, color		; get ready to write colors
	SCAN_START:
		MOV CX, scanCount	; count the line
		MOV BL, AL		; remember color
		MOV AL, left		; do the left clip
		OUT DX, AL		; set the left clip
		MOV AL, BL		; restore color
		STOSB			; write the color
		DEC CX
		JZ SCAN_DONE		; handle 1 group stuff

		;-- write the main body of the scanline
		MOV BL, AL	  	; remember color
		MOV AL, 0x0f		; write to all pixels
		OUT DX, AL
		MOV AL, BL		; restore color
		REP STOSB		; write the color
	SCAN_DONE:
		MOV BL, AL		; remeber color
		MOV AL, right
		OUT DX, AL		; do the right clip
		MOV AL, BL		; restore color
		STOSB			; write pixel
		ADD DI, nextRow		; go to the next row
		DEC h
		JNZ SCAN_START
    }
}


void
modexDrawBmp(page_t *page, int x, int y, bitmap_t *bmp) {
    /* draw the region (the entire freakin bitmap) */
    modexDrawBmpRegion(page, x, y, 0, 0, bmp->width, bmp->height, bmp);
}


void
modexDrawBmpRegion(page_t *page, int x, int y,
                   int rx, int ry, int rw, int rh, bitmap_t *bmp) {
    word poffset = (word) page->data  + y*(page->width/4) + x/4;
    byte *data = bmp->data;
    word bmpOffset = (word) data + ry * bmp->width + rx;
    word width = rw;
    word height = rh;
    byte plane = 1 << ((byte) x & 0x03);
    word scanCount = width/4 + (width%4 ? 1 :0);
    word nextPageRow = page->width/4 - scanCount;
    word nextBmpRow = (word) bmp->width - width;
    word rowCounter;
    byte planeCounter = 4;

    __asm {
		MOV AX, SCREEN_SEG      ; go to the VGA memory
		MOV ES, AX

		MOV DX, SC_INDEX	; point at the map mask register
		MOV AL, MAP_MASK	;
		OUT DX, AL		;

	PLANE_LOOP:
		MOV DX, SC_DATA		; select the current plane
		MOV AL, plane		;
		OUT DX, AL		;

		;-- begin plane painting
		MOV AX, height		; start the row counter
		MOV rowCounter, AX	; 
		MOV DI, poffset		; go to the first pixel
		MOV SI, bmpOffset	; go to the bmp pixel
	ROW_LOOP:
		MOV CX, width		; count the columns
	SCAN_LOOP:
		MOVSB			; copy the pixel
		SUB CX, 3		; we skip the next 3
		ADD SI, 3		; skip the bmp pixels
		LOOP SCAN_LOOP		; finish the scan

		MOV AX, nextPageRow
		ADD DI, AX		; go to the next row on screen
		MOV AX, nextBmpRow
		ADD SI, AX		; go to the next row on bmp

		DEC rowCounter
		JNZ ROW_LOOP		; do all the rows
		;-- end plane painting

		MOV AL, plane		; advance to the next plane
		SHL AL, 1		;
		AND AL, 0x0f		; mask the plane properly
		MOV plane, AL		; store the plane

		INC bmpOffset		; start bmp at the right spot

		DEC planeCounter
		JNZ PLANE_LOOP		; do all 4 planes
    }
}


void
modexDrawSprite(page_t *page, int x, int y, bitmap_t *bmp) {
    /* draw the whole sprite */
    modexDrawSpriteRegion(page, x, y, 0, 0, bmp->width, bmp->height, bmp);
}

void
modexDrawSpriteRegion(page_t *page, int x, int y,
		      int rx, int ry, int rw, int rh, bitmap_t *bmp) {
    word poffset = (word)page->data + y*(page->width/4) + x/4;
    byte *data = bmp->data;
    word bmpOffset = (word) data + ry * bmp->width + rx;
    word width = rw;
    word height = rh;
    byte plane = 1 << ((byte) x & 0x03);
    word scanCount = width/4 + (width%4 ? 1 :0);
    word nextPageRow = page->width/4 - scanCount;
    word nextBmpRow = (word) bmp->width - width;
    word rowCounter;
    byte planeCounter = 4;

    __asm {
		MOV AX, SCREEN_SEG      ; go to the VGA memory
		MOV ES, AX

		MOV DX, SC_INDEX	; point at the map mask register
		MOV AL, MAP_MASK	;
		OUT DX, AL		;

	PLANE_LOOP:
		MOV DX, SC_DATA		; select the current plane
		MOV AL, plane		;
		OUT DX, AL		;

		;-- begin plane painting
		MOV AX, height		; start the row counter
		MOV rowCounter, AX	; 
		MOV DI, poffset		; go to the first pixel
		MOV SI, bmpOffset	; go to the bmp pixel
	ROW_LOOP:
		MOV CX, width		; count the columns
	SCAN_LOOP:
		LODSB
		DEC SI
		CMP AL, 0
		JNE DRAW_PIXEL		; draw non-zero pixels

		INC DI			; skip the transparent pixel
		ADD SI, 1
		JMP NEXT_PIXEL
	DRAW_PIXEL:
		MOVSB			; copy the pixel
	NEXT_PIXEL:
		SUB CX, 3		; we skip the next 3
		ADD SI, 3		; skip the bmp pixels
		LOOP SCAN_LOOP		; finish the scan

		MOV AX, nextPageRow
		ADD DI, AX		; go to the next row on screen
		MOV AX, nextBmpRow
		ADD SI, AX		; go to the next row on bmp

		DEC rowCounter
		JNZ ROW_LOOP		; do all the rows
		;-- end plane painting

		MOV AL, plane		; advance to the next plane
		SHL AL, 1		;
		AND AL, 0x0f		; mask the plane properly
		MOV plane, AL		; store the plane

		INC bmpOffset		; start bmp at the right spot

		DEC planeCounter
		JNZ PLANE_LOOP		; do all 4 planes
    }
}


void
modexCopyPageRegion(page_t *dest, page_t src,
		    word sx, word sy,
		    word dx, word dy,
		    word width, word height)
{
    /* todo */
}


/* fade and flash */
void
modexFadeOn(word fade, byte *palette) {
    fadePalette(-fade, 64, 64/fade+1, palette);
}


void
modexFadeOff(word fade, byte *palette) {
    fadePalette(fade, 0, 64/fade+1, palette);
}


void
modexFlashOn(word fade, byte *palette) {
    fadePalette(fade, -64, 64/fade+1, palette);
}


void
modexFlashOff(word fade, byte *palette) {
    fadePalette(-fade, 0, 64/fade+1, palette);
}


static void
fadePalette(sbyte fade, sbyte start, word iter, byte *palette) {
    word i;
    byte dim = start;

    /* handle the case where we just update */
    if(iter == 0) {
	modexPalUpdate(palette);
	return;
    }

    while(iter > 0) {  /* FadeLoop */
	for(i=0; i<PAL_SIZE; i++) { /* loadpal_loop */
	    tmppal[i] = palette[i] - dim;
	    if(tmppal[i] > 127) {
		tmppal[i] = 0;
	    } else if(tmppal[i] > 63) {
		tmppal[i] = 63;
	    }
	}
        modexPalUpdate(tmppal);
	iter--;
	dim += fade;
    }
}


/* save and load */
void
modexPalSave(byte *palette) {
    int  i;

    outp(PAL_READ_REG, 0);	/* start at palette entry 0 */
    for(i=0; i<PAL_SIZE; i++) {
	palette[i] = inp(PAL_DATA_REG); /* read the palette data */
    }
}


byte *
modexNewPal() {
    byte *ptr;
    ptr = malloc(PAL_SIZE);

    /* handle errors */
    if(!ptr) {
	printf("Could not allocate palette.\n");
	exit(-1);
    }

    return ptr;
}


void
modexLoadPalFile(byte *filename, byte **palette) {
    FILE *file;
    byte *ptr;

    /* free the palette if it exists */
    if(*palette) {
	free(*palette);
    }

    /* allocate the new palette */
    *palette = modexNewPal();

    /* open the file */
    file = fopen(filename, "rb");
    if(!file) {
	printf("Could not open palette file: %s\n", filename);
	exit(-2);
    }

    /* read the file */
    ptr = *palette;
    while(!feof(file)) {
	*ptr++ = fgetc(file);
    }

    fclose(file);
}


void
modexSavePalFile(char *filename, byte *pal) {
    unsigned int i;
    FILE *file;

    /* open the file for writing */
    file = fopen(filename, "wb");
    if(!file) {
	printf("Could not open %s for writing\n", filename);
	exit(-2);
    }

    /* write the data to the file */
    fwrite(pal, 1, PAL_SIZE, file);
    fclose(file);
}


/* blanking */
void
modexPalBlack() {
    fadePalette(-1, 64, 1, tmppal);
}


void
modexPalWhite() {
    fadePalette(-1, -64, 1, tmppal);
}


/* utility */
void
modexPalUpdate(byte *p) {
    int i;
    modexWaitBorder();
    outp(PAL_WRITE_REG, 0);  /* start at the beginning of palette */
    for(i=0; i<PAL_SIZE/2; i++) {
	outp(PAL_DATA_REG, p[i]);
    }
    modexWaitBorder();	    /* waits one retrace -- less flicker */
    for(i=PAL_SIZE/2; i<PAL_SIZE; i++) {
	outp(PAL_DATA_REG, p[i]);
    }
}


void
modexWaitBorder() {
    while(inp(INPUT_STATUS_1)  & 8)  {
	/* spin */
    }

    while(!(inp(INPUT_STATUS_1)  & 8))  {
	/* spin */
    }
}


bitmap_t
modexLoadPcx(char *filename) {
    FILE *file;
    bitmap_t result;
    struct pcxHeader head;
    long bufSize;
    int index;
    byte count, val;

    /* open the PCX file for reading */
    file = fopen(filename, "rb");
    if(!file) {
	printf("Could not open %s for reading.\n", filename);
	exit(-2);
    }

    /* read the header */
    fread(&head, sizeof(char), sizeof(struct pcxHeader), file);

    /* make sure this  is 8bpp */
    if(head.bpp != 8) {
	printf("I only know how to handle 8bpp pcx files!\n");
	fclose(file);
	exit(-2);
    }

    /* allocate the buffer */
    result.width = head.xmax - head.xmin + 1;
    result.height = head.ymax - head.ymin + 1;
    bufSize = result.width * result.height;
    result.data = malloc(bufSize);
    if(!result.data) {
	printf("Could not allocate memory for bitmap data.");
	fclose(file);
	exit(-1);
    }

    /*  read the buffer in */
    index = 0;
    do {
	/* get the run length and the value */
	count = fgetc(file);
	if(0xC0 ==  (count & 0xC0)) { /* this is the run count */
	    count &= 0x3f;
	    val = fgetc(file);
	} else {
	    val = count;
	    count = 1;
	}

	/* write the pixel the specified number of times */
	for(; count && index < bufSize; count--,index++)  {
	    result.data[index] = val;
	}
    } while(index < bufSize);

    /* handle the palette */
    fseek(file, -769, SEEK_END);
    val = fgetc(file);
    result.palette = modexNewPal();
    if(head.version == 5 && val == 12) {
	/* use the vga palette */
	for(index=0; !feof(file) && index < PAL_SIZE; index++) {
	    val = fgetc(file);
	    result.palette[index] = val >> 2;
	}
    } else {
	/* use the 16 color palette */
	for(index=0; index<48; index++) {
	    result.palette[index]  = head.pal16[index];
	}
    }

    fclose(file);

    return result;
}
