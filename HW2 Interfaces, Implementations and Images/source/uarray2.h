/**************************************************************
 *
 *                     uarray2.h
 *
 *     Assignment: iii
 *     Authors:  Jack Burton, Maurice Jang
 *     Date:     Feb 4, 2023
 *
 *    Summary
 *
 *    This file provides the interface for the implementation of
 *    of a 2 dimensional polymorphic unboxed array
 *
 **************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <uarray.h>
#include <assert.h>

struct UArray2_T {
        UArray_T majorArray;
        int elemSize;
        bool isInitialized;
};

typedef struct UArray2_T UArray2_T; 




UArray2_T UArray2_new(int width, int height, int size);
int UArray2_height(UArray2_T majorArray);
int UArray2_width(UArray2_T majorArray);
int UArray2_size(UArray2_T majorArray); 
void *UArray2_at(UArray2_T majorArray, int row, int col); 
void UArray2_map_col_major(UArray2_T majorArray,
        void apply(int col, int row, UArray2_T array, void *element, void *cl),
        void *cl);
void UArray2_map_row_major(UArray2_T majorArray,
        void apply(int col, int row, UArray2_T array, void *element, void *cl),
        void *cl);
void UArray2_free(UArray2_T *array);




