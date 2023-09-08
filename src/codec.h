//
// Created by XuHuaiLin on 2023/5/27.
//

#ifndef JPEG_LS_OFFICIAL_CODEC_H
#define JPEG_LS_OFFICIAL_CODEC_H
#include <cstring>
#include <iostream>
#include <fstream>
#include <math.h>
#include "defines.h"
#include "rgbTileProc.h"
#include "stdio.h"


typedef struct _TileCompressionInfo {
    int tilePosition;
    int tileSize;
} TileCompressionInfo;


// compressed by 8*8
int compressARGB(char const * inFileName, char const * outFileName);
int decompressARGB(char const * compressedFileName, char const * outFileName);

// compressed by 16*16
int compressARGB16(char const * inFileName, char const * outFileName);
int decompressARGB16(char const * compressedFileName, char const * outFileName);

// compressed by 4*4
int compressARGB4(char const * inFileName, char const * outFileName);
int decompressARGB4(char const * compressedFileName, char const * outFileName);

// compressed by line
int compressARGB512(char const * inFileName, char const * outFileName);
int decompressARGB512(char const * compressedFileName, char const * outFileName);

int linecompressARGB64(char const * inFileName, char const * outFileName);
int linedecompressARGB64(char const * compressedFileName, char const * outFileName);

int linecompressARGB256(char const * inFileName, char const * outFileName);
int linedecompressARGB256(char const * compressedFileName, char const * outFileName);

int linecompressARGB512(char const * inFileName, char const * outFileName);
int linedecompressARGB512(char const * compressedFileName, char const * outFileName);

int linecompressARGB1024(char const * inFileName, char const * outFileName);
int linedecompressARGB1024(char const * compressedFileName, char const * outFileName);

// compare the original image and the decompressed image
int compareBMP(char const* file1, char const* file2);



#endif //JPEG_LS_OFFICIAL_CODEC_H
