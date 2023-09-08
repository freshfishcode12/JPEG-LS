//
// Created by 13453 on 2023/5/27.
//
#include "codec.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/*
* compress ARGB data to TILE
*/
int compressARGB(char const * inFileName, char const * outFileName) {
    float ratio;
    int     ret = ERROR_OK;
    int     width, height, nrChannels;
    unsigned char *data = stbi_load(inFileName, &width, &height, &nrChannels, STBI_rgb_alpha);
    int over_cnt = 0;
    if (data == NULL) {
        std::cout << "cannot open file: " << inFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "image info: width = " << width
              << ", height = " << height
              << ", channels = " << nrChannels
              << std::endl;

    const int TILE_WIDTH = 8;
    const int TILE_HEIGHT = 8;
    int numRows = height / TILE_HEIGHT;
    int numColumns = width / TILE_WIDTH;
    const int BYTES_PER_PIXEL = 4;
    int rowStride = width * BYTES_PER_PIXEL; // 4 bytes per pixel
    unsigned char pARGB[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };
    unsigned char pCompressed[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };

    tileSetSize(TILE_WIDTH, TILE_HEIGHT);

    unsigned char * pCompressionBuffer = new unsigned char [numRows * numColumns * TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL];
    if (NULL == pCompressionBuffer) {
        return ERROR_CUSTOM;
    }
    TileCompressionInfo *pTCInfos = new TileCompressionInfo [numRows * numColumns];
    if (NULL == pTCInfos) {
        delete[] pCompressionBuffer;
        return ERROR_CUSTOM;
    }

    int tileRowIndex = 0;
    int tileColumnIndex = 0;
    int totalBitsAfterCompression = 0;
    int posInCompressionBuffer = 0;
    unsigned char* pClr = NULL;

    for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
        for (tileColumnIndex = 0; tileColumnIndex < numColumns; tileColumnIndex++) {
            int tileIndex = tileRowIndex * numColumns + tileColumnIndex;
            pClr = pARGB;
            for (int i = 0; i < TILE_HEIGHT; i++) {
                for (int j = 0; j < TILE_WIDTH; j++) {
                    int row = tileRowIndex * TILE_HEIGHT + i;
                    int col = tileColumnIndex * TILE_WIDTH + j;
                    int pixelDataOffset = rowStride * row + col * BYTES_PER_PIXEL;
                    pClr[0] = data[pixelDataOffset];     // b
                    pClr[1] = data[pixelDataOffset + 1]; // g
                    pClr[2] = data[pixelDataOffset + 2]; // r
                    pClr[3] = data[pixelDataOffset + 3]; // a
                    pClr += 4;
                }
            }
            pTCInfos[tileIndex].tilePosition = posInCompressionBuffer;

            // compress
            argb2tile(pARGB, pCompressionBuffer + posInCompressionBuffer, &pTCInfos[tileIndex].tileSize);
            posInCompressionBuffer += pTCInfos[tileIndex].tileSize;
        }
    }
    std::cout << "compression ratio = " << (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100 << "%" << std::endl;


    ratio = (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100;

    // save compressed data to JLCD file
    std::ofstream ofs;
    ofs.open(outFileName, std::ios::binary | std::ios::out);

    int tileCount = numRows * numColumns;
    int fileHeaderSize = 24;
    int tileInfoSize = tileCount * 8;
    int dataOffsetInFile = fileHeaderSize + tileInfoSize;
    if (ofs.is_open()) {
        ofs.write("JLCD", 4);
        ofs.write(reinterpret_cast<const char *>(&width), 4);
        ofs.write(reinterpret_cast<const char *>(&height), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_WIDTH), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_HEIGHT), 4);
        ofs.write(reinterpret_cast<const char *>(&tileCount), 4);
        // tile data offset + len
        for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
            for (tileColumnIndex = 0; tileColumnIndex < numColumns; tileColumnIndex++) {
                int tileIndex = tileRowIndex * numColumns + tileColumnIndex;
                ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tilePosition), 4);
                ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tileSize), 4);
            }
        }
        ofs.flush();
        // all tile data
        ofs.write(reinterpret_cast<const char *>(pCompressionBuffer), posInCompressionBuffer);
        ofs.close();
    } else {
        std::cout << "fail to open output file(" << outFileName << ")" << std::endl;
    }

    stbi_image_free(data);
    delete [] pCompressionBuffer;
    delete [] pTCInfos;
    return ERROR_OK;
}

/*
* decompress TILE data to ARGB
*/
int decompressARGB(char const * compressedFileName, char const * outFileName) {
    std::ifstream ifs;
    ifs.open(compressedFileName, std::ios::binary | std::ios::in);

    if (!ifs.is_open()) {
        std::cout << "fail to open output file: " << compressedFileName << std::endl;
        return ERROR_OUTPUT_FILE;
    }

    char readBuffer[1024];
    // read "JLCD"
    ifs.read(readBuffer, 4);
    if (strncmp(readBuffer, "JLCD", 4) != 0) {
        ifs.close();
        std::cout << "ERROR: INVALID tile file: " << compressedFileName << std::endl;
        return ERROR_INVALID_INPUT_FILE;
    }

    const int BYTES_PER_PIXEL = 4;
    int imgWidth, imgHeight, tileWidth, tileHeight, tileCount;
    // read image width
    ifs.read(reinterpret_cast<char*>(&imgWidth), 4);
    ifs.read(reinterpret_cast<char*>(&imgHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileWidth), 4);
    ifs.read(reinterpret_cast<char*>(&tileHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileCount), 4);

    if(tileWidth != 8 && tileHeight != 8){
        ifs.close();
        std::cout << "ERROR: the compressed data is not tile 8x8 compress: " << compressedFileName << std::endl;
        return ERROR_INPUT_FILE;
    }

    std::cout << "imgWidth = " << imgWidth
              << ", imgHeight = " << imgHeight
              << ", tileWidth = " << tileWidth
              << ", tileHeight = " << tileHeight
              << std::endl;

    int tileRowCount = imgHeight / tileHeight;
    int tileColumnCount = imgWidth / tileWidth;
    int tileDataStartPos = 24 + 8 * tileCount;
    unsigned char *pDecompressedARGB = new unsigned char [imgWidth * imgHeight * 4];
    unsigned char pTempDecompressionBuffer[1024];

    for (int row = 0; row < tileRowCount; row++) {
        for (int col = 0; col < tileColumnCount; col++) {
            int tileIndex = row * tileColumnCount + col;
            int tileInfoOffset = 24 + 8 * tileIndex;
            int tileDataOffset = 0;
            int tileDataBytes = 0;
            ifs.seekg(tileInfoOffset);
            ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
            ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
            ifs.seekg(tileDataStartPos + tileDataOffset);
            ifs.read(readBuffer, tileDataBytes);
            // decompress
            tile2argb(readBuffer, tileDataBytes, pTempDecompressionBuffer);

            for (int i = 0; i < tileHeight; i++) {
                for (int j = 0; j < tileWidth; j++) {
                    int globalRow = row * tileHeight + i;
                    int globalCol = col * tileWidth + j;
                    int indexInTile = i * tileWidth + j;
                    memcpy(&pDecompressedARGB[(globalRow * imgWidth + globalCol) * BYTES_PER_PIXEL],
                           &pTempDecompressionBuffer[indexInTile * BYTES_PER_PIXEL],
                           4);
                }
            }
        }
    }
    ifs.close();

    // save decompressed image to output file
    stbi_write_bmp(outFileName, imgWidth, imgHeight, STBI_rgb_alpha, reinterpret_cast<char const *>(pDecompressedARGB));

    delete[] pDecompressedARGB;
    return ERROR_OK;
}

