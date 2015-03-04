// Cut this out as emm.c

/*      File:       emm.c
 *      Module:     All Modules
 *      Author(s):  Chris Somers
 *      Date:       December 1, 1992
 *      Version:    V.1.1

        minor mods by Alex Russell to simplify

        Must use memory model with FAR code

 */


#include <stdio.h>
#include <stdlib.h>
#include <mem.h>

#include "memory.h"

void TransformData(char *pEmmData, unsigned int len)
{
   while ( len )
      {
      (*pEmmData)++;
      pEmmData++;

      len--;
      }
}

void main(void)
{
   char    *pEmmData;
   int     hEData;

   if ( OpenEMM() != SUCCESS )
      {     // make sure we got EMM
      printf("EMM unavailable.\n");
      exit(1);
      }
   else
      printf("Emm available\n");

   pEmmData = (char *)EMMalloc(&hEData, 6);  // get 6 * 16K bytes - 96K
   if ( pEmmData == NULL )
      {
      printf("Not enough EMM or out of handles.\n");
      exit(2);
      }
   else
      printf("emm alloced OK\n");


   printf("Map 1st 4 pages\n");
   MapEMM(hEData, 0, 4);   // load 1st 4 pages into page frame: 0-3

   memset(pEmmData, 0x0e, 64000u);
   UnmapEMM(hEData, 0, 4);          // not absolutely necessary
   
   printf("Map next 2 pages\n");
   MapEMM(hEData, 4, 2);            // map last 2 pages: 4-5
   memset(pEmmData, 0x0e, 32768u);

   MapEMM(hEData, 0, 4);
   // do some stuff with the first 64K of file data.
   printf("Transform data\n");
   TransformData(pEmmData, 64000UL);
   MapEMM(hEData, 4, 2);  // only unmaps 1st two pages of prior 64k mapping
   // do stuff with remaining 32K of data
   TransformData(pEmmData, 32768UL);
   UnmapEMM(hEData, 0, 4);  // should unmap before freeing

   printf("Close emm\n");
   EMMFree(hEData);     // finished with the file data
   CloseEMM();
}