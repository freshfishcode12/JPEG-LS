/* rgbTileProc.cpp
*  implementation of TILE COMPRESSION
*/
#include "rgbTileProc.h"
#include "include.h"
#include "math.h"
static int g_nTileWidth = 0;
static int g_nTileHeight = 0;

void tileSetSize(int nTileWidth, int nTileHeight)
{
	g_nTileWidth = nTileWidth;
	g_nTileHeight = nTileHeight;
}

/* compress ARGB data to tile
*  param:
*    pClrBlk      -- IN, pixel's ARGB data
*    pTile        -- OUT, tile data
*    pTileSize    -- OUT, tile's bytes
*  return:
*    0  -- succeed
*   -1  -- failed
*/
int argb2tile(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize)
{
	//assert(g_nTileWidth > 0 && g_nTileHeight > 0);
    char*** out;
    char*** yuv;
    char** dr;
    char** dg;
    char** db;
    char** da;
    int index;
    int bitstream_size;
    out = Allocate3D(8, 8);
    dr = Allocate2D(8, 8);
    dg = Allocate2D(8, 8);
    db = Allocate2D(8, 8);
    da = Allocate2D(8, 8);
    //yuv = Allocate3D(8, 8);


    for(int i =0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            index = (i*8+j)*4;
            db[i][j] = pClrBlk[index];
            dg[i][j] = pClrBlk[index+1];
            dr[i][j] = pClrBlk[index+2];
            da[i][j] = pClrBlk[index+3];

        }
    }

    out[0] = dr;
    out[1] = dg;
    out[2] = db;
    out[3] = da;
    ModifyInputData(out,8,8);
    EncodeColorLineIlv(out, pTile,8, 8, &bitstream_size);
    if(bitstream_size >= 256) {  //将块原长度跟压缩后进行对比，如果反向压缩则保持原来数据
        for(int i = 0;i<256;i++){pTile[i] = pClrBlk[i];}
        *pTileSize = 256;
    }
    else {
        *pTileSize = bitstream_size; //需要改为压缩后每个块的长度
    }
	return 0;
}

/* decompress tile data to ARGB
*  param:
*    pTile        -- IN, tile data
*    pTileSize    -- IN, tile's bytes
*    pClrBlk      -- OUT, pixel's ARGB data
*  return:
*    0  -- succeed
*   -1  -- failed
*/
int tile2argb(char* pTile, int nTileSize, unsigned char* pClrBlk)
{
    if(nTileSize == 256){  //若块长度是256，则未被压缩，不需解压
        for(int i = 0;i<256;i++){pClrBlk[i] = pTile[i];}
    }
    else {
        DecodeColorLineIlv(pTile, pClrBlk, 8, 8, nTileSize);
    }

	return 0;
}


int argb2tile16(const unsigned char *pClrBlk, unsigned char *pTile, int *pTileSize)
{
    //assert(g_nTileWidth > 0 && g_nTileHeight > 0);
    char*** out;
    char** dr;
    char** dg;
    char** db;
    char** da;
    int index;
    int bitstream_size;
    out = Allocate3D(16, 16);
    dr = Allocate2D(16, 16);
    dg = Allocate2D(16, 16);
    db = Allocate2D(16, 16);
    da = Allocate2D(16, 16);

    for(int i =0;i<16;i++)
    {
        for(int j=0;j<16;j++)
        {
            index = (i*16+j)*4;
            db[i][j] = pClrBlk[index];
            dg[i][j] = pClrBlk[index+1];
            dr[i][j] = pClrBlk[index+2];
            da[i][j] = pClrBlk[index+3];

        }
    }
    out[0] = dr;
    out[1] = dg;
    out[2] = db;
    out[3] = da;
    ModifyInputData(out,16,16);
    EncodeColorLineIlv(out, pTile,16, 16, &bitstream_size);
    if(bitstream_size >= 1024) {  //将块原长度跟压缩后进行对比，如果反向压缩则保持原来数据
        for(int i = 0;i<1024;i++){pTile[i] = pClrBlk[i];}
        *pTileSize = 1024;
    }
    else {
        *pTileSize = bitstream_size; //需要改为压缩后每个块的长度
    }
    return 0;



}

