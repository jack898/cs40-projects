/**************************************************************
 *
 *                         uarray2b.c
 *
 *        Assignment: locality
 *        Authors:  Alekha Rao, Jack Burton 
 *        Date:     2/13/24
 *
 *        Implementation for UArray2b, a blocked two-dimensional
 *        polymorphic array.
 *        INVARIANTS:
 *        - Array will be of nonnegative width, positive height, and store
 *        elements of a positive size.
 *        - Array's blocksize will be >= 1
 *         
 *
 **************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <uarray2b.h>
#include <math.h>
#include "uarray2.h"
#include "mem.h"

#define T UArray2b_T

/* Stores total blocked UArray2b width, height, and element size,
   as well as width and height of the outer UArray2 (which is also
   contained), and length of one side of a block*/
struct T {
    int width;
    int height;
    int size;
    int outerWidth;
    int outerHeight;
    int blocksize;
    UArray2_T outerArray;
};

void freeElements(int col, int row, UArray2_T uarray2, void *elem, void *cl);



/******************* UArray2b_new **********************
 * Purpose: Creates a new UArray2b object
 *
 * Parameters: 
 *      int width: Width (eg. number of cols) of UArray2b
 *      int height: Height (eg. number of rows) of UArray2b
 *      int size: Size (in bytes) of each element of the UArray2b
 *      int blocksize: Number of cells on one side of a block
 * Return: 
 *      Initialized UArray2b object
 * Expects: 
 *      Nonnegative width
 *      Positive height and size
 *      Blocksize >= 1
 * Notes: 
 *      CRE if expectations above are not met, or if ALLOC fails.
 ************************************************/
extern T UArray2b_new(int width, int height, int size, int blocksize) 
{
        assert(blocksize >= 1);
        assert(width >= 0);
        assert(height > 0);
        assert(size > 0);
        T blockedArray = ALLOC(sizeof(struct T));
        
        blockedArray->outerWidth = width / blocksize;
        blockedArray->outerHeight = height / blocksize;
        
        if (width % blocksize != 0) {
                blockedArray->outerWidth += 1;
        } 
        if (height % blocksize != 0) {
                blockedArray->outerHeight += 1;
        }

        blockedArray->outerArray = UArray2_new(blockedArray->outerWidth, 
                blockedArray->outerHeight, sizeof(UArray_T));
        blockedArray->width = width;
        blockedArray->height = height;
        blockedArray->size = size;
        blockedArray->blocksize = blocksize;
        
        for (int row = 0; row < UArray2_height(blockedArray->outerArray); 
        row++) 
        {
                for (int col = 0; col < 
                UArray2_width(blockedArray->outerArray); col++) {
                    *(UArray_T*)UArray2_at(blockedArray->outerArray, col, row) 
                    = UArray_new(blocksize * blocksize, size);
                }
        }
        

        return blockedArray;
}


/******************* UArray2b_new_64K_block **********************
 * Purpose: Creates a new UArray2b object with largest possible
 * block size (below 64KB of RAM)
 *
 * Parameters: 
 *      int width: Width (eg. number of cols) of UArray2b
 *      int height: Height (eg. number of rows) of UArray2b
 *      int size: Size (in bytes) of each element of the UArray2b
 *
 * Return: 
 *      Initialized UArray2b object
 * Expects: 
 *      Nonnegative width
 *      Positive height and size
 * Notes: 
 *      CRE if expectations above are not met, or if ALLOC fails.
 *      If size > 64KB, blocksize will default to 1.
 *      Depends on UArray2b_new function
 ************************************************/
extern T UArray2b_new_64K_block(int width, int height, int size) {
        int blocksize;
        assert(size > 0);
        if(size > 64 * 1024) {
                blocksize = 1;
        } else {
                blocksize = sqrt((64 * 1024) / size);
        }

        return UArray2b_new(width, height, size, blocksize);
}

/******************* UArray2b_free **********************
 * Purpose: Frees all memory allocated for the given uarray2b
 *
 * Parameters: 
 *              T *uarray2b: 2d blocked array to be freed
 * Return: 
 *         N/A
 * expects:
 *        uarray2b and *uarray2b are not NULL
 * Notes: 
 *        CRE if uarray2b or *uarray2b are NULL
 ************************************************/
extern void UArray2b_free(T *array2b) {
        assert(array2b != NULL && *array2b != NULL);

        UArray2_map_row_major((*array2b)->outerArray, freeElements, NULL);
        
        UArray2_free(&(*array2b)->outerArray);
        FREE(*array2b);
}

/******************* freeElements **********************
 * Purpose: Helper function to free memory for
 *       each element (UArray) from a UArray2
 *
 * Parameters: 
 *      int col: Column in UArray2 (unused)
 *      int row: Row in UArray2 (unused)
 *      UArray2_T uarray2: UArray2 being mapped on (unused)
 *      void *elem: Pointer to UArray2 element at (col, row)
 *      void *cl: Closure variable (unused)
 * Return: 
 *         N/A
 * Expects:
 *      Initialized, valid UArray2_T object  
 * Notes: 
 *      freeElements is called from a UArray2 map function      
 ************************************************/
 void freeElements(int col, int row, UArray2_T uarray2, void *elem, void *cl) {
        UArray_free((UArray_T*)elem);
        (void) cl;
        (void) col;
        (void) row;
        (void) uarray2;
}