int compressARGB16(char const * inFileName, char const * outFileName) {
    float ratio;
    int     ret = ERROR_OK;
    int     width, height, nrChannels;
    unsigned char *data = stbi_load(inFileName, &width, &height, &nrChannels, STBI_rgb_alpha);
    int over_cnt = 0;
    if (data == NULL) {
        std::cout << "cannot open file: " << inFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "image info: width = " << width
              << ", height = " << height
              << ", channels = " << nrChannels
              << std::endl;

    const int TILE_WIDTH = 16;
    const int TILE_HEIGHT = 16;
    int numRows = (height - 1) / TILE_HEIGHT + 1;
    int numColumns = (width - 1) / TILE_WIDTH + 1;
    const int BYTES_PER_PIXEL = 4;
    int rowStride = numColumns * 16 * BYTES_PER_PIXEL; // 4 bytes per pixel
    unsigned char pARGB[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };
    unsigned char pCompressed[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };

    tileSetSize(TILE_WIDTH, TILE_HEIGHT);

    unsigned char * pCompressionBuffer = new unsigned char [numRows * numColumns * TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL];
    if (NULL == pCompressionBuffer) {
        return ERROR_CUSTOM;
    }
    TileCompressionInfo *pTCInfos = new TileCompressionInfo [numRows * numColumns];
    if (NULL == pTCInfos) {
        delete[] pCompressionBuffer;
        return ERROR_CUSTOM;
    }

    int tileRowIndex = 0;
    int tileColumnIndex = 0;
    int totalBitsAfterCompression = 0;
    int posInCompressionBuffer = 0;
    unsigned char* pClr = NULL;

    //------------------------------------------拓展不能16整除的边---------------------------------------------------//
    unsigned char * extend_buffer = new unsigned char [numRows * numColumns * TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL];
    for (int h = 0; h < height; h++){
        for(int w = 0; w < width; w++){
            int buffer_index = (h*TILE_WIDTH*numColumns+w)*4;
            int data_index = (h*width+w)*4;
            extend_buffer[buffer_index] = data[data_index];
            extend_buffer[buffer_index+1] = data[data_index+1];
            extend_buffer[buffer_index+2] = data[data_index+2];
            extend_buffer[buffer_index+3] = data[data_index+3];
        }
    }

    //----------------------------------------分块--------------------------------------------//
    for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
        for (tileColumnIndex = 0; tileColumnIndex < numColumns; tileColumnIndex++) {
            int tileIndex = tileRowIndex * numColumns + tileColumnIndex;
            pClr = pARGB;
            for (int i = 0; i < TILE_HEIGHT; i++) {
                for (int j = 0; j < TILE_WIDTH; j++) {
                    int row = tileRowIndex * TILE_HEIGHT + i;
                    int col = tileColumnIndex * TILE_WIDTH + j;
                    int pixelDataOffset = rowStride * row + col * BYTES_PER_PIXEL;
                    pClr[0] = extend_buffer[pixelDataOffset];     // b
                    pClr[1] = extend_buffer[pixelDataOffset + 1]; // g
                    pClr[2] = extend_buffer[pixelDataOffset + 2]; // r
                    pClr[3] = extend_buffer[pixelDataOffset + 3]; // a
                    pClr += 4;

                }
            }
            pTCInfos[tileIndex].tilePosition = posInCompressionBuffer;

            //---------------------------------------------------------------------------------------//
            // compress
            argb2tile16(pARGB, pCompressionBuffer + posInCompressionBuffer, &pTCInfos[tileIndex].tileSize);
            posInCompressionBuffer += pTCInfos[tileIndex].tileSize;
        }
    }
    std::cout << "compression ratio = " << (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100 << "%" << std::endl;

    ratio = (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100;

    // save compressed data to JLCD file
    std::ofstream ofs;
    ofs.open(outFileName, std::ios::binary | std::ios::out);

    int tileCount = numRows * numColumns;
    int fileHeaderSize = 24;
    int tileInfoSize = tileCount * 8;
    int dataOffsetInFile = fileHeaderSize + tileInfoSize;
    if (ofs.is_open()) {
        ofs.write("JLCD", 4);
        ofs.write(reinterpret_cast<const char *>(&width), 4);
        ofs.write(reinterpret_cast<const char *>(&height), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_WIDTH), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_HEIGHT), 4);
        ofs.write(reinterpret_cast<const char *>(&tileCount), 4);
        // tile data offset + len
        for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
            for (tileColumnIndex = 0; tileColumnIndex < numColumns; tileColumnIndex++) {
                int tileIndex = tileRowIndex * numColumns + tileColumnIndex;
                ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tilePosition), 4);
                ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tileSize), 4);
            }
        }
        ofs.flush();
        // all tile data
        ofs.write(reinterpret_cast<const char *>(pCompressionBuffer), posInCompressionBuffer);
        ofs.close();
    } else {
        std::cout << "fail to open output file(" << outFileName << ")" << std::endl;
    }

    stbi_image_free(data);
    delete [] pCompressionBuffer;
    delete [] pTCInfos;
    return ERROR_OK;
}


int decompressARGB16(char const * compressedFileName, char const * outFileName) {
    std::ifstream ifs;
    ifs.open(compressedFileName, std::ios::binary | std::ios::in);

    if (!ifs.is_open()) {
        std::cout << "fail to open output file: " << compressedFileName << std::endl;
        return ERROR_OUTPUT_FILE;
    }

    char readBuffer[2048];
    // read "JLCD"
    ifs.read(readBuffer, 4);
    if (strncmp(readBuffer, "JLCD", 4) != 0) {
        ifs.close();
        std::cout << "ERROR: INVALID tile file: " << compressedFileName << std::endl;
        return ERROR_INVALID_INPUT_FILE;
    }

    const int BYTES_PER_PIXEL = 4;
    int imgWidth, imgHeight, tileWidth, tileHeight, tileCount;
    // read image width
    ifs.read(reinterpret_cast<char*>(&imgWidth), 4);
    ifs.read(reinterpret_cast<char*>(&imgHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileWidth), 4);
    ifs.read(reinterpret_cast<char*>(&tileHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileCount), 4);

    if(tileWidth != 16 && tileHeight != 16){
        ifs.close();
        std::cout << "ERROR: the compressed data is not tile 16x16 compress: " << compressedFileName << std::endl;
        return ERROR_INPUT_FILE;
    }

    std::cout << "imgWidth = " << imgWidth
              << ", imgHeight = " << imgHeight
              << ", tileWidth = " << tileWidth
              << ", tileHeight = " << tileHeight
              << std::endl;

    int tileRowCount = (imgHeight - 1) / tileHeight + 1;
    int tileColumnCount = (imgWidth - 1) / tileWidth + 1;
    int tileDataStartPos = 24 + 8 * tileCount;
    unsigned char *pDecompressedARGB = new unsigned char [tileRowCount * tileColumnCount * 16 * 16 * 4];
    unsigned char *final_data = new unsigned char [imgWidth * imgHeight * 4];
    unsigned char pTempDecompressionBuffer[2048];

    for (int row = 0; row < tileRowCount; row++) {
        for (int col = 0; col < tileColumnCount; col++) {
            int tileIndex = row * tileColumnCount + col;
            int tileInfoOffset = 24 + 8 * tileIndex;
            int tileDataOffset = 0;
            int tileDataBytes = 0;
            ifs.seekg(tileInfoOffset);
            ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
            ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
            ifs.seekg(tileDataStartPos + tileDataOffset);
            ifs.read(readBuffer, tileDataBytes);
            // decompress
            tile2argb16(readBuffer, tileDataBytes, pTempDecompressionBuffer);


            for (int i = 0; i < tileHeight; i++) {
                for (int j = 0; j < tileWidth; j++) {
                    int globalRow = row * tileHeight + i;
                    int globalCol = col * tileWidth + j;
                    int indexInTile = i * tileWidth + j;
                    memcpy(&pDecompressedARGB[(globalRow * 16*tileColumnCount + globalCol) * BYTES_PER_PIXEL],
                           &pTempDecompressionBuffer[indexInTile * BYTES_PER_PIXEL],
                           4);
                }
            }
        }
    }

    for (int h = 0; h < imgHeight; h++) {
        for (int w = 0; w < imgWidth; w++) {
            int buffer_index = (h * 16 * tileColumnCount + w) * 4;
            int data_index = (h * imgWidth + w) * 4;
            final_data[data_index] = pDecompressedARGB[buffer_index];
            final_data[data_index + 1] = pDecompressedARGB[buffer_index + 1];
            final_data[data_index + 2] = pDecompressedARGB[buffer_index + 2];
            final_data[data_index + 3] = pDecompressedARGB[buffer_index + 3];


        }
    }



    ifs.close();

    // save decompressed image to output file
    stbi_write_bmp(outFileName, imgWidth, imgHeight, STBI_rgb_alpha, reinterpret_cast<char const *>(final_data));

    delete[] pDecompressedARGB;
    return ERROR_OK;
}