int tile2argb16(char* pTile, int nTileSize, unsigned char* pClrBlk)
{
    if(nTileSize == 1024){  //若块长度是1024，则未被压缩，不需解压
        for(int i = 0;i<1024;i++){pClrBlk[i] = pTile[i];}
    }
    else {
        DecodeColorLineIlv(pTile, pClrBlk, 16, 16, nTileSize);
    }
    return 0;


}

int argb2tile4(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize)
{
    //assert(g_nTileWidth > 0 && g_nTileHeight > 0);
    char*** out;
    char** dr;
    char** dg;
    char** db;
    char** da;
    int index;
    int bitstream_size;
    out = Allocate3D(4, 4);
    dr = Allocate2D(4, 4);
    dg = Allocate2D(4, 4);
    db = Allocate2D(4, 4);
    da = Allocate2D(4, 4);

    for(int i =0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            index = (i*4+j)*4;
            db[i][j] = pClrBlk[index];
            dg[i][j] = pClrBlk[index+1];
            dr[i][j] = pClrBlk[index+2];
            da[i][j] = pClrBlk[index+3];

        }
    }
    out[0] = dr;
    out[1] = dg;
    out[2] = db;
    out[3] = da;
    ModifyInputData(out,4,4);
    EncodeColorLineIlv(out, pTile,4, 4, &bitstream_size);
    if(bitstream_size >= 64) {  //将块原长度跟压缩后进行对比，如果反向压缩则保持原来数据
        for(int i = 0;i<64;i++){pTile[i] = pClrBlk[i];}
        *pTileSize = 64;
    }
    else {
        *pTileSize = bitstream_size; //需要改为压缩后每个块的长度
    }
    return 0;
}
int tile2argb4(char* pTile, int nTileSize, unsigned char* pClrBlk)
{
    if(nTileSize == 64){  //若块长度是256，则未被压缩，不需解压
        for(int i = 0;i<64;i++){pClrBlk[i] = pTile[i];}
    }
    else {
        DecodeColorLineIlv(pTile, pClrBlk, 4, 4, nTileSize);
    }
    return 0;
}

int lineargb2tile1024(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize)
{
    //assert(g_nTileWidth > 0 && g_nTileHeight > 0);
    char*** out;
    char** dr;
    char** dg;
    char** db;
    char** da;
    int index;
    int bitstream_size;
    out = Allocate3D(256, 1);
    dr = Allocate2D(256, 1);
    dg = Allocate2D(256, 1);
    db = Allocate2D(256, 1);
    da = Allocate2D(256, 1);

    for(int i =0;i<1;i++)
    {
        for(int j=0;j<256;j++)
        {
            index = (i*256+j)*4;
            db[i][j] = pClrBlk[index];
            dg[i][j] = pClrBlk[index+1];
            dr[i][j] = pClrBlk[index+2];
            da[i][j] = pClrBlk[index+3];

        }
    }
    out[0] = dr;
    out[1] = dg;
    out[2] = db;
    out[3] = da;
    ModifyInputData(out,256,1);
    EncodeColorLineIlv(out, pTile,256, 1, &bitstream_size);
    if(bitstream_size >= 1024) {  //将块原长度跟压缩后进行对比，如果反向压缩则保持原来数据
        for(int i = 0;i<1024;i++){pTile[i] = pClrBlk[i];}
        *pTileSize = 1024;
    }
    else {
        *pTileSize = bitstream_size; //需要改为压缩后每个块的长度
    }
    //*pTileSize = bitstream_size; //闇€瑕佹敼涓哄帇缂╁悗姣忎釜鍧楃殑闀垮害
    return 0;
}
int linetile2argb1024(char* pTile, int nTileSize, unsigned char* pClrBlk)
{
    if(nTileSize == 1024){  //若块长度是256，则未被压缩，不需解压
        for(int i = 0;i<1024;i++){pClrBlk[i] = pTile[i];}
    }
    else {
        DecodeColorLineIlv(pTile, pClrBlk, 256, 1, nTileSize);
    }
    return 0;
}

