/* Project 16 Source Code~
 * Copyright (C) 2012-2016 sparky4 & pngwen & andrius4669 & joncampbell123
 *
 * This file is part of Project 16.
 *
 * Project 16 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Project 16 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>, or
 * write to the Free Software Foundation, Inc., 51 Franklin Street,
 * Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef __BAKAPEE_H_
#define __BAKAPEE_H_

/* Project 16 Source Code~
 * Copyright (C) 2012-2016 sparky4 & pngwen & andrius4669 & joncampbell123 & yakui-lover
 *
 * This file is part of Project 16.
 *
 * Project 16 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Project 16 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>, or
 * write to the Free Software Foundation, Inc., 51 Franklin Street,
 * Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include "src/lib/16_head.h"
#include "src/lib/modex16.h"
#include <hw/cpu/cpu.h>
#include <hw/dos/dos.h>
#include <hw/vga/vga.h>

//#include "src/lib/modex/modex.h"
//#include "16/x/modex.h"

#define TILEWH 16
#define QUADWH			TILEWH/2
#define BONK	    400
#define LGQ	     32
#define HGQ	     55

#define TILE

//which lib?
#define MXLIB

typedef struct {
	word gq;
	sword bakax, bakay;
	sword xx, yy, sx, sy;
	byte coor;
	boolean tile;
} bakapee_t;

void clrstdin();
void colortest(page_t *page, bakapee_t *pee);
void colorz(page_t *page, bakapee_t *pee);
void ssd(page_t *page, bakapee_t *pee, word svq);
void dingpp(page_t *page, bakapee_t *pee);
void dingo(page_t *page, bakapee_t *pee);
void dingas(bakapee_t *pee);
void dingu(bakapee_t *pee);
void dingq(bakapee_t *pee);
void ding(page_t *page, bakapee_t *pee, word q);

#endif /*__BAKAPEE_H_*/
