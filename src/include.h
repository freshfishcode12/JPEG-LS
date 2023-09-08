//
// Created by 20375 on 2023/5/12.
//

#ifndef MATCH_INCLUDE_H
#define MATCH_INCLUDE_H

#define dataChanels 4
#define BUFFER_LENGHT 16
#define COMPONENTS_COUNT 4
typedef struct
{
    int x;
    int y;
} point;

#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdio.h"

#include "encoder_color_line_ilv.h"
#include "decoder_color_line_ilv.h"
#include "globals.h"

#endif //MATCH_INCLUDE_H