/*
* compare two bmp files
*/
int compareBMP(char const* file1, char const* file2)
{
    int		ret = ERROR_OK;
    int     width, height, nrChannels;
    int     w2, h2, ch2;
    FILE *fp;
    unsigned char* data1 = stbi_load(file1, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (NULL == data1) {
        std::cout << "cannot open file1: " << file1 << std::endl;
        return ERROR_INPUT_FILE;
    }

    unsigned char* data2 = stbi_load(file2, &w2, &h2, &ch2, STBI_rgb_alpha);
    if (NULL == data2) {
        stbi_image_free(data1);
        std::cout << "cannot open file2: " << file2 << std::endl;
        return ERROR_OUTPUT_FILE;
    }
    std::cout << "file1 size: " << width << "x" << height << " (" << nrChannels << " channels)" << std::endl;
    std::cout << "file2 size: " << w2 << "x" << h2 << " (" << ch2 << " channels)" << std::endl;

    if (width != w2 || height != h2 || nrChannels != ch2) {
        std::cout << "the two files's size not equal" << std::endl;
    }
    else {
        unsigned int* pImg1 = reinterpret_cast<unsigned int*>(data1);
        unsigned int* pImg2 = reinterpret_cast<unsigned int*>(data2);
        fp=fopen("./test/err.txt","w");
        int errCnt = 0;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (*pImg1 != *pImg2) {
                    ++errCnt;
                    fprintf(fp,"%d err in i=%d j =%d  original:%d depressed: %d\n",errCnt,i,j,*pImg1,*pImg2);
                }
                ++pImg1;
                ++pImg2;
            }
        }
        if (errCnt > 0) {
            ret = ERROR_CUSTOM;
            std::cout << "there are " << errCnt << " different pixels" << std::endl;
        }
        else {
            std::cout << "the two files are same" << std::endl;
        }
    }
    fclose(fp);
    stbi_image_free(data1);
    stbi_image_free(data2);
    return ret;
}

/*
* compress ARGB data to TILE
*/
int compressARGB4(char const * inFileName, char const * outFileName) {
    float ratio;
    int     ret = ERROR_OK;
    int     width, height, nrChannels;
    unsigned char *data = stbi_load(inFileName, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data == NULL) {
        std::cout << "cannot open file: " << inFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "image info: width = " << width
                << ", height = " << height
                << ", channels = " << nrChannels
                << std::endl;
    const int TILE_WIDTH = 4;
    const int TILE_HEIGHT = 4;
    int numRows = height / TILE_HEIGHT;
    int numColumns = width / TILE_WIDTH;
    const int BYTES_PER_PIXEL = 4;
    int rowStride = width * BYTES_PER_PIXEL; // 4 bytes per pixel
    unsigned char pARGB[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };

    tileSetSize(TILE_WIDTH, TILE_HEIGHT);

    unsigned char * pCompressionBuffer = new unsigned char [numRows * numColumns * TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL];
    if (NULL == pCompressionBuffer) {
        return ERROR_CUSTOM;
    }
    TileCompressionInfo *pTCInfos = new TileCompressionInfo [numRows * numColumns];
    if (NULL == pTCInfos) {
        delete[] pCompressionBuffer;
        return ERROR_CUSTOM;
    }

    int tileRowIndex = 0;
    int tileColumnIndex = 0;
    int totalBitsAfterCompression = 0;
    int posInCompressionBuffer = 0;
    unsigned char* pClr = NULL;

    for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
        for (tileColumnIndex = 0; tileColumnIndex < numColumns; tileColumnIndex++) {
            int tileIndex = tileRowIndex * numColumns + tileColumnIndex;
            pClr = pARGB;
            for (int i = 0; i < TILE_HEIGHT; i++) {
                for (int j = 0; j < TILE_WIDTH; j++) {
                    int row = tileRowIndex * TILE_HEIGHT + i;
                    int col = tileColumnIndex * TILE_WIDTH + j;
                    int pixelDataOffset = rowStride * row + col * BYTES_PER_PIXEL;
                    pClr[0] = data[pixelDataOffset];     // b
                    pClr[1] = data[pixelDataOffset + 1]; // g
                    pClr[2] = data[pixelDataOffset + 2]; // r
                    pClr[3] = data[pixelDataOffset + 3]; // a
                    pClr += 4;
                }
            }
            pTCInfos[tileIndex].tilePosition = posInCompressionBuffer;

            // compress
            argb2tile4(pARGB, pCompressionBuffer + posInCompressionBuffer, &pTCInfos[tileIndex].tileSize);
            posInCompressionBuffer += pTCInfos[tileIndex].tileSize;
        }
    }
    std::cout << "compression ratio = " << (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100 << "%" << std::endl;

    ratio = (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100;
    // save compressed data to JLCD file
    std::ofstream ofs;
    ofs.open(outFileName, std::ios::binary | std::ios::out);

    int tileCount = numRows * numColumns;
    int fileHeaderSize = 24;
    int tileInfoSize = tileCount * 8;
    int dataOffsetInFile = fileHeaderSize + tileInfoSize;
    if (ofs.is_open()) {
        ofs.write("JLCD", 4);
        ofs.write(reinterpret_cast<const char *>(&width), 4);
        ofs.write(reinterpret_cast<const char *>(&height), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_WIDTH), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_HEIGHT), 4);
        ofs.write(reinterpret_cast<const char *>(&tileCount), 4);
        // tile data offset + len
        for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
            for (tileColumnIndex = 0; tileColumnIndex < numColumns; tileColumnIndex++) {
                int tileIndex = tileRowIndex * numColumns + tileColumnIndex;
                ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tilePosition), 4);
                ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tileSize), 4);
            }
        }
        ofs.flush();
        // all tile data
        ofs.write(reinterpret_cast<const char *>(pCompressionBuffer), posInCompressionBuffer);
        ofs.close();
    } else {
        std::cout << "fail to open output file(" << outFileName << ")" << std::endl;
    }

    stbi_image_free(data);
    delete [] pCompressionBuffer;
    delete [] pTCInfos;
    return ERROR_OK;
}

