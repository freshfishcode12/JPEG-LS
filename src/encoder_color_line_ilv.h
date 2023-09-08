#ifndef _ENCODER_COLOR_LINE_ILV_H_  
#define _ENCODER_COLOR_LINE_ILV_H_

#include "include.h"
#include "globals.h"
typedef struct 
{				
	char* bitstream;														
	int A[367], B[365], C[365], N[367], Nn[367];
	int J[32];
	int D[COMPONENTS_COUNT];
	int Q[COMPONENTS_COUNT];
	int Ra, Rb, Rc, Rd;
	int Ix;
	int Px;
	int RANGE, MAXVAL, qbpp, bpp, LIMIT;
	int RUNindex[COMPONENTS_COUNT];
	int RUNindex_prev[COMPONENTS_COUNT];
	int RUNval, RUNcnt;
	int RItype;
	int EOline[COMPONENTS_COUNT];		
	int w, h;
	int glimit;
	int MAX_C, MIN_C;
	int RESET;
	int NEAR;
	int T1, T2, T3;
	int RT1,RT2;
	int SIGN;
	int TEMP;
	int Errval, EMErrval, MErrval;
	int q;
	int k;
	int map;
	char*** raw_data;
	char*** extended_data;
	point x[COMPONENTS_COUNT];						
} 
encoder_color_line_ilv;


#ifdef __cplusplus
extern "C" {
#endif
	void EncodeColorLineIlv(char*** data, unsigned char* pTile,int w, int h, int* size);
#ifdef __cplusplus
}
#endif
#endif