int lineargb2tile512(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize)
{
    //assert(g_nTileWidth > 0 && g_nTileHeight > 0);
    char*** out;
    char** dr;
    char** dg;
    char** db;
    char** da;
    int index;
    int bitstream_size;
    out = Allocate3D(128, 1);
    dr = Allocate2D(128, 1);
    dg = Allocate2D(128, 1);
    db = Allocate2D(128, 1);
    da = Allocate2D(128, 1);

    for(int i =0;i<1;i++)
    {
        for(int j=0;j<128;j++)
        {
            index = (i*128+j)*4;
            db[i][j] = pClrBlk[index];
            dg[i][j] = pClrBlk[index+1];
            dr[i][j] = pClrBlk[index+2];
            da[i][j] = pClrBlk[index+3];

        }
    }
    out[0] = dr;
    out[1] = dg;
    out[2] = db;
    out[3] = da;
    ModifyInputData(out,128,1);
    EncodeColorLineIlv(out, pTile,128, 1, &bitstream_size);
    if(bitstream_size >= 512) {  //将块原长度跟压缩后进行对比，如果反向压缩则保持原来数据
        for(int i = 0;i<512;i++){pTile[i] = pClrBlk[i];}
        *pTileSize = 512;
    }
    else {
        *pTileSize = bitstream_size; //需要改为压缩后每个块的长度
    }
    //*pTileSize = bitstream_size; //闇€瑕佹敼涓哄帇缂╁悗姣忎釜鍧楃殑闀垮害
    return 0;
}
int linetile2argb512(char* pTile, int nTileSize, unsigned char* pClrBlk)
{
    if(nTileSize == 512){  //若块长度是256，则未被压缩，不需解压
        for(int i = 0;i<512;i++){pClrBlk[i] = pTile[i];}
    }
    else {
        DecodeColorLineIlv(pTile, pClrBlk, 128, 1, nTileSize);
    }
    return 0;
}

int lineargb2tile256(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize)
{
    //assert(g_nTileWidth > 0 && g_nTileHeight > 0);
    char*** out;
    char** dr;
    char** dg;
    char** db;
    char** da;
    int index;
    int bitstream_size;
    out = Allocate3D(64, 1);
    dr = Allocate2D(64, 1);
    dg = Allocate2D(64, 1);
    db = Allocate2D(64, 1);
    da = Allocate2D(64, 1);

    for(int i =0;i<1;i++)
    {
        for(int j=0;j<64;j++)
        {
            index = (i*64+j)*4;
            db[i][j] = pClrBlk[index];
            dg[i][j] = pClrBlk[index+1];
            dr[i][j] = pClrBlk[index+2];
            da[i][j] = pClrBlk[index+3];

        }
    }
    out[0] = dr;
    out[1] = dg;
    out[2] = db;
    out[3] = da;
    ModifyInputData(out,64,1);
    EncodeColorLineIlv(out, pTile,64, 1, &bitstream_size);
    if(bitstream_size >= 256) {  //将块原长度跟压缩后进行对比，如果反向压缩则保持原来数据
        for(int i = 0;i<256;i++){pTile[i] = pClrBlk[i];}
        *pTileSize = 256;
    }
    else {
        *pTileSize = bitstream_size; //需要改为压缩后每个块的长度
    }
    //*pTileSize = bitstream_size; //闇€瑕佹敼涓哄帇缂╁悗姣忎釜鍧楃殑闀垮害
    return 0;
}
int linetile2argb256(char* pTile, int nTileSize, unsigned char* pClrBlk)
{
    if(nTileSize == 256){  //若块长度是256，则未被压缩，不需解压
        for(int i = 0;i<256;i++){pClrBlk[i] = pTile[i];}
    }
    else {
        DecodeColorLineIlv(pTile, pClrBlk, 64, 1, nTileSize);
    }
    return 0;
}