/*
* decompress TILE data to ARGB
*/
int decompressARGB4(char const * compressedFileName, char const * outFileName) {
    std::ifstream ifs;
    ifs.open(compressedFileName, std::ios::binary | std::ios::in);

    if (!ifs.is_open()) {
        std::cout << "fail to open output file: " << compressedFileName << std::endl;
        return ERROR_OUTPUT_FILE;
    }

    char readBuffer[1024];
    // read "JLCD"
    ifs.read(readBuffer, 4);
    if (strncmp(readBuffer, "JLCD", 4) != 0) {
        ifs.close();
        std::cout << "ERROR: INVALID tile file: " << compressedFileName << std::endl;
        return ERROR_INVALID_INPUT_FILE;
    }

    const int BYTES_PER_PIXEL = 4;
    int imgWidth, imgHeight, tileWidth, tileHeight, tileCount;
    // read image width
    ifs.read(reinterpret_cast<char*>(&imgWidth), 4);
    ifs.read(reinterpret_cast<char*>(&imgHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileWidth), 4);
    ifs.read(reinterpret_cast<char*>(&tileHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileCount), 4);

    if(tileWidth != 4 && tileHeight != 4){
        ifs.close();
        std::cout << "ERROR: the compressed data is not tile 4x4 compress: " << compressedFileName << std::endl;
        return ERROR_INPUT_FILE;
    }

    std::cout << "imgWidth = " << imgWidth
              << ", imgHeight = " << imgHeight
              << ", tileWidth = " << tileWidth
              << ", tileHeight = " << tileHeight
              << std::endl;

    int tileRowCount = imgHeight / tileHeight;
    int tileColumnCount = imgWidth / tileWidth;
    int tileDataStartPos = 24 + 8 * tileCount;
    unsigned char *pDecompressedARGB = new unsigned char [imgWidth * imgHeight * 4];
    unsigned char pTempDecompressionBuffer[256];

    for (int row = 0; row < tileRowCount; row++) {
        for (int col = 0; col < tileColumnCount; col++) {
            int tileIndex = row * tileColumnCount + col;
            int tileInfoOffset = 24 + 8 * tileIndex;
            int tileDataOffset = 0;
            int tileDataBytes = 0;
            ifs.seekg(tileInfoOffset);
            ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
            ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
            ifs.seekg(tileDataStartPos + tileDataOffset);
            ifs.read(readBuffer, tileDataBytes);
            // decompress
            tile2argb4(readBuffer, tileDataBytes, pTempDecompressionBuffer);

            for (int i = 0; i < tileHeight; i++) {
                for (int j = 0; j < tileWidth; j++) {
                    int globalRow = row * tileHeight + i;
                    int globalCol = col * tileWidth + j;
                    int indexInTile = i * tileWidth + j;
                    memcpy(&pDecompressedARGB[(globalRow * imgWidth + globalCol) * BYTES_PER_PIXEL],
                           &pTempDecompressionBuffer[indexInTile * BYTES_PER_PIXEL],
                           4);
                }
            }
        }
    }
    ifs.close();

    // save decompressed image to output file
    stbi_write_bmp(outFileName, imgWidth, imgHeight, STBI_rgb_alpha, reinterpret_cast<char const *>(pDecompressedARGB));

    delete[] pDecompressedARGB;
    return ERROR_OK;
}

/*
* compress ARGB data to TILE
*/
int compressARGB512(char const * inFileName, char const * outFileName) {
    float ratio;
    int     ret = ERROR_OK;
    int     width, height, nrChannels;
    unsigned char *data = stbi_load(inFileName, &width, &height, &nrChannels, STBI_rgb_alpha);
    int over_cnt = 0;
    if (data == NULL) {
        std::cout << "cannot open file: " << inFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "image info: width = " << width
              << ", height = " << height
              << ", channels = " << nrChannels
              << std::endl;

    const int TILE_WIDTH = 16;
    const int TILE_HEIGHT = 8;
    int numRows = (height - 1) / TILE_HEIGHT + 1;
    int numColumns = (width - 1) / TILE_WIDTH + 1;
    const int BYTES_PER_PIXEL = 4;
    int rowStride = numColumns * 16 * BYTES_PER_PIXEL; // 4 bytes per pixel
    unsigned char pARGB[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };
    unsigned char pCompressed[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };

    tileSetSize(TILE_WIDTH, TILE_HEIGHT);

    unsigned char * pCompressionBuffer = new unsigned char [numRows * numColumns * TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL];
    if (NULL == pCompressionBuffer) {
        return ERROR_CUSTOM;
    }
    TileCompressionInfo *pTCInfos = new TileCompressionInfo [numRows * numColumns];
    if (NULL == pTCInfos) {
        delete[] pCompressionBuffer;
        return ERROR_CUSTOM;
    }

    int tileRowIndex = 0;
    int tileColumnIndex = 0;
    int totalBitsAfterCompression = 0;
    int posInCompressionBuffer = 0;
    unsigned char* pClr = NULL;

    //------------------------------------------拓展不能16整除的边---------------------------------------------------//
    unsigned char * extend_buffer = new unsigned char [numRows * numColumns * TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL];
    for (int h = 0; h < height; h++){
        for(int w = 0; w < width; w++){
            int buffer_index = (h*TILE_WIDTH*numColumns+w)*4;
            int data_index = (h*width+w)*4;
            extend_buffer[buffer_index] = data[data_index];
            extend_buffer[buffer_index+1] = data[data_index+1];
            extend_buffer[buffer_index+2] = data[data_index+2];
            extend_buffer[buffer_index+3] = data[data_index+3];
        }
    }

    //----------------------------------------分块--------------------------------------------//
    for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
        for (tileColumnIndex = 0; tileColumnIndex < numColumns; tileColumnIndex++) {
            int tileIndex = tileRowIndex * numColumns + tileColumnIndex;
            pClr = pARGB;
            for (int i = 0; i < TILE_HEIGHT; i++) {
                for (int j = 0; j < TILE_WIDTH; j++) {
                    int row = tileRowIndex * TILE_HEIGHT + i;
                    int col = tileColumnIndex * TILE_WIDTH + j;
                    int pixelDataOffset = rowStride * row + col * BYTES_PER_PIXEL;
                    pClr[0] = extend_buffer[pixelDataOffset];     // b
                    pClr[1] = extend_buffer[pixelDataOffset + 1]; // g
                    pClr[2] = extend_buffer[pixelDataOffset + 2]; // r
                    pClr[3] = extend_buffer[pixelDataOffset + 3]; // a
                    pClr += 4;

                }
            }
            pTCInfos[tileIndex].tilePosition = posInCompressionBuffer;

            //---------------------------------------------------------------------------------------//
            // compress
            argb2tile512(pARGB, pCompressionBuffer + posInCompressionBuffer, &pTCInfos[tileIndex].tileSize);
            posInCompressionBuffer += pTCInfos[tileIndex].tileSize;
        }
    }
    std::cout << "compression ratio = " << (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100 << "%" << std::endl;

    // save compressed data to JLCD file
    std::ofstream ofs;
    ofs.open(outFileName, std::ios::binary | std::ios::out);

    int tileCount = numRows * numColumns;
    int fileHeaderSize = 24;
    int tileInfoSize = tileCount * 8;
    int dataOffsetInFile = fileHeaderSize + tileInfoSize;
    if (ofs.is_open()) {
        ofs.write("JLCD", 4);
        ofs.write(reinterpret_cast<const char *>(&width), 4);
        ofs.write(reinterpret_cast<const char *>(&height), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_WIDTH), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_HEIGHT), 4);
        ofs.write(reinterpret_cast<const char *>(&tileCount), 4);
        // tile data offset + len
        for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
            for (tileColumnIndex = 0; tileColumnIndex < numColumns; tileColumnIndex++) {
                int tileIndex = tileRowIndex * numColumns + tileColumnIndex;
                ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tilePosition), 4);
                ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tileSize), 4);
            }
        }
        ofs.flush();
        // all tile data
        ofs.write(reinterpret_cast<const char *>(pCompressionBuffer), posInCompressionBuffer);
        ofs.close();
    } else {
        std::cout << "fail to open output file(" << outFileName << ")" << std::endl;
    }

    stbi_image_free(data);
    delete [] pCompressionBuffer;
    delete [] pTCInfos;
    return ERROR_OK;
}