/******************* UArray2b_width **********************
 * Purpose: Returns the width of the given uarray2b
 *
 * Parameters: 
 *        T uarray2b: 2d blocked array to get the width of
 * Return: 
 *        int containing the width 
 * Expects: 
 *         uarray2b is not NULL
 * Notes: 
 *        CRE if uarray2b is NULL                   
 ************************************************/
extern int UArray2b_width (T array2b) {
        assert(array2b != NULL);
        return array2b->width;
}

/******************* UArray2b_height **********************
 * Purpose: Returns the height of the given uarray2b
 *
 * Parameters: 
 *        T uarray2b: 2d blocked array to get the height of
 * Return: 
 *        int containing the heigght 
 * Expects: 
 *         uarray2b is not NULL
 * Notes: 
 *        CRE if uarray2b is NULL                   
 ************************************************/
extern int UArray2b_height (T array2b) {
        assert(array2b != NULL);
        return array2b->height;
}

/******************* UArray2b_size **********************
 * Purpose: Returns the size of the given uarray2b
 *
 * Parameters: 
 *        T uarray2b: 2d blocked array to get the size of
 * Return: 
 *        int containing the size 
 * Expects: 
 *         uarray2b is not NULL
 * Notes: 
 *        CRE if uarray2b is NULL                   
 ************************************************/
extern int UArray2b_size (T array2b) {
        assert(array2b != NULL);
        return array2b->size;
}

/******************* UArray2b_blocksize **********************
 * Purpose: Returns the blocksize of the given uarray2b
 *
 * Parameters: 
 *        T uarray2b: 2d blocked array to get the blocksize of
 * Return: 
 *        int containing the blocksize 
 * Expects: 
 *         uarray2b is not NULL
 * Notes: 
 *        CRE if uarray2b is NULL                   
 ************************************************/
extern int UArray2b_blocksize(T array2b) {
        assert(array2b != NULL);
        return array2b->blocksize;
}


/******************* UArray2b_at **********************
 * Purpose: Returns a pointer to the element at a specified (col, row) position
 *
 * Parameters: 
 *              T uarray2b: 2d blocked array to get element from        
 *              int row: specific row to get element from
 *              int column: specific col to get element from
 * Return:  
 *        A void pointer to the element at a specified (col, row) position
 * 
 * Expects: 
 *           uarrayb to be not NULL
 * Notes: 
 *        CRE if uarray2b is NULL
 *        Calls UArray2_at and UArray_at
 ************************************************/
extern void *UArray2b_at(T array2b, int column, int row) {
        assert(array2b != NULL);
        int blocksize = array2b->blocksize;
        UArray_T *block = UArray2_at(array2b->outerArray, 
                column / blocksize, row / blocksize);
        
        return 
        UArray_at(*block, blocksize * (column % blocksize) + 
        (row % blocksize));
}

/******************* UArray2b_map **********************
 * Purpose: Runs apply function on each element of UArray2b in block-major
 * order (each block is traversed in row-major order)
 *
 * Parameters: 
 *        T uarray2b: 2d blocked array to map over
 *        void apply(int col, int row, T array2b, void *elem, void *cl):
 *        Pointer to function to apply to each element of array2b
 *              * int col: Current col index in array2b
 *              * int row: Current row index in array2b 
 *              * T array2b: 2d blocked array being mapped over
 *              * void *elem: Pointer to element in array2b at index (col, row)
 *              * void *cl: Pointer to client-specified closure
 * Return:
 *       N/a
 * Expects: 
 *         uarray2b is not NULL and apply function pointer is not NULL
 * Notes: 
 *        CRE if uarray2b or apply is NULL                   
 ************************************************/
extern void UArray2b_map(T array2b, void apply(int col, int row, T array2b,
void *elem, void *cl), void *cl) 
{
        assert(array2b != NULL && apply);
        int outerHeight = array2b->outerHeight;
        int outerWidth = array2b->outerWidth;
        int height = array2b->height;
        int width = array2b->width;
        int blocksize = array2b->blocksize;
        
        
        UArray2_T outer = array2b->outerArray;

        for (int outerRow = 0; outerRow < outerHeight; outerRow++) {
                for (int outerCol = 0; outerCol < outerWidth; outerCol++) {
                        UArray_T currBlock = 
                        *(UArray_T*)UArray2_at(outer, outerCol, outerRow);
                        
                        for (int blockIndex = 0; blockIndex < 
                        UArray_length(currBlock); blockIndex++) {

                                int col = (blockIndex / blocksize) + 
                                (outerCol * blocksize);
                                int row = (blockIndex % blocksize) + 
                                (outerRow * blocksize);
                                if(col < width && row < height) {
                                        void *curr = 
                                        UArray_at(currBlock, blockIndex);
                                        apply(col, row, array2b, curr, cl);
                                }
                        }
                }
        }
}


#undef T