/* Compress and Decompress image data
*/
#include <cstring>
#include <iostream>
#include "defines.h"
#include "codec.h"

#define APP_VERSION     "1.6.3"

int readRandomTile(char const * compressedFileName,unsigned char* readtileout,int col,int row)
{
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
    int tile_widthcount,tile_heightcount;
    // read image width
    ifs.read(reinterpret_cast<char*>(&imgWidth), 4);
    ifs.read(reinterpret_cast<char*>(&imgHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileWidth), 4);
    ifs.read(reinterpret_cast<char*>(&tileHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileCount), 4);
    if(tileWidth == 64 && tileHeight == 1){
        tile_widthcount = imgWidth*imgHeight / tileWidth;
        if (row != 0) {
            ifs.close();
            std::cout << "ERROR: compressed data is line tile compress ,please input row value to 0" << tile_heightcount << std::endl;
            return ERROR_INVALID_PARAM;
        }
        if (col > tile_widthcount) {
            ifs.close();
            std::cout << "ERROR: col out of range width , width range is 0 ~ " << tile_widthcount << std::endl;
            return ERROR_INVALID_PARAM;
        }


    }else {
        tile_widthcount = (imgHeight - 1) / tileHeight + 1;
        tile_heightcount = (imgWidth - 1) / tileWidth + 1;
        if (col > tile_widthcount) {
            ifs.close();
            std::cout << "ERROR: col out of range width , width range is 0 ~ " << tile_widthcount << std::endl;
            return ERROR_INVALID_PARAM;
        }
        if (row > tile_heightcount) {
            ifs.close();
            std::cout << "ERROR: row out of range height , width range is 0 ~ " << tile_heightcount << std::endl;
            return ERROR_INVALID_PARAM;
        }
    }


    int tileIndex = col;
    int tileInfoOffset = 24 + 8 * tileIndex;
    int tileDataStartPos = 24 + 8 * tileCount;
    int tileDataOffset = 0;
    int tileDataBytes = 0;
    ifs.seekg(tileInfoOffset);
    ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
    ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
    ifs.seekg(tileDataStartPos + tileDataOffset);
    ifs.read(readBuffer, tileDataBytes);
    // decompress
    if(tileWidth == 4 && tileHeight == 4) tile2argb4(readBuffer, tileDataBytes, readtileout);
    else if(tileWidth == 8 && tileHeight == 8) tile2argb(readBuffer, tileDataBytes, readtileout);
    else if(tileWidth == 16 && tileHeight == 16) tile2argb16(readBuffer, tileDataBytes, readtileout);
    else if(tileWidth == 64 && tileHeight == 1) tile2argb512(readBuffer, tileDataBytes, readtileout);

    ifs.close();
    return ERROR_OK;

    ifs.close();
    std::cout << "ERROR: different size not support: " << compressedFileName << std::endl;
    return ERROR_INVALID_PARAM;

}

