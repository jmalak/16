/* Catacomb Apocalypse Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// ID_MM.H

#ifndef __16_EXMM__
#define __16_EXMM__

#include <string.h>
#include <malloc.h>
#include "src/lib/16_head.h"
#include "src/lib/16_hc.h"
//#include "src/lib/modex16.h"
//++++mh	#include "src/lib/16_in.h"

#ifdef __DEBUG__		// 1 == Debug/Dev  ;  0 == Production/final
#define OUT_OF_MEM_MSG	"MM_GetPtr: Out of memory!\nYou were short :%lu bytes\n"
#else
#define OUT_OF_MEM_MSG	"\npee\n"
#endif

//#define GETNEWBLOCK {if(!(mmnew=mmfree))Quit("MM_GETNEWBLOCK: No free blocks!") ;mmfree=mmfree->next;}
#define GETNEWBLOCK {if(!mm->mmfree)MML_ClearBlock(mm);mm->mmnew=mm->mmfree;mm->mmfree=mm->mmfree->next;}

#define FREEBLOCK(x) {*x->useptr=NULL;x->next=mm->mmfree;mm->mmfree=x;}

#define SAVENEARHEAP	0x200		// space to leave in data segment
#define SAVEFARHEAP	0//x400			// space to leave in far heap

#define	BUFFERSIZE		0x1000		// miscelanious, allways available buffer

//moved to typdefst
//#define MAXBLOCKS		1024



//--------

#define	EMS_INT			0x67

#define	EMS_STATUS		0x40
#define	EMS_GETFRAME	0x41
#define	EMS_GETPAGES	0x42
#define	EMS_ALLOCPAGES	0x43
#define	EMS_MAPPAGE		0x44
#define	EMS_MAPXPAGE		0x50
#define	EMS_FREEPAGES	0x45
#define	EMS_VERSION		0x46

//--------

#define	XMS_VERSION		0x00

#define	XMS_ALLOCHMA	0x01
#define	XMS_FREEHMA		0x02

#define	XMS_GENABLEA20	0x03
#define	XMS_GDISABLEA20	0x04
#define	XMS_LENABLEA20	0x05
#define	XMS_LDISABLEA20	0x06
#define	XMS_QUERYA20	0x07

#define	XMS_QUERYREE	0x08
#define	XMS_ALLOC		0x09
#define	XMS_FREE		0x0A
#define	XMS_MOVE		0x0B
#define	XMS_LOCK		0x0C
#define	XMS_UNLOCK		0x0D
#define	XMS_GETINFO		0x0E
#define	XMS_RESIZE		0x0F

#define	XMS_ALLOCUMB	0x10
#define	XMS_FREEUMB		0x11

//==========================================================================

//moved to typdefst
/*typedef struct
{
	dword	nearheap,farheap,EMSmem,XMSmem,mainmem;
} mminfotype;*/

//==========================================================================

extern	void		(* beforesort) (void);
extern	void		(* aftersort) (void);
extern void		(* XMSaddr) (void);		// far pointer to XMS driver

//==========================================================================

/*
=============================================================================

							LOCAL INFO

=============================================================================
*/

#define LOCKBIT		0x80	// if set in attributes, block cannot be moved
#define PURGEBITS	3		// 0-3 level, 0= unpurgable, 3= purge first
#define PURGEMASK	0xfffc
#define BASEATTRIBUTES	0	// unlocked, non purgable

//moved to typedefst
//#define MAXUMBS		12
#define MAPPAGES		4//mm->EMSpagesmapped

//moved to typdefst
/*typedef struct mmblockstruct
{
	word	start,length;
	//word	start;	dword length;
	word	blob;	//for data larger than 64k
	unsigned	attributes;
	memptr		*useptr;	// pointer to the segment start
	//huge struct mmblockstruct huge *next;
	struct mmblockstruct far *next;
} mmblocktype;


typedef struct
{
	memptr bufferseg;
	boolean		mmstarted, bombonerror, mmerror;
	//huge void huge	*farheap;
	void far	*farheap;
#ifdef __BORLANDC__
	void	*nearheap;
#endif
#ifdef __WATCOMC__
	void __near	*nearheap;
#endif
	//byte		EMS_status;
	unsigned	totalEMSpages,freeEMSpages,EMSpageframe,EMSpagesmapped,EMShandle;
	unsigned int EMSVer;
	word numUMBs,UMBbase[MAXUMBS];
	//dword	numUMBs,UMBbase[MAXUMBS];
	//huge mmblocktype	huge mmblocks[MAXBLOCKS],huge *mmhead,huge *mmfree,huge *mmrover,huge *mmnew;
	mmblocktype	far mmblocks[MAXBLOCKS],far *mmhead,far *mmfree,far *mmrover,far *mmnew;
} mminfo_t;*/

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/


/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

//==========================================================================

boolean MML_CheckForEMS(void);
byte MML_SetupEMS(mminfo_t *mm);
void MML_ShutdownEMS(mminfo_t *mm);
byte MM_MapEMS(mminfo_t *mm, mminfotype *mmi);
byte MM_MapXEMS(mminfo_t *mm, mminfotype *mmi);
boolean MML_CheckForXMS(mminfo_t *mm);
void MML_SetupXMS(mminfo_t *mm, mminfotype *mmi);
void MML_ShutdownXMS(mminfo_t *mm);
void MML_UseSpace(word segstart, dword seglength, mminfo_t *mm);
void MML_ClearBlock(mminfo_t *mm);

void MM_Startup(mminfo_t *mm, mminfotype *mmi);
void MM_Shutdown(mminfo_t *mm);

void MM_GetPtr(memptr *baseptr,dword size, mminfo_t *mm, mminfotype *mmi);
void MM_FreePtr(memptr *baseptr, mminfo_t *mm);
void MM_SetPurge(memptr *baseptr, int purge, mminfo_t *mm);
void MM_SetLock(memptr *baseptr, boolean locked, mminfo_t *mm);
void MM_SortMem(mminfo_t *mm);
void MM_ShowMemory(global_game_variables_t *gvar,/*page_t *page, */mminfo_t *mm);
void MM_DumpData(mminfo_t *mm);
dword MM_UnusedMemory(mminfo_t *mm);
dword MM_TotalFree(mminfo_t *mm);
void MM_Report(/*page_t *page, */mminfo_t *mm, mminfotype *mmi);
static void MM_EMSerr(byte *stri, byte err);
void MM_BombOnError(boolean bomb, mminfo_t *mm);
//void MM_GetNewBlock(mminfo_t *mm);
//void MM_FreeBlock(mmblocktype *x, mminfo_t *mm);

//==========================================================================

#endif
