/* rgbTileProc.h
*  implementation of TILE COMPRESSION
*/
#ifndef _RGBTILEPROC_H_
#define _RGBTILEPROC_H_

void tileSetSize(int nTileWidth, int nTileHeight);

/* compress ARGB data to tile
*  param:
*    pClrBlk      -- IN, pixel's ARGB data
*    pTile        -- OUT, tile data
*    pTileSize    -- OUT, tile's bytes
*  return:
*    0  -- succeed
*   -1  -- failed
*/
int argb2tile(const unsigned char *pClrBlk, unsigned char *pTile, int *pTileSize);

/* decompress tile data to ARGB
*  param:
*    pTile        -- IN, tile data
*    pTileSize    -- IN, tile's bytes
*    pClrBlk      -- OUT, pixel's ARGB data
*  return:
*    0  -- succeed
*   -1  -- failed
*/
int tile2argb(char* pTile, int nTileSize, unsigned char* pClrBlk);

int tile2argb16(char* pTile, int nTileSize, unsigned char* pClrBlk);
int argb2tile16(const unsigned char *pClrBlk, unsigned char *pTile, int *pTileSize);

int tile2argb4(char* pTile, int nTileSize, unsigned char* pClrBlk);
int argb2tile4(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize);


int argb2tile512(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize);
int tile2argb512(char* pTile, int nTileSize, unsigned char* pClrBlk);

int lineargb2tile64(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize);
int linetile2argb64(char* pTile, int nTileSize, unsigned char* pClrBlk);

int lineargb2tile256(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize);
int linetile2argb256(char* pTile, int nTileSize, unsigned char* pClrBlk);

int lineargb2tile512(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize);
int linetile2argb512(char* pTile, int nTileSize, unsigned char* pClrBlk);

int lineargb2tile1024(const unsigned char* pClrBlk, unsigned char* pTile, int* pTileSize);
int linetile2argb1024(char* pTile, int nTileSize, unsigned char* pClrBlk);

#endif