/*
* line decompress TILE data to ARGB
*/
int decompressARGB512(char const * compressedFileName, char const * outFileName) {
    std::ifstream ifs;
    ifs.open(compressedFileName, std::ios::binary | std::ios::in);

    if (!ifs.is_open()) {
        std::cout << "fail to open output file: " << compressedFileName << std::endl;
        return ERROR_OUTPUT_FILE;
    }

    char readBuffer[2048];
    // read "JLCD"
    ifs.read(readBuffer, 4);
    if (strncmp(readBuffer, "JLCD", 4) != 0) {
        ifs.close();
        std::cout << "ERROR: INVALID tile file: " << compressedFileName << std::endl;
        return ERROR_INVALID_INPUT_FILE;
    }

    const int BYTES_PER_PIXEL = 4;
    int imgWidth, imgHeight, tileWidth, tileHeight, tileCount;
    // read image width
    ifs.read(reinterpret_cast<char*>(&imgWidth), 4);
    ifs.read(reinterpret_cast<char*>(&imgHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileWidth), 4);
    ifs.read(reinterpret_cast<char*>(&tileHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileCount), 4);

    if(tileWidth != 16 && tileHeight != 8){
        ifs.close();
        std::cout << "ERROR: the compressed data is not tile 16x16 compress: " << compressedFileName << std::endl;
        return ERROR_INPUT_FILE;
    }

    std::cout << "imgWidth = " << imgWidth
              << ", imgHeight = " << imgHeight
              << ", tileWidth = " << tileWidth
              << ", tileHeight = " << tileHeight
              << std::endl;

    int tileRowCount = (imgHeight - 1) / tileHeight + 1;
    int tileColumnCount = (imgWidth - 1) / tileWidth + 1;
    int tileDataStartPos = 24 + 8 * tileCount;
    unsigned char *pDecompressedARGB = new unsigned char [tileRowCount * tileColumnCount * 16 * 8 * 4];
    unsigned char *final_data = new unsigned char [imgWidth * imgHeight * 4];
    unsigned char pTempDecompressionBuffer[2048];

    for (int row = 0; row < tileRowCount; row++) {
        for (int col = 0; col < tileColumnCount; col++) {
            int tileIndex = row * tileColumnCount + col;
            int tileInfoOffset = 24 + 8 * tileIndex;
            int tileDataOffset = 0;
            int tileDataBytes = 0;
            ifs.seekg(tileInfoOffset);
            ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
            ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
            ifs.seekg(tileDataStartPos + tileDataOffset);
            ifs.read(readBuffer, tileDataBytes);
            // decompress
            tile2argb512(readBuffer, tileDataBytes, pTempDecompressionBuffer);


            for (int i = 0; i < tileHeight; i++) {
                for (int j = 0; j < tileWidth; j++) {
                    int globalRow = row * tileHeight + i;
                    int globalCol = col * tileWidth + j;
                    int indexInTile = i * tileWidth + j;
                    memcpy(&pDecompressedARGB[(globalRow * 16*tileColumnCount + globalCol) * BYTES_PER_PIXEL],
                           &pTempDecompressionBuffer[indexInTile * BYTES_PER_PIXEL],
                           4);
                }
            }
        }
    }

    for (int h = 0; h < imgHeight; h++) {
        for (int w = 0; w < imgWidth; w++) {
            int buffer_index = (h * 16 * tileColumnCount + w) * 4;
            int data_index = (h * imgWidth + w) * 4;
            final_data[data_index] = pDecompressedARGB[buffer_index];
            final_data[data_index + 1] = pDecompressedARGB[buffer_index + 1];
            final_data[data_index + 2] = pDecompressedARGB[buffer_index + 2];
            final_data[data_index + 3] = pDecompressedARGB[buffer_index + 3];


        }
    }

    ifs.close();

    // save decompressed image to output file
    stbi_write_bmp(outFileName, imgWidth, imgHeight, STBI_rgb_alpha, reinterpret_cast<char const *>(final_data));

    delete[] pDecompressedARGB;
    return ERROR_OK;
}

/*
* compress ARGB data to TILE
*/
int linecompressARGB64(char const * inFileName, char const * outFileName) {
    float ratio;
    int     ret = ERROR_OK;
    int     width, height, nrChannels;
    unsigned char *data = stbi_load(inFileName, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data == NULL) {
        std::cout << "cannot open file: " << inFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "image info: width = " << width
              << ", height = " << height
              << ", channels = " << nrChannels
              << std::endl;
    const int TILE_WIDTH = 16;
    const int TILE_HEIGHT = 1;
    int numRows = width * height / TILE_WIDTH;
    const int BYTES_PER_PIXEL = 4;
    unsigned char pARGB[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };

    tileSetSize(TILE_WIDTH, TILE_HEIGHT);

    unsigned char * pCompressionBuffer = new unsigned char [ width * height * BYTES_PER_PIXEL];
    if (NULL == pCompressionBuffer) {
        return ERROR_CUSTOM;
    }
    TileCompressionInfo *pTCInfos = new TileCompressionInfo [numRows];
    if (NULL == pTCInfos) {
        delete[] pCompressionBuffer;
        return ERROR_CUSTOM;
    }

    int tileRowIndex = 0;
    int posInCompressionBuffer = 0;
    unsigned char* pClr = NULL;

    for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++)
    {
        int tileIndex = tileRowIndex;
        pClr = pARGB;
        for (int j = 0; j < TILE_WIDTH; j++)
        {
            int pixelDataOffset = BYTES_PER_PIXEL * (tileRowIndex*TILE_WIDTH + j);
            pClr[0] = data[pixelDataOffset];     // b
            pClr[1] = data[pixelDataOffset + 1]; // g
            pClr[2] = data[pixelDataOffset + 2]; // r
            pClr[3] = data[pixelDataOffset + 3]; // a
            pClr += 4;
        }
        pTCInfos[tileIndex].tilePosition = posInCompressionBuffer;
        // compress
        lineargb2tile64(pARGB, pCompressionBuffer + posInCompressionBuffer, &pTCInfos[tileIndex].tileSize);
        posInCompressionBuffer += pTCInfos[tileIndex].tileSize;
    }
    std::cout <<"compression ratio = " << (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100 << "%" << std::endl;

    ratio = (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100;
    // save compressed data to JLCD file
    std::ofstream ofs;
    ofs.open(outFileName, std::ios::binary | std::ios::out);

    int tileCount = numRows;
    int fileHeaderSize = 24;
    int tileInfoSize = tileCount * 8;
    int dataOffsetInFile = fileHeaderSize + tileInfoSize;
    if (ofs.is_open()) {
        ofs.write("JLCD", 4);
        ofs.write(reinterpret_cast<const char *>(&width), 4);
        ofs.write(reinterpret_cast<const char *>(&height), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_WIDTH), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_HEIGHT), 4);
        ofs.write(reinterpret_cast<const char *>(&tileCount), 4);
        // tile data offset + len
        for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
            int tileIndex = tileRowIndex;
            ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tilePosition), 4);
            ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tileSize), 4);
        }
        ofs.flush();
        // all tile data
        ofs.write(reinterpret_cast<const char *>(pCompressionBuffer), posInCompressionBuffer);
        ofs.close();
    } else {
        std::cout << "fail to open output file(" << outFileName << ")" << std::endl;
    }

    stbi_image_free(data);
    delete [] pCompressionBuffer;
    delete [] pTCInfos;
    return ERROR_OK;
}

