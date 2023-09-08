#include "include.h"
#include "globals.h"
/**********************************************************************
	Allocate3D		:	This function allocates memory for 3d array
						of char's.
						width - width of matrix
						height - height of matrix
						return NULL if cannot allocate memory
						return pointer to 2d tables of pointers (3d matrix)
**********************************************************************/
char*** Allocate3D(int width, int height)
{
	char*** out_data;
	out_data = (char***)malloc(4 * sizeof(char**));
	char** red_data = Allocate2D(width, height);
	char** green_data = Allocate2D(width, height);
	char** blue_data = Allocate2D(width, height);
    char** alpha_data = Allocate2D(width, height);
	out_data[0] = red_data;
	out_data[1] = green_data;
	out_data[2] = blue_data;
    out_data[3] = alpha_data;
	return out_data;
}

/**********************************************************************
	Allocate2D		:	This function allocates memory for 2d array
						of char's.
						width - width of matrix
						height - height of matrix
						return NULL if cannot allocate memory
						return pointer to tables of pointers (2d matrix)
**********************************************************************/
char** Allocate2D(int width, int height)
{
	int i;
  	char **prow, *pdata;
  	/* allocate and clean memory for all emenents */
  	pdata = (char*)calloc(width * height, sizeof(char));
    /* allocate memory for pointers to rows */    
  	prow = (char**)malloc(height * sizeof(char*));
    /* assign to each row pointer, pointer to data */
  	for (i = 0; i < height ; i++)
    {
      	prow[i] = pdata;       
      	pdata += sizeof(char) * width;    
    }
    return prow;             
}
/**********************************************************************
	Allocate1D		:	This function allocates and clear
						memory for 1d array of char's.
						lenght - lenght of vector
						return NULL if cannot allocate memory
						return pointer to tables of chars (1d array)
**********************************************************************/
char* Allocate1D(int lenght)
{
	char* data;
	/* allocate and clean memory for all elements */
	data = (char*)calloc(lenght,sizeof(char));
	return data; 
}

/**********************************************************************
	Free2D			:	This function free memory allocated for 2d
						char's array
						data - pointer to table of pointers
**********************************************************************/
void Free2D(char** data)
{
	free(*data);
	free(data);
	data = NULL;
}

/**********************************************************************
	Free1D			:	This function free memory allocated for 1d
						char's array
						data - pointer to table 
**********************************************************************/
void Free1D(char* data)
{
	free(data);
	data = NULL;
}

/**********************************************************************
	Free3D			:	This function free memory allocated for 3d
						char's array
						data - pointer to table of pointers
**********************************************************************/
void Free3D(char*** data)
{
    Free2D(data[0]);
    Free2D(data[1]);
    Free2D(data[2]);
    Free2D(data[3]);
    free(data);
    data = NULL;
}

/**********************************************************************
	ModifyInputData	:	This function modify input 3D data, pixel R stays R,
						pixel G = R-G and pixel B = R-B
						data - pointer to table 
						w - width
						h - height
**********************************************************************/
void ModifyInputData(char*** data, int w, int h)
{
	int i, j;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
//            data[0][i][j] = (unsigned char)data[1][i][j] - (unsigned char)data[0][i][j];//G-R
//            data[2][i][j] = (unsigned char)data[1][i][j] - (unsigned char)data[2][i][j];//G-B
            data[2][i][j] = (unsigned char)data[1][i][j] - (unsigned char)data[2][i][j];//DB = G - B
            data[1][i][j] = (unsigned char)data[0][i][j] - (unsigned char)data[1][i][j];//DG = R-G

		}
	}
}

/**********************************************************************
	DemodifyInputData:	This function modify input 3D data, pixel R stays R,
						pixel G = R-G and pixel B = R-B
						data - pointer to table 
						w - width
						h - height
**********************************************************************/
void DemodifyInputData(char* data, int w, int h)
{
	int i, j;
	int index;

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			index = (i*w+j)*4;
//            data[index+2] = (unsigned char)data[index+1] -  (unsigned char)data[index+2];// R = G - R
//            data[index] = (unsigned char)data[index+1] -  (unsigned char)data[index]; //  B = G - B

            data[index+1] = (unsigned char)data[index+2] -  (unsigned char)data[index+1];//G = R - DG
            data[index] = (unsigned char)data[index+1] -  (unsigned char)data[index]; //B = G - B

		}
	}
}

