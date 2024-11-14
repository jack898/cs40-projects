/**************************************************************
 *
 *                     Bit2.h
 *
 *     Assignment: iii
 *     Authors:  Jack Burton, Maurice Jang
 *     Date:     Feb 4, 2023
 *
 *    Summary
 *
 *    This file provides the interface for the implementation of
 *    of a 2 dimensional unboxed array of bits
 *
 **************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <bit.h>
#include <uarray.h>

struct Bit2_T {
        UArray_T majorArray;
        bool isInitialized;
};

typedef struct Bit2_T Bit2_T; 


Bit2_T Bit2_new(int width, int height);
int Bit2_height(Bit2_T array);
int Bit2_width(Bit2_T array);

int Bit2_put(Bit2_T array, int col, int row, int bit);
int Bit2_get(Bit2_T array, int col, int row); 

void Bit2_map_col_major(Bit2_T array,
        void apply(int col, int row, Bit2_T array, int bit, void *cl), 
        void *cl);
void Bit2_map_row_major(Bit2_T array,
        void apply(int col, int row, Bit2_T array, int bit, void *cl), 
        void *cl);

void Bit2_free(Bit2_T *array);