/*
* line decompress TILE data to ARGB
*/
int linedecompressARGB64(char const * compressedFileName, char const * outFileName) {
    std::ifstream ifs;
    ifs.open(compressedFileName, std::ios::binary | std::ios::in);

    if (!ifs.is_open()) {
        std::cout << "fail to open output file: " << compressedFileName << std::endl;
        return ERROR_OUTPUT_FILE;
    }

    char readBuffer[1024];
    // read "JLCD"
    ifs.read(readBuffer, 4);
    if (strncmp(readBuffer, "JLCD", 4) != 0) {
        ifs.close();
        std::cout << "ERROR: INVALID tile file: " << compressedFileName << std::endl;
        return ERROR_INVALID_INPUT_FILE;
    }

    const int BYTES_PER_PIXEL = 4;
    int imgWidth, imgHeight, tileWidth, tileHeight, tileCount;
    // read image width
    ifs.read(reinterpret_cast<char*>(&imgWidth), 4);
    ifs.read(reinterpret_cast<char*>(&imgHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileWidth), 4);
    ifs.read(reinterpret_cast<char*>(&tileHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileCount), 4);

    if(tileWidth != 16 && tileHeight != 1){
        ifs.close();
        std::cout << "ERROR: the compressed data is not line tile compress: " << compressedFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "imgWidth = " << imgWidth
              << ", imgHeight = " << imgHeight
              << ", tileWidth = " << tileWidth
              << ", tileHeight = " << tileHeight
              << std::endl;
    int tileColumnCount = imgWidth*imgHeight / tileWidth;
    int tileDataStartPos = 24 + 8 * tileCount;
    unsigned char *pDecompressedARGB = new unsigned char [imgWidth * imgHeight * 4];
    unsigned char pTempDecompressionBuffer[1024];

    for (int col = 0; col < tileColumnCount; col++)
    {
        int tileIndex = col;
        int tileInfoOffset = 24 + 8 * tileIndex;
        int tileDataOffset = 0;
        int tileDataBytes = 0;
        ifs.seekg(tileInfoOffset);
        ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
        ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
        ifs.seekg(tileDataStartPos + tileDataOffset);
        ifs.read(readBuffer, tileDataBytes);
        // decompress
        linetile2argb64(readBuffer, tileDataBytes, pTempDecompressionBuffer);

        for (int j = 0; j < tileWidth; j++)
        {
            int globalCol = col * tileWidth + j;
            memcpy(&pDecompressedARGB[globalCol* BYTES_PER_PIXEL],
                   &pTempDecompressionBuffer[j * BYTES_PER_PIXEL],
                   4);
        }
    }
    ifs.close();

    // save decompressed image to output file
    stbi_write_bmp(outFileName, imgWidth, imgHeight, STBI_rgb_alpha, reinterpret_cast<char const *>(pDecompressedARGB));

    delete[] pDecompressedARGB;
    return ERROR_OK;
}

/*
* compress ARGB data to TILE
*/
int linecompressARGB256(char const * inFileName, char const * outFileName) {
    float ratio;
    int     ret = ERROR_OK;
    int     width, height, nrChannels;
    unsigned char *data = stbi_load(inFileName, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data == NULL) {
        std::cout << "cannot open file: " << inFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "image info: width = " << width
              << ", height = " << height
              << ", channels = " << nrChannels
              << std::endl;
    const int TILE_WIDTH = 64;
    const int TILE_HEIGHT = 1;
    int numRows = width * height / TILE_WIDTH;
    const int BYTES_PER_PIXEL = 4;
    unsigned char pARGB[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };

    tileSetSize(TILE_WIDTH, TILE_HEIGHT);

    unsigned char * pCompressionBuffer = new unsigned char [ width * height * BYTES_PER_PIXEL];
    if (NULL == pCompressionBuffer) {
        return ERROR_CUSTOM;
    }
    TileCompressionInfo *pTCInfos = new TileCompressionInfo [numRows];
    if (NULL == pTCInfos) {
        delete[] pCompressionBuffer;
        return ERROR_CUSTOM;
    }

    int tileRowIndex = 0;
    int posInCompressionBuffer = 0;
    unsigned char* pClr = NULL;

    for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++)
    {
        int tileIndex = tileRowIndex;
        pClr = pARGB;
        for (int j = 0; j < TILE_WIDTH; j++)
        {
            int pixelDataOffset = BYTES_PER_PIXEL * (tileRowIndex*TILE_WIDTH + j);
            pClr[0] = data[pixelDataOffset];     // b
            pClr[1] = data[pixelDataOffset + 1]; // g
            pClr[2] = data[pixelDataOffset + 2]; // r
            pClr[3] = data[pixelDataOffset + 3]; // a
            pClr += 4;
        }
        pTCInfos[tileIndex].tilePosition = posInCompressionBuffer;
        // compress
        lineargb2tile256(pARGB, pCompressionBuffer + posInCompressionBuffer, &pTCInfos[tileIndex].tileSize);
        posInCompressionBuffer += pTCInfos[tileIndex].tileSize;
    }
    std::cout<<"compression ratio = " << (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100 << "%" << std::endl;

    ratio = (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100;
    // save compressed data to JLCD file
    std::ofstream ofs;
    ofs.open(outFileName, std::ios::binary | std::ios::out);

    int tileCount = numRows;
    int fileHeaderSize = 24;
    int tileInfoSize = tileCount * 8;
    int dataOffsetInFile = fileHeaderSize + tileInfoSize;
    if (ofs.is_open()) {
        ofs.write("JLCD", 4);
        ofs.write(reinterpret_cast<const char *>(&width), 4);
        ofs.write(reinterpret_cast<const char *>(&height), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_WIDTH), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_HEIGHT), 4);
        ofs.write(reinterpret_cast<const char *>(&tileCount), 4);
        // tile data offset + len
        for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
            int tileIndex = tileRowIndex;
            ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tilePosition), 4);
            ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tileSize), 4);
        }
        ofs.flush();
        // all tile data
        ofs.write(reinterpret_cast<const char *>(pCompressionBuffer), posInCompressionBuffer);
        ofs.close();
    } else {
        std::cout << "fail to open output file(" << outFileName << ")" << std::endl;
    }

    stbi_image_free(data);
    delete [] pCompressionBuffer;
    delete [] pTCInfos;
    return ERROR_OK;
}