int lineargb2tile64(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize)
{
    //assert(g_nTileWidth > 0 && g_nTileHeight > 0);
    char*** out;
    char** dr;
    char** dg;
    char** db;
    char** da;
    int index;
    int bitstream_size;
    out = Allocate3D(16, 1);
    dr = Allocate2D(16, 1);
    dg = Allocate2D(16, 1);
    db = Allocate2D(16, 1);
    da = Allocate2D(16, 1);

    for(int i =0;i<1;i++)
    {
        for(int j=0;j<16;j++)
        {
            index = (i*16+j)*4;
            db[i][j] = pClrBlk[index];
            dg[i][j] = pClrBlk[index+1];
            dr[i][j] = pClrBlk[index+2];
            da[i][j] = pClrBlk[index+3];

        }
    }
    out[0] = dr;
    out[1] = dg;
    out[2] = db;
    out[3] = da;
    ModifyInputData(out,16,1);
    EncodeColorLineIlv(out, pTile,16, 1, &bitstream_size);
    if(bitstream_size >= 64) {  //将块原长度跟压缩后进行对比，如果反向压缩则保持原来数据
        for(int i = 0;i<64;i++){pTile[i] = pClrBlk[i];}
        *pTileSize = 64;
    }
    else {
        *pTileSize = bitstream_size; //需要改为压缩后每个块的长度
    }
    //*pTileSize = bitstream_size; //闇€瑕佹敼涓哄帇缂╁悗姣忎釜鍧楃殑闀垮害
    return 0;
}
int linetile2argb64(char* pTile, int nTileSize, unsigned char* pClrBlk)
{
    if(nTileSize == 64){  //若块长度是256，则未被压缩，不需解压
        for(int i = 0;i<64;i++){pClrBlk[i] = pTile[i];}
    }
    else {
        DecodeColorLineIlv(pTile, pClrBlk, 16, 1, nTileSize);
    }
    return 0;
}

int argb2tile512(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize)
{
    //assert(g_nTileWidth > 0 && g_nTileHeight > 0);
    char*** out;
    char** dr;
    char** dg;
    char** db;
    char** da;
    int index;
    int bitstream_size;
    out = Allocate3D(16, 8);
    dr = Allocate2D(16, 8);
    dg = Allocate2D(16, 8);
    db = Allocate2D(16, 8);
    da = Allocate2D(16, 8);

    for(int i =0;i<8;i++)
    {
        for(int j=0;j<16;j++)
        {
            index = (i*16+j)*4;
            db[i][j] = pClrBlk[index];
            dg[i][j] = pClrBlk[index+1];
            dr[i][j] = pClrBlk[index+2];
            da[i][j] = pClrBlk[index+3];

        }
    }
    out[0] = dr;
    out[1] = dg;
    out[2] = db;
    out[3] = da;
    ModifyInputData(out,16,8);
    EncodeColorLineIlv(out, pTile,16, 8, &bitstream_size);
    if(bitstream_size >= 512) {  //将块原长度跟压缩后进行对比，如果反向压缩则保持原来数据
        for(int i = 0;i<512;i++){pTile[i] = pClrBlk[i];}
        *pTileSize = 512;
    }
    else {
        *pTileSize = bitstream_size; //需要改为压缩后每个块的长度
    }
    //*pTileSize = bitstream_size; //闇€瑕佹敼涓哄帇缂╁悗姣忎釜鍧楃殑闀垮害
    return 0;
}
int tile2argb512(char* pTile, int nTileSize, unsigned char* pClrBlk)
{
    if(nTileSize == 512){  //若块长度是256，则未被压缩，不需解压
        for(int i = 0;i<512;i++){pClrBlk[i] = pTile[i];}
    }
    else {
        DecodeColorLineIlv(pTile, pClrBlk, 16, 8, nTileSize);
    }
    return 0;
}