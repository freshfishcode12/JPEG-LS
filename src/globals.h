#ifndef _GLOBALS_H_  
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include "include.h"


#define MAX(x,y) ((x > y) ? x:y)
#define MIN(x,y) ((x > y) ? y:x)
#define ABS(x) ((x < 0) ? -x:x)



#ifdef __cplusplus
extern "C" {
#endif
	char*** Allocate3D(int width, int height);
	char** Allocate2D(int width, int height);
	char* Allocate1D(int lenght);
	void Free3D(char*** data);
	void Free2D(char** data);
	void Free1D(char* data);
	void ModifyInputData(char*** data, int w, int h);
	void DemodifyInputData(char* data, int w, int h);
#ifdef __cplusplus
}
#endif
static unsigned int bit_set_mask[] = { 	0x00000001, 0x00000002, 0x00000004, 0x00000008,
								0x00000010, 0x00000020, 0x00000040, 0x00000080,
								0x00000100, 0x00000200, 0x00000400, 0x00000800,
								0x00001000, 0x00002000, 0x00004000, 0x00008000,
								0x00010000, 0x00020000, 0x00040000, 0x00080000,
								0x00100000, 0x00200000, 0x00400000, 0x00800000,
								0x01000000, 0x02000000, 0x04000000, 0x08000000,
								0x10000000, 0x20000000, 0x40000000, 0x80000000};
								
#endif


