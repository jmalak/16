#include <hw/cpu/cpu.h>
#include <hw/dos/dos.h>
#include <hw/vga/vga.h>
#include <hw/vga/vrl.h>

#include "src/tesuto.h"

#define FILENAME_1 "data/aconita.vrl"
#define FILENAME_2 "data/aconita.pal"

static unsigned char palette[768];
global_game_variables_t gvar;
player_t player[1];

int main(int argc,char **argv) {
	struct vrl1_vgax_header *vrl_header;
	vrl1_vgax_offset_t *vrl_lineoffs;
	unsigned char *buffer;
	unsigned int bufsz;
	int fd;
	char *bakapee1,*bakapee2;

	bakapee1=malloc(64);
	bakapee2=malloc(1024);

	IN_Startup();
	IN_Default(0,&player,ctrl_Joystick);
	IN_initplayer(&player, 0);

	if (argc < 3) {
		fprintf(stderr,"drawvrl <VRL file> <palette file>\n");
		bakapee1 = FILENAME_1;//"data/aconita.vrl";
		bakapee2 = FILENAME_2;//"data/aconita.pal";
		//return 1;
	}else{
		if(argv[1]) bakapee1 = argv[1];
		if(argv[2]) bakapee2 = argv[2];
	}

	fd = open(bakapee1,O_RDONLY|O_BINARY);
	if (fd < 0) {
		fprintf(stderr,"Unable to open '%s'\n", bakapee1);
		return 1;
	}
	{
		unsigned long sz = lseek(fd,0,SEEK_END);
		if (sz < sizeof(*vrl_header)) return 1;
		if (sz >= 65535UL) return 1;

		bufsz = (unsigned int)sz;
		buffer = malloc(bufsz);
		if (buffer == NULL) return 1;

		lseek(fd,0,SEEK_SET);
		if ((unsigned int)read(fd,buffer,bufsz) < bufsz) return 1;

		vrl_header = (struct vrl1_vgax_header*)buffer;
		if (memcmp(vrl_header->vrl_sig,"VRL1",4) || memcmp(vrl_header->fmt_sig,"VGAX",4)) return 1;
		if (vrl_header->width == 0 || vrl_header->height == 0) return 1;
	}
	close(fd);

	probe_dos();
	if (!probe_vga()) {
		printf("VGA probe failed\n");
		return 1;
	}
	VGAmodeX(1, 1, &gvar);

	/* load color palette */
	fd = open(bakapee2,O_RDONLY|O_BINARY);
	if (fd >= 0) {
		unsigned int i;

		read(fd,palette,768);
		close(fd);

		vga_palette_lseek(0);
		for (i=0;i < 256;i++) vga_palette_write(palette[(i*3)+0]>>2,palette[(i*3)+1]>>2,palette[(i*3)+2]>>2);
	}

	/* preprocess the sprite to generate line offsets */
	vrl_lineoffs = vrl1_vgax_genlineoffsets(vrl_header,buffer+sizeof(*vrl_header),bufsz-sizeof(*vrl_header));
	if (vrl_lineoffs == NULL) return 1;

	/* setup camera and screen~ */
	modexHiganbanaPageSetup(&gvar.video);
	gvar.video.page[1].dx=gvar.video.page[0].dx=16;
	gvar.video.page[1].dy=gvar.video.page[0].dy=16;
	modexShowPage(&(gvar.video.page[0]));

	#define VMEMHEIGHT gvar.video.page[0].height+gvar.video.page[1].height

	//4	this dose the screen
	{
		unsigned int i,j,o;
		/* fill screen with a distinctive pattern */
		for (i=0;i < gvar.video.page[0].width;i++) {
			o = i >> 2;
			vga_write_sequencer(0x02/*map mask*/,1 << (i&3));
			for (j=0;j < VMEMHEIGHT;j++,o += gvar.video.page[0].stridew)
				vga_state.vga_graphics_ram[o] = (i^j)&15; // VRL samples put all colors in first 15!
		}
	}

	//while (getch() != 13);

	/* make distinctive pattern offscreen, render sprite, copy onscreen.
	 * this time, we render the distinctive pattern to another offscreen location and just copy.
	 * note this version is much faster too! */
	{
		unsigned int i,j,o,o2;
		int x,y,rx,ry,w,h;
		unsigned int overdraw = 1;	// how many pixels to "overdraw" so that moving sprites with edge pixels don't leave streaks.
						// if the sprite's edge pixels are clear anyway, you can set this to 0.
		VGA_RAM_PTR omemptr;
		int xdir=1,ydir=1;

		//4	this dose the sprite? wwww
		/* fill pattern offset with a distinctive pattern */
		for (i=0;i < gvar.video.page[0].width;i++) {
			o = (i >> 2) + (0x10000UL - (uint16_t)gvar.video.page[1].data);
			vga_write_sequencer(0x02/*map mask*/,1 << (i&3));
			for (j=0;j < VMEMHEIGHT;j++,o += gvar.video.page[0].stridew)
				vga_state.vga_graphics_ram[o] = (i^j)&15; // VRL samples put all colors in first 15!
		}

		/* starting coords. note: this technique is limited to x coordinates of multiple of 4 */
		x = 0;
		y = 0;

		/* do it */
		omemptr = vga_state.vga_graphics_ram; // save original mem ptr

		while(!IN_KeyDown(sc_Escape))
		{
			IN_ReadControl(0,&player);
			if(IN_KeyDown(2)) modexShowPage(&(gvar.video.page[0]));
			if(IN_KeyDown(3)) modexShowPage(&(gvar.video.page[1]));
			if(IN_KeyDown(68))	//f10
			{
				//gvar.kurokku.fpscap=!gvar.kurokku.fpscap;
				IN_UserInput(1,1);
			}

			/* render box bounds. y does not need modification, but x and width must be multiple of 4 */
			if (x >= overdraw) rx = (x - overdraw) & (~3);
			else rx = -(gvar.video.page[0].dx);
			if (y >= overdraw) ry = (y - overdraw);
			else ry = -(gvar.video.page[0].dy);
			h = vrl_header->height + overdraw + y - ry;
			w = (x + vrl_header->width + (overdraw*2) + 3/*round up*/ - rx) & (~3);
			if ((rx+w) > gvar.video.page[0].width) w = gvar.video.page[0].width-rx;
			if ((ry+h) > gvar.video.page[0].height) h = (gvar.video.page[0].height)-ry;

			/* block copy pattern to where we will draw the sprite */
			vga_setup_wm1_block_copy();
			o2 = gvar.video.page[0].pagesize;
			o = (0x10000UL - (uint16_t)gvar.video.page[1].data) + (ry * gvar.video.page[0].stridew) + (rx >> 2); // source offscreen
			for (i=0;i < h;i++,o += gvar.video.page[0].stridew,o2 += (w >> 2)) vga_wm1_mem_block_copy(o2,o,w >> 2);
			/* must restore Write Mode 0/Read Mode 0 for this code to continue drawing normally */
			vga_restore_rm0wm0();

			/* replace VGA stride with our own and mem ptr. then sprite rendering at this stage is just (0,0) */
			vga_state.vga_draw_stride_limit = (gvar.video.page[0].width + 3/*round up*/ - x) >> 2;
			vga_state.vga_draw_stride = w >> 2;
			vga_state.vga_graphics_ram = omemptr + gvar.video.page[0].pagesize;

			/* then the sprite. note modding ram ptr means we just draw to (x&3,0) */
			draw_vrl1_vgax_modex(x-rx,y-ry,vrl_header,vrl_lineoffs,buffer+sizeof(*vrl_header),bufsz-sizeof(*vrl_header));

			/* restore ptr */
			vga_state.vga_graphics_ram = omemptr;

			/* block copy to visible RAM from offscreen */
			vga_setup_wm1_block_copy();
			o = gvar.video.page[0].pagesize; // source offscreen
			o2 = (ry * gvar.video.page[0].stridew) + (rx >> 2); // dest visible (original stride)
			for (i=0;i < h;i++,o += vga_state.vga_draw_stride,o2 += gvar.video.page[0].stridew) vga_wm1_mem_block_copy(o2,o,w >> 2);
			/* must restore Write Mode 0/Read Mode 0 for this code to continue drawing normally */
			vga_restore_rm0wm0();

			/* restore stride */
			vga_state.vga_draw_stride_limit = vga_state.vga_draw_stride = gvar.video.page[0].stridew;

			/* step */
			x += xdir; y += ydir;
			if ((x + vrl_header->width) >= ((gvar.video.page[0].width + gvar.video.page[0].dx) - 1) || x == -(gvar.video.page[0].dx))
				xdir = -xdir;
			if ((y + vrl_header->height) >= ((gvar.video.page[0].height + gvar.video.page[0].dy) - 1) || y == -(gvar.video.page[0].dy))
				ydir = -ydir;
			//printf("[x%u y%u]	[rx%u ry%u]		[w%u h%u]\n", x, y, rx, ry, w, h);
		}
	}

	IN_UserInput(1,1);

	while(!IN_KeyDown(sc_Escape))
	{
		if(IN_KeyDown(2)) modexShowPage(&(gvar.video.page[0]));
		if(IN_KeyDown(3)) modexShowPage(&(gvar.video.page[1]));
	}

	modexShowPage(&(gvar.video.page[0]));
	/* another handy "demo" effect using VGA write mode 1.
	 * we can take what's on screen and vertically squash it like an old analog TV set turning off. */
	{
		unsigned int blank_line_ofs = (gvar.video.page[0].stridew * gvar.video.page[0].height * 2);
		unsigned int copy_ofs = (gvar.video.page[0].stridew * gvar.video.page[0].height);
		unsigned int display_ofs = 0x0000;
		unsigned int i,y,soh,doh,dstart;
		unsigned int dh_blankfill = 8;
		unsigned int dh_step = 8;
		uint32_t sh,dh,yf,ystep;

		/* copy active display (0) to offscreen buffer (0x4000) */
		vga_state.vga_draw_stride_limit = vga_state.vga_draw_stride = gvar.video.page[0].stridew;
		vga_setup_wm1_block_copy();
		vga_wm1_mem_block_copy(copy_ofs,display_ofs,gvar.video.page[0].stridew * gvar.video.page[0].height);
		vga_restore_rm0wm0();

		/* need a blank line as well */
		for (i=0;i < gvar.video.page[0].stridew;i++) vga_state.vga_graphics_ram[i+blank_line_ofs] = 0;

		sh = dh = gvar.video.page[0].height;
		while (dh >= dh_step) {
			/* stop animating if the user hits ENTER */
			if (kbhit()) {
				if (getch() == 13) break;
			}

			/* wait for vsync end */
			vga_wait_for_vsync_end();

			/* what scalefactor to use for stretching? */
			ystep = (0x10000UL * sh) / dh;
			dstart = (gvar.video.page[0].height - dh) / 2; // center the squash effect on screen, otherwise it would squash to top of screen
			doh = display_ofs;
			soh = copy_ofs;
			yf = 0;
			y = 0;

			/* for performance, keep VGA in write mode 1 the entire render */
			vga_setup_wm1_block_copy();

			/* blank lines */
			if (dstart >= dh_blankfill) y = dstart - dh_blankfill;
			else y = 0;
			doh = gvar.video.page[0].stridew * y;

			while (y < dstart) {
				vga_wm1_mem_block_copy(doh,blank_line_ofs,gvar.video.page[0].stridew);
				doh += gvar.video.page[0].stridew;
				y++;
			}

			/* draw */
			while (y < (dh+dstart)) {
				soh = copy_ofs + ((yf >> 16UL) * gvar.video.page[0].stridew);
				vga_wm1_mem_block_copy(doh,soh,gvar.video.page[0].stridew);
				doh += gvar.video.page[0].stridew;
				yf += ystep;
				y++;
			}

			/* blank lines */
			while (y < gvar.video.page[0].height && y < (dh+dstart+dh_blankfill)) {
				vga_wm1_mem_block_copy(doh,blank_line_ofs,gvar.video.page[0].stridew);
				doh += gvar.video.page[0].stridew;
				y++;
			}

			/* done */
			vga_restore_rm0wm0();

			/* wait for vsync */
			vga_wait_for_vsync();

			/* make it shrink */
			dh -= dh_step;
			if (dh < 40) dh_step = 1;
		}
	}

	IN_Shutdown();
	VGAmodeX(0, 1, &gvar);
	free(vrl_lineoffs);
	buffer = NULL;
	free(buffer);
	bufsz = 0;
	free(bakapee1);
	free(bakapee2);
	return 0;
}