int writeRandomTile(char const * compressedFileName,unsigned char* writetilein,int col,int row)
{
    std::fstream ifs;
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
    int tile_widthcount, tile_heightcount;
    // read image width
    ifs.read(reinterpret_cast<char*>(&imgWidth), 4);
    ifs.read(reinterpret_cast<char*>(&imgHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileWidth), 4);
    ifs.read(reinterpret_cast<char*>(&tileHeight), 4);
    ifs.read(reinterpret_cast<char*>(&tileCount), 4);
    int numRows = (imgHeight - 1) / tileHeight + 1;
    int numColumns = (imgWidth - 1) / tileWidth + 1;

    int tileIndex = 0;
    if(tileWidth == 64 && tileHeight == 1){
        tile_widthcount = imgWidth*imgHeight / tileWidth;
        tileIndex = col;
        if (row != 0) {
            ifs.close();
            std::cout << "ERROR: compressed data is line tile compress ,please input row value to 0" << tile_heightcount << std::endl;
            return ERROR_INVALID_PARAM;
        }
        if (col > tile_widthcount) {
            ifs.close();
            std::cout << "ERROR: col out of range width , width range is 0 ~ " << tile_widthcount << std::endl;
            return ERROR_INVALID_PARAM;
        }

    }else{
        tile_widthcount = (imgHeight - 1) / tileHeight + 1;
        tile_heightcount = (imgWidth - 1) / tileWidth + 1;
        tileIndex = row * numRows + col;
        if (col > tile_widthcount) {
            ifs.close();
            std::cout << "ERROR: col out of range width , width range is 0 ~ " << tile_widthcount << std::endl;
            return ERROR_INVALID_PARAM;
        }
        if (row > tile_heightcount) {
            ifs.close();
            std::cout << "ERROR: row out of range height , width range is 0 ~ " << tile_heightcount << std::endl;
            return ERROR_INVALID_PARAM;
        }

    }
    int tileDataStartPos = 24 + 8 * tileCount;
    int new_size=0;
    int size_diff=0;
    unsigned char * new_data = new unsigned char [1024];
    unsigned char * new_compress_data = new unsigned char [numRows * numColumns * tileWidth * tileHeight * BYTES_PER_PIXEL];
    int * tile_offset = new int [numRows * numColumns];
    int * tile_size = new int [numRows * numColumns];


    int total_length=0;
    for(int i = 0;i < tileIndex;i ++){
        int tileInfoOffset = 24 + 8 * i;
        int tileDataStartPos = 24 + 8 * tileCount;
        int tileDataOffset = 0;
        int tileDataBytes = 0;
        ifs.seekg(tileInfoOffset);
        ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
        ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
        tile_offset[i] = tileDataOffset;
        tile_size[i] = tileDataBytes;
        ifs.seekg(tileDataStartPos + tileDataOffset);
        ifs.read(readBuffer, tileDataBytes);
        for(int index = 0;index<tileDataBytes;index++) {
            new_compress_data[index+total_length] = readBuffer[index];
        }
        //new_compress_data += tileDataBytes;
        total_length += tileDataBytes;
    }

    int tileDataOffset = 0;
    int tileDataBytes = 0;
    ifs.seekg(24 + 8 * tileIndex);
    ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
    ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
    // compress
    if(tileWidth == 4 && tileHeight == 4) argb2tile4(writetilein, new_data, &new_size);
    else if(tileWidth == 8 && tileHeight == 8) argb2tile(writetilein, new_data, &new_size);
    else if(tileWidth == 16 && tileHeight == 16) argb2tile16(writetilein, new_data, &new_size);
    else if(tileWidth == 64 && tileHeight == 1) argb2tile512(writetilein, new_data, &new_size);
    size_diff = new_size - tileDataBytes;
    for(int index = 0;index<new_size;index++) {
        new_compress_data[index+total_length] = new_data[index];
    }
    total_length += new_size;
    tile_offset[tileIndex] = tileDataOffset;
    tile_size[tileIndex] = new_size;


    for(int i = tileIndex + 1;i < tileCount;i ++){
        ifs.seekg(24 + 8 * i);
        int tileDataOffset = 0;
        int tileDataBytes = 0;
        ifs.read(reinterpret_cast<char *>(&tileDataOffset), 4);
        ifs.read(reinterpret_cast<char *>(&tileDataBytes), 4);
        tile_offset[i] = tileDataOffset + size_diff;
        tile_size[i] = tileDataBytes;
        ifs.seekg(tileDataStartPos + tileDataOffset);
        ifs.read(readBuffer, tileDataBytes);
        for(int index = 0;index<tileDataBytes;index++) {
            new_compress_data[index+total_length] = readBuffer[index];
        }
        //new_compress_data += tileDataBytes;
        total_length += tileDataBytes;

    }
    ifs.close();
    //-----------------------------------------------write----------------------------------------------//
    std::ofstream ofs;
    ofs.open(compressedFileName, std::ios::binary | std::ios::out);

    int fileHeaderSize = 24;
    int tileInfoSize = tileCount * 8;
    int dataOffsetInFile = fileHeaderSize + tileInfoSize;
    if (ofs.is_open()) {
        ofs.write("JLCD", 4);
        ofs.write(reinterpret_cast<const char *>(&imgWidth), 4);
        ofs.write(reinterpret_cast<const char *>(&imgHeight), 4);
        ofs.write(reinterpret_cast<const char *>(&tileWidth), 4);
        ofs.write(reinterpret_cast<const char *>(&tileHeight), 4);
        ofs.write(reinterpret_cast<const char *>(&tileCount), 4);
        // tile data offset + len
        for (int tileRowIndex = 0; tileRowIndex < numRows; tileRowIndex++) {
            for (int tileColumnIndex = 0; tileColumnIndex < numColumns; tileColumnIndex++) {
                int tileIndex = tileRowIndex * numColumns + tileColumnIndex;
                ofs.write(reinterpret_cast<const char *>(&tile_offset[tileIndex]), 4);
                ofs.write(reinterpret_cast<const char *>(&tile_size[tileIndex]), 4);
            }
        }
        ofs.flush();
        // all tile data
        ofs.write(reinterpret_cast<const char *>(new_compress_data), total_length);
        ofs.close();
    } else {
        std::cout << "fail to open output file(" << compressedFileName << ")" << std::endl;
    }
    delete[] new_data;
    delete[] new_compress_data;

}
int main(int argc, char *argv[]) {
    int     func = 0;
    char const* inFileName = NULL;
    char const* outFileName = NULL;
    int     IsNewBuff = 0;
    int     ret = ERROR_OK;

#define USAGE   "USAGE: fblcd.out [--version] [-{en,de,cp,en16,de16,en4,de4,en512,de512,enline64,deline64,enline256,deline256,enline512,deline512,enline1024,deline1024} infile outfile]"

    if (argc < 2) {
        std::cout << USAGE << std::endl;
        std::cout << "parameters: " << std::endl;
        std::cout << "  --version              show author's info" << std::endl;
        std::cout << "  -en 8*8 compression     -en4 4*4 compression        -en16 16*16 compression     -en512 8*16 compression" << std::endl;
        std::cout << "  -enline64 1*16 compression     -enline256 1*64 compression        -enline512 1*128 compression     -enline1024 1*256 compression" << std::endl;
        std::cout << "  -de 8*8 decompression     -de4 4*4 decompression        -de16 16*16 decompression     -de512 8*16 decompression" << std::endl;
        std::cout << "  -deline64 1*16 decompression     -deline256 1*64 decompression        -deline512 1*128 decompression     -deline1024 1*256 decompression" << std::endl;
        std::cout << "  -en infile outfile     encode(compress) `infile` to `outfile`" << std::endl;
        std::cout << "                           `infile` should be BMP file" << std::endl;
        std::cout << "                           `outfile` is TILE file" << std::endl;
        std::cout << "                           if `outfile` parameter is not specified, it will assigned to `infile` and add JLCD suffix" << std::endl;
        std::cout << "  -de infile outfile     decode(decompress) `infile` to `outfile`" << std::endl;
        std::cout << "                           `infile` should be TILE file" << std::endl;
        std::cout << "                           `outfile` is BMP file" << std::endl;
        std::cout << "                           if `outfile` parameter is not specified, it will assigned to `infile` and add BMP suffix" << std::endl;
        std::cout << "  -cp infile outfile     compare `infile` and `outfile`, pixel by pixel" << std::endl;
        return ERROR_PARAM_NOT_ENOUGH;
    }

    if (strcmp(argv[1], "--version") == 0) {
        std::cout << USAGE << std::endl;
        // !!! REPLACE THE NAME TO YOUR TEAM'S
        std::cout << "made by: 摆浪大队" << std::endl;
        std::cout << "version: " << APP_VERSION << std::endl;
        return ERROR_PARAM_NOT_ENOUGH;
    }
    else if (strcmp(argv[1], "-en") == 0) {
        func = 1;
    }
    else if (strcmp(argv[1], "-de") == 0) {
        func = 2;
    }
    else if (strcmp(argv[1], "-cp") == 0) {
        func = 3;
    }
    else if (strcmp(argv[1], "-en16") == 0) {
        func = 4;
    }
    else if (strcmp(argv[1], "-de16") == 0) {
        func = 5;
    }
    else if (strcmp(argv[1], "-en4") == 0) {
        func = 6;
    }
    else if (strcmp(argv[1], "-de4") == 0) {
        func = 7;
    }
    else if (strcmp(argv[1], "-en512") == 0) {
        func = 8;
    }
    else if (strcmp(argv[1], "-de512") == 0) {
        func = 9;
    }
    else if (strcmp(argv[1], "-enline64") == 0) {
        func = 10;
    }
    else if (strcmp(argv[1], "-deline64") == 0) {
        func = 11;
    }
    else if (strcmp(argv[1], "-enline256") == 0) {
        func = 12;
    }
    else if (strcmp(argv[1], "-deline256") == 0) {
        func = 13;
    }
    else if (strcmp(argv[1], "-enline512") == 0) {
        func = 14;
    }
    else if (strcmp(argv[1], "-deline512") == 0) {
        func = 15;
    }
    else if (strcmp(argv[1], "-enline1024") == 0) {
        func = 16;
    }
    else if (strcmp(argv[1], "-deline1024") == 0) {
        func = 17;
    }
    else {
        return ERROR_INVALID_PARAM;
    }

    if (argc < 3) {
        std::cout << "ERROR: parameter is not enough!" << std::endl;
        return ERROR_PARAM_NOT_ENOUGH;
    }
    inFileName = argv[2];
    if (argc >= 4) {
        outFileName = argv[3];
    }
    else {
        IsNewBuff = 1;
        outFileName = new char[strlen(inFileName) + 6];
        sprintf((char*)outFileName, "%s.%s", inFileName, (1==func) ? "jlcd" : "bmp");
        std::cout << "output file is not assigned, we assign it to: " << outFileName << std::endl;
    }

    if (func == 1) {
        // compress
        ret = compressARGB(inFileName, outFileName);
    } else if (func == 2) {
        // decompress
        ret = decompressARGB(inFileName, outFileName);
    }
    else if (func == 4) {
        // compress
        ret = compressARGB16(inFileName, outFileName);
    }
    else if (func == 5) {
        // decompress
        ret = decompressARGB16(inFileName, outFileName);
    }
    else if (func == 6) {
        // compress
        ret = compressARGB4(inFileName, outFileName);
    }
    else if (func == 7) {
        // decompress
        ret = decompressARGB4(inFileName, outFileName);
    }
    else if (func == 8) {
        // 8*16 compress
        ret = compressARGB512(inFileName, outFileName);
    }
    else if (func == 9) {
        // 8*16 decompress
        ret = decompressARGB512(inFileName, outFileName);
    }
    else if (func == 10) {
        // 1*16 compress
        ret = linecompressARGB64(inFileName, outFileName);
    }
    else if (func == 11) {
        // 1*16 decompress
        ret = linedecompressARGB64(inFileName, outFileName);
    }
    else if (func == 12) {
        // 1*64 compress
        ret = linecompressARGB256(inFileName, outFileName);
    }
    else if (func == 13) {
        // 1*64 decompress
        ret = linedecompressARGB256(inFileName, outFileName);
    }
    else if (func == 14) {
        // 1*128 compress
        ret = linecompressARGB512(inFileName, outFileName);
    }
    else if (func == 15) {
        // 1*128 decompress
        ret = linedecompressARGB512(inFileName, outFileName);
    }
    else if (func == 16) {
        // 1*256 compress
        ret = linecompressARGB1024(inFileName, outFileName);
    }
    else if (func == 17) {
        // 1*256 decompress
        ret = linedecompressARGB1024(inFileName, outFileName);
    }
    else {
        ret = compareBMP(inFileName, outFileName);
    }

    std::cout << "result = " << ret << std::endl;
    if (IsNewBuff) {
        delete[] outFileName;
        outFileName = NULL;
    }
    return ret;
}
//FILE *fp;
//float ratio;
//int main(int argc, char *argv[]) {
//    int     func = 1;
//    char const* inFileName = NULL;
//    char const* outFileName = NULL;
//    int     IsNewBuff = 0;
//    int     ret = ERROR_OK;
//    char* dataPath[33];
//    char* compress[33];
//    char* decompress[33];
//    dataPath[0] = "..\\res\\sample01.bmp";
//     dataPath[1] = "..\\res\\sample02.bmp";
//     dataPath[2] = "..\\res\\sample03.bmp";
//     dataPath[3] = "..\\res\\sample04.bmp";
//     dataPath[4] = "..\\res\\sample05.bmp";
//     dataPath[5] = "..\\res\\sample06.bmp";
//     dataPath[6] = "..\\res\\sample07.bmp";
//     dataPath[7] = "..\\res\\sample08.bmp";
//     dataPath[8] = "..\\res\\sample09.bmp";
//     dataPath[9] = "..\\res\\sample10.bmp";
//     dataPath[10] = "..\\res\\sample11.bmp";
//     dataPath[11] = "..\\res\\sample12.bmp";
//     dataPath[12] = "..\\res\\sample13.bmp";
//     dataPath[13] = "..\\res\\sample14.bmp";
//     dataPath[14] = "..\\res\\sample15.bmp";
//     dataPath[15] = "..\\res\\sample16.bmp";
//     dataPath[16] = "..\\res\\sample17.bmp";
//     dataPath[17] = "..\\res\\sample18.bmp";
//     dataPath[18] = "..\\res\\sample19.bmp";
//     dataPath[19] = "..\\res\\sample20.bmp";
//     dataPath[20] = "..\\res\\sample21.bmp";
//     dataPath[21] = "..\\res\\sample22.bmp";
//     dataPath[22] = "..\\res\\sample23.bmp";
//     dataPath[23] = "..\\res\\sample24.bmp";
//     dataPath[24] = "..\\res\\sample25.bmp";
//     dataPath[25] = "..\\res\\sample26.bmp";
//     dataPath[26] = "..\\res\\sample27.bmp";
//     dataPath[27] = "..\\res\\sample28.bmp";
//     dataPath[28] = "..\\res\\sample29.bmp";
//     dataPath[29] = "..\\res\\sample30.bmp";
//     dataPath[30] = "..\\res\\sample31.bmp";
//     dataPath[31] = "..\\res\\sample32.bmp";
//     dataPath[32] = "..\\res\\sample33.bmp";
//
//    compress[0] = "..\\res\\compress\\sample01.jlcd";
//    compress[1] = "..\\res\\compress\\sample02.jlcd";
//    compress[2] = "..\\res\\compress\\sample03.jlcd";
//    compress[3] = "..\\res\\compress\\sample04.jlcd";
//    compress[4] = "..\\res\\compress\\sample05.jlcd";
//    compress[5] = "..\\res\\compress\\sample06.jlcd";
//    compress[6] = "..\\res\\compress\\sample07.jlcd";
//    compress[7] = "..\\res\\compress\\sample08.jlcd";
//    compress[8] = "..\\res\\compress\\sample09.jlcd";
//    compress[9] = "..\\res\\compress\\sample10.jlcd";
//    compress[10] = "..\\res\\compress\\sample11.jlcd";
//    compress[11] = "..\\res\\compress\\sample12.jlcd";
//    compress[12] = "..\\res\\compress\\sample13.jlcd";
//    compress[13] = "..\\res\\compress\\sample14.jlcd";
//    compress[14] = "..\\res\\compress\\sample15.jlcd";
//    compress[15] = "..\\res\\compress\\sample16.jlcd";
//    compress[16] = "..\\res\\compress\\sample17.jlcd";
//    compress[17] = "..\\res\\compress\\sample18.jlcd";
//    compress[18] = "..\\res\\compress\\sample19.jlcd";
//    compress[19] = "..\\res\\compress\\sample20.jlcd";
//    compress[20] = "..\\res\\compress\\sample21.jlcd";
//    compress[21] = "..\\res\\compress\\sample22.jlcd";
//    compress[22] = "..\\res\\compress\\sample23.jlcd";
//    compress[23] = "..\\res\\compress\\sample24.jlcd";
//    compress[24] = "..\\res\\compress\\sample25.jlcd";
//    compress[25] = "..\\res\\compress\\sample26.jlcd";
//    compress[26] = "..\\res\\compress\\sample27.jlcd";
//    compress[27] = "..\\res\\compress\\sample28.jlcd";
//    compress[28] = "..\\res\\compress\\sample29.jlcd";
//    compress[29] = "..\\res\\compress\\sample30.jlcd";
//    compress[30] = "..\\res\\compress\\sample31.jlcd";
//    compress[31] = "..\\res\\compress\\sample32.jlcd";
//    compress[32] = "..\\res\\compress\\sample33.jlcd";
//
//    decompress[0] = "..\\res\\decompress\\sample01.bmp";
//    decompress[1] = "..\\res\\decompress\\sample02.bmp";
//    decompress[2] = "..\\res\\decompress\\sample03.bmp";
//    decompress[3] = "..\\res\\decompress\\sample04.bmp";
//    decompress[4] = "..\\res\\decompress\\sample05.bmp";
//    decompress[5] = "..\\res\\decompress\\sample06.bmp";
//    decompress[6] = "..\\res\\decompress\\sample07.bmp";
//    decompress[7] = "..\\res\\decompress\\sample08.bmp";
//    decompress[8] = "..\\res\\decompress\\sample09.bmp";
//    decompress[9] = "..\\res\\decompress\\sample10.bmp";
//    decompress[10] = "..\\res\\decompress\\sample11.bmp";
//    decompress[11] = "..\\res\\decompress\\sample12.bmp";
//    decompress[12] = "..\\res\\decompress\\sample13.bmp";
//    decompress[13] = "..\\res\\decompress\\sample14.bmp";
//    decompress[14] = "..\\res\\decompress\\sample15.bmp";
//    decompress[15] = "..\\res\\decompress\\sample16.bmp";
//    decompress[16] = "..\\res\\decompress\\sample17.bmp";
//    decompress[17] = "..\\res\\decompress\\sample18.bmp";
//    decompress[18] = "..\\res\\decompress\\sample19.bmp";
//    decompress[19] = "..\\res\\decompress\\sample20.bmp";
//    decompress[20] = "..\\res\\decompress\\sample21.bmp";
//    decompress[21] = "..\\res\\decompress\\sample22.bmp";
//    decompress[22] = "..\\res\\decompress\\sample23.bmp";
//    decompress[23] = "..\\res\\decompress\\sample24.bmp";
//    decompress[24] = "..\\res\\decompress\\sample25.bmp";
//    decompress[25] = "..\\res\\decompress\\sample26.bmp";
//    decompress[26] = "..\\res\\decompress\\sample27.bmp";
//    decompress[27] = "..\\res\\decompress\\sample28.bmp";
//    decompress[28] = "..\\res\\decompress\\sample29.bmp";
//    decompress[29] = "..\\res\\decompress\\sample30.bmp";
//    decompress[30] = "..\\res\\decompress\\sample31.bmp";
//    decompress[31] = "..\\res\\decompress\\sample32.bmp";
//    decompress[32] = "..\\res\\decompress\\sample33.bmp";
//
//    fp=fopen("D:\\jichuang\\JPEG_LS_OFFICIAL\\line_ratio.txt","w");
//    for(int i=0;i<33;i++)
//    {
//        compressARGB(dataPath[i], compress[i]);
//        fprintf(fp,"%f%\n",ratio);
//        decompressARGB(compress[i], decompress[i]);
//        ret = compareBMP(dataPath[i], decompress[i]);
//        if(ret!=ERROR_OK)
//        {
//            printf("%s is difference\n",dataPath[i]);
//            return 1;
//        }
//    }
//    fclose(fp);
//    return 0;
//}