/*
* line decompress TILE data to ARGB
*/
int linedecompressARGB256(char const * compressedFileName, char const * outFileName) {
    std::ifstream ifs;
    ifs.open(compressedFileName, std::ios::binary | std::ios::in);

    if (!ifs.is_open()) {
        std::cout << "fail to open output file: " << compressedFileName << std::endl;
        return ERROR_OUTPUT_FILE;
    }

    char readBuffer[1024];
    // read "JLCD"
    ifs.read(readBuffer, 4);
    if (strncmp(readBuffer, "JLCD", 4) != 0) {
        ifs.close();
        std::cout << "ERROR: INVALID tile file: " << compressedFileName << std::endl;
        return ERROR_INVALID_INPUT_FILE;
    }

    const int BYTES_PER_PIXEL = 4;
    int imgWidth, imgHeight, tileWidth, tileHeight, tileCount;
    // read image width
    ifs.read(reinterpret_cast<char*>(&imgWidth), 4);
    ifs.read(reinterpret_cast<char*>(&imgHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileWidth), 4);
    ifs.read(reinterpret_cast<char*>(&tileHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileCount), 4);

    if(tileWidth != 64 && tileHeight != 1){
        ifs.close();
        std::cout << "ERROR: the compressed data is not line tile compress: " << compressedFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "imgWidth = " << imgWidth
              << ", imgHeight = " << imgHeight
              << ", tileWidth = " << tileWidth
              << ", tileHeight = " << tileHeight
              << std::endl;
    int tileColumnCount = imgWidth*imgHeight / tileWidth;
    int tileDataStartPos = 24 + 8 * tileCount;
    unsigned char *pDecompressedARGB = new unsigned char [imgWidth * imgHeight * 4];
    unsigned char pTempDecompressionBuffer[1024];

    for (int col = 0; col < tileColumnCount; col++)
    {
        int tileIndex = col;
        int tileInfoOffset = 24 + 8 * tileIndex;
        int tileDataOffset = 0;
        int tileDataBytes = 0;
        ifs.seekg(tileInfoOffset);
        ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
        ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
        ifs.seekg(tileDataStartPos + tileDataOffset);
        ifs.read(readBuffer, tileDataBytes);
        // decompress
        linetile2argb256(readBuffer, tileDataBytes, pTempDecompressionBuffer);

        for (int j = 0; j < tileWidth; j++)
        {
            int globalCol = col * tileWidth + j;
            memcpy(&pDecompressedARGB[globalCol* BYTES_PER_PIXEL],
                   &pTempDecompressionBuffer[j * BYTES_PER_PIXEL],
                   4);
        }
    }
    ifs.close();

    // save decompressed image to output file
    stbi_write_bmp(outFileName, imgWidth, imgHeight, STBI_rgb_alpha, reinterpret_cast<char const *>(pDecompressedARGB));

    delete[] pDecompressedARGB;
    return ERROR_OK;
}

/*
* compress ARGB data to TILE
*/
int linecompressARGB512(char const * inFileName, char const * outFileName) {
    float ratio;
    int     ret = ERROR_OK;
    int     width, height, nrChannels;
    unsigned char *data = stbi_load(inFileName, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data == NULL) {
        std::cout << "cannot open file: " << inFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "image info: width = " << width
              << ", height = " << height
              << ", channels = " << nrChannels
              << std::endl;
    const int TILE_WIDTH = 128;
    const int TILE_HEIGHT = 1;
    int numRows = (width * height-1) / TILE_WIDTH+1;
    const int BYTES_PER_PIXEL = 4;
    unsigned char pARGB[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };

    tileSetSize(TILE_WIDTH, TILE_HEIGHT);

    unsigned char * pCompressionBuffer = new unsigned char [ width * height * BYTES_PER_PIXEL];
    if (NULL == pCompressionBuffer) {
        return ERROR_CUSTOM;
    }
    TileCompressionInfo *pTCInfos = new TileCompressionInfo [numRows];
    if (NULL == pTCInfos) {
        delete[] pCompressionBuffer;
        return ERROR_CUSTOM;
    }

    int tileRowIndex = 0;
    int posInCompressionBuffer = 0;
    unsigned char* pClr = NULL;


    //------------------------------------------拓展不能16整除的边---------------------------------------------------//
    unsigned char * extend_buffer = new unsigned char [numRows * TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL];
    for (int j = 0; j < width*height; j++){
        int dataIndex = j*4;
        extend_buffer[dataIndex] = data[dataIndex];
        extend_buffer[dataIndex+1] = data[dataIndex+1];
        extend_buffer[dataIndex+2] = data[dataIndex+2];
        extend_buffer[dataIndex+3] = data[dataIndex+3];
    }
    for (int i = width*height; i < numRows*128; i++){
        int index = i*4;
        int preIndex = (i-1)*4;
        extend_buffer[index] = extend_buffer[preIndex];
        extend_buffer[index+1] = extend_buffer[preIndex+1];
        extend_buffer[index+2] = extend_buffer[preIndex+2];
        extend_buffer[index+3] = extend_buffer[preIndex+3];
    }

    for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++)
    {
        int tileIndex = tileRowIndex;
        pClr = pARGB;
        for (int j = 0; j < TILE_WIDTH; j++)
        {
            int pixelDataOffset = BYTES_PER_PIXEL * (tileRowIndex*TILE_WIDTH + j);
            pClr[0] = data[pixelDataOffset];     // b
            pClr[1] = data[pixelDataOffset + 1]; // g
            pClr[2] = data[pixelDataOffset + 2]; // r
            pClr[3] = data[pixelDataOffset + 3]; // a
            pClr += 4;
        }
        pTCInfos[tileIndex].tilePosition = posInCompressionBuffer;
        // compress
        lineargb2tile512(pARGB, pCompressionBuffer + posInCompressionBuffer, &pTCInfos[tileIndex].tileSize);
        posInCompressionBuffer += pTCInfos[tileIndex].tileSize;
    }
    std::cout<<"compression ratio = " << (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100 << "%" << std::endl;

    ratio = (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100;
    // save compressed data to JLCD file
    std::ofstream ofs;
    ofs.open(outFileName, std::ios::binary | std::ios::out);

    int tileCount = numRows;
    int fileHeaderSize = 24;
    int tileInfoSize = tileCount * 8;
    int dataOffsetInFile = fileHeaderSize + tileInfoSize;
    if (ofs.is_open()) {
        ofs.write("JLCD", 4);
        ofs.write(reinterpret_cast<const char *>(&width), 4);
        ofs.write(reinterpret_cast<const char *>(&height), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_WIDTH), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_HEIGHT), 4);
        ofs.write(reinterpret_cast<const char *>(&tileCount), 4);
        // tile data offset + len
        for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
            int tileIndex = tileRowIndex;
            ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tilePosition), 4);
            ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tileSize), 4);
        }
        ofs.flush();
        // all tile data
        ofs.write(reinterpret_cast<const char *>(pCompressionBuffer), posInCompressionBuffer);
        ofs.close();
    } else {
        std::cout << "fail to open output file(" << outFileName << ")" << std::endl;
    }

    stbi_image_free(data);
    delete [] pCompressionBuffer;
    delete [] pTCInfos;
    return ERROR_OK;
}

/*
* line decompress TILE data to ARGB
*/
int linedecompressARGB512(char const * compressedFileName, char const * outFileName) {
    std::ifstream ifs;
    ifs.open(compressedFileName, std::ios::binary | std::ios::in);

    if (!ifs.is_open()) {
        std::cout << "fail to open output file: " << compressedFileName << std::endl;
        return ERROR_OUTPUT_FILE;
    }

    char readBuffer[4096];
    // read "JLCD"
    ifs.read(readBuffer, 4);
    if (strncmp(readBuffer, "JLCD", 4) != 0) {
        ifs.close();
        std::cout << "ERROR: INVALID tile file: " << compressedFileName << std::endl;
        return ERROR_INVALID_INPUT_FILE;
    }

    const int BYTES_PER_PIXEL = 4;
    int imgWidth, imgHeight, tileWidth, tileHeight, tileCount;
    // read image width
    ifs.read(reinterpret_cast<char*>(&imgWidth), 4);
    ifs.read(reinterpret_cast<char*>(&imgHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileWidth), 4);
    ifs.read(reinterpret_cast<char*>(&tileHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileCount), 4);

    if(tileWidth != 128 && tileHeight != 1){
        ifs.close();
        std::cout << "ERROR: the compressed data is not line tile compress: " << compressedFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "imgWidth = " << imgWidth
              << ", imgHeight = " << imgHeight
              << ", tileWidth = " << tileWidth
              << ", tileHeight = " << tileHeight
              << std::endl;
    int tileColumnCount = (imgWidth*imgHeight-1) / tileWidth +1;
    int tileDataStartPos = 24 + 8 * tileCount;
    unsigned char *pDecompressedARGB = new unsigned char [tileColumnCount* tileWidth* 4];
    unsigned char pTempDecompressionBuffer[4096];

    for (int col = 0; col < tileColumnCount; col++)
    {
        int tileIndex = col;
        int tileInfoOffset = 24 + 8 * tileIndex;
        int tileDataOffset = 0;
        int tileDataBytes = 0;
        ifs.seekg(tileInfoOffset);
        ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
        ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
        ifs.seekg(tileDataStartPos + tileDataOffset);
        ifs.read(readBuffer, tileDataBytes);
        // decompress
        linetile2argb512(readBuffer, tileDataBytes, pTempDecompressionBuffer);

        for (int j = 0; j < tileWidth; j++)
        {
            int globalCol = col * tileWidth + j;
            memcpy(&pDecompressedARGB[globalCol* BYTES_PER_PIXEL],
                   &pTempDecompressionBuffer[j * BYTES_PER_PIXEL],
                   4);
        }
    }
    ifs.close();

    // save decompressed image to output file
    stbi_write_bmp(outFileName, imgWidth, imgHeight, STBI_rgb_alpha, reinterpret_cast<char const *>(pDecompressedARGB));

    delete[] pDecompressedARGB;
    return ERROR_OK;
}

/*
* compress ARGB data to TILE
*/
int linecompressARGB1024(char const * inFileName, char const * outFileName) {
    float ratio;
    int     ret = ERROR_OK;
    int     width, height, nrChannels;
    unsigned char *data = stbi_load(inFileName, &width, &height, &nrChannels, STBI_rgb_alpha);
    if (data == NULL) {
        std::cout << "cannot open file: " << inFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "image info: width = " << width
              << ", height = " << height
              << ", channels = " << nrChannels
              << std::endl;
    const int TILE_WIDTH = 256;
    const int TILE_HEIGHT = 1;

    int numRows = (width * height -1)/ TILE_WIDTH+1;

    const int BYTES_PER_PIXEL = 4;
    unsigned char pARGB[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL] = { 0u };

    tileSetSize(TILE_WIDTH, TILE_HEIGHT);

    unsigned char * pCompressionBuffer = new unsigned char [ width * height * BYTES_PER_PIXEL];
    if (NULL == pCompressionBuffer) {
        return ERROR_CUSTOM;
    }
    TileCompressionInfo *pTCInfos = new TileCompressionInfo [numRows];
    if (NULL == pTCInfos) {
        delete[] pCompressionBuffer;
        return ERROR_CUSTOM;
    }

    int tileRowIndex = 0;
    int posInCompressionBuffer = 0;
    unsigned char* pClr = NULL;

    //------------------------------------------拓展不能16整除的边---------------------------------------------------//
    unsigned char * extend_buffer = new unsigned char [numRows * TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL];
    for (int j = 0; j < width*height; j++){
        int dataIndex = j*4;
        extend_buffer[dataIndex] = data[dataIndex];
        extend_buffer[dataIndex+1] = data[dataIndex+1];
        extend_buffer[dataIndex+2] = data[dataIndex+2];
        extend_buffer[dataIndex+3] = data[dataIndex+3];
    }
    for (int i = width*height; i < numRows*256; i++){
            int index = i*4;
            int preIndex = (i-1)*4;
            extend_buffer[index] = extend_buffer[preIndex];
            extend_buffer[index+1] = extend_buffer[preIndex+1];
            extend_buffer[index+2] = extend_buffer[preIndex+2];
            extend_buffer[index+3] = extend_buffer[preIndex+3];
    }


    for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++)
    {
        int tileIndex = tileRowIndex;
        pClr = pARGB;
        for (int j = 0; j < TILE_WIDTH; j++)
        {
            int pixelDataOffset = BYTES_PER_PIXEL * (tileRowIndex*TILE_WIDTH + j);
            pClr[0] = extend_buffer[pixelDataOffset];     // b
            pClr[1] = extend_buffer[pixelDataOffset + 1]; // g
            pClr[2] = extend_buffer[pixelDataOffset + 2]; // r
            pClr[3] = extend_buffer[pixelDataOffset + 3]; // a
            pClr += 4;
        }
        pTCInfos[tileIndex].tilePosition = posInCompressionBuffer;
        // compress
        lineargb2tile1024(pARGB, pCompressionBuffer + posInCompressionBuffer, &pTCInfos[tileIndex].tileSize);
        posInCompressionBuffer += pTCInfos[tileIndex].tileSize;
    }
    std::cout <<"compression ratio = " << (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100 << "%" << std::endl;

    ratio = (float)posInCompressionBuffer / (float)(width * height * BYTES_PER_PIXEL) * 100;
    // save compressed data to JLCD file
    std::ofstream ofs;
    ofs.open(outFileName, std::ios::binary | std::ios::out);

    int tileCount = numRows;
    int fileHeaderSize = 24;
    int tileInfoSize = tileCount * 8;
    int dataOffsetInFile = fileHeaderSize + tileInfoSize;
    if (ofs.is_open()) {
        ofs.write("JLCD", 4);
        ofs.write(reinterpret_cast<const char *>(&width), 4);
        ofs.write(reinterpret_cast<const char *>(&height), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_WIDTH), 4);
        ofs.write(reinterpret_cast<const char *>(&TILE_HEIGHT), 4);
        ofs.write(reinterpret_cast<const char *>(&tileCount), 4);
        // tile data offset + len
        for (tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
            int tileIndex = tileRowIndex;
            ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tilePosition), 4);
            ofs.write(reinterpret_cast<const char *>(&pTCInfos[tileIndex].tileSize), 4);
        }
        ofs.flush();
        // all tile data
        ofs.write(reinterpret_cast<const char *>(pCompressionBuffer), posInCompressionBuffer);
        ofs.close();
    } else {
        std::cout << "fail to open output file(" << outFileName << ")" << std::endl;
    }

    stbi_image_free(data);
    delete [] pCompressionBuffer;
    delete [] pTCInfos;
    return ERROR_OK;
}

/*
* line decompress TILE data to ARGB
*/
int linedecompressARGB1024(char const * compressedFileName, char const * outFileName) {
    std::ifstream ifs;
    ifs.open(compressedFileName, std::ios::binary | std::ios::in);

    if (!ifs.is_open()) {
        std::cout << "fail to open output file: " << compressedFileName << std::endl;
        return ERROR_OUTPUT_FILE;
    }

    char readBuffer[4096];
    // read "JLCD"
    ifs.read(readBuffer, 4);
    if (strncmp(readBuffer, "JLCD", 4) != 0) {
        ifs.close();
        std::cout << "ERROR: INVALID tile file: " << compressedFileName << std::endl;
        return ERROR_INVALID_INPUT_FILE;
    }

    const int BYTES_PER_PIXEL = 4;
    int imgWidth, imgHeight, tileWidth, tileHeight, tileCount;
    // read image width
    ifs.read(reinterpret_cast<char*>(&imgWidth), 4);
    ifs.read(reinterpret_cast<char*>(&imgHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileWidth), 4);
    ifs.read(reinterpret_cast<char*>(&tileHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileCount), 4);

    if(tileWidth != 256 && tileHeight != 1){
        ifs.close();
        std::cout << "ERROR: the compressed data is not line tile compress: " << compressedFileName << std::endl;
        return ERROR_INPUT_FILE;
    }
    std::cout << "imgWidth = " << imgWidth
              << ", imgHeight = " << imgHeight
              << ", tileWidth = " << tileWidth
              << ", tileHeight = " << tileHeight
              << std::endl;
    int tileColumnCount = (imgWidth*imgHeight-1) / tileWidth+1;
    int tileDataStartPos = 24 + 8 * tileCount;
    unsigned char *pDecompressedARGB = new unsigned char [tileColumnCount* tileWidth * 4];
    unsigned char pTempDecompressionBuffer[4096];

    for (int col = 0; col < tileColumnCount; col++)
    {
        int tileIndex = col;
        int tileInfoOffset = 24 + 8 * tileIndex;
        int tileDataOffset = 0;
        int tileDataBytes = 0;
        ifs.seekg(tileInfoOffset);
        ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
        ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
        ifs.seekg(tileDataStartPos + tileDataOffset);
        ifs.read(readBuffer, tileDataBytes);
        // decompress
        linetile2argb1024(readBuffer, tileDataBytes, pTempDecompressionBuffer);

        for (int j = 0; j < tileWidth; j++)
        {
            int globalCol = col * tileWidth + j;
            memcpy(&pDecompressedARGB[globalCol* BYTES_PER_PIXEL],
                   &pTempDecompressionBuffer[j * BYTES_PER_PIXEL],
                   4);
        }
    }
    ifs.close();

    // save decompressed image to output file
    stbi_write_bmp(outFileName, imgWidth, imgHeight, STBI_rgb_alpha, reinterpret_cast<char const *>(pDecompressedARGB));

    delete[] pDecompressedARGB;
    return ERROR_OK;
}
