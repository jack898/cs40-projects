/**************************************************************
 *
 *                     uarray2.c
 *
 *         Assignment: iii
 *         Authors:  Alekha Rao, Grace Stapkowski 
 *         Date:         2/12/24
 *
 *        Implementation for UArray2, an unboxed two-dimensional array. 
 *         
 *
 **************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "uarray.h"
#include "mem.h"
#include "uarray2.h"
#include "uarrayrep.h"

#define T UArray2_T

struct T {
        int width;
        int height;
        int size;
        UArray_T uarray;
};

/******************* UArray2_new **********************
 * Purpose: Makes a new empty 2D uarray given dimensions and size
 *
 * Parameters: 
 *	             int width:		width of the 2D array 
 *	             int height: 	        height of the 2D array 
 *	             int size: 	 	the size of each element in the 2d array
 * 
 * Return: T uarray2, the newly created uarray2
 *	
 * Expects: 
 *            width, height are non-negative. size is greater than 0
 * Notes: 
 *           CRE if width, height are negative
 *           CRE if size is less than or equal to 0
 *           Responsibility of caller of UArray2_new to call UAarray2_free 
 *                          to free the UArray2
 ************************************************/

T UArray2_new(int width, int height, int size) 
{
        assert(width >= 0);
        assert(height >= 0);
        assert(size > 0);
        UArray_T outer = UArray_new(width, sizeof(UArray_T));

        /*initializes an inner uarray for each index in the outer uarray*/
        for (int i = 0; i < width; i++) {
                UArray_T inner_array = UArray_new(height, size);

                /* point to slot for inner array */
                UArray_T *innerp = UArray_at(outer, i); 
                *innerp = inner_array;
        }

        /*allocate memory for struct, and initialize all members*/
        T uarray2 = ALLOC(sizeof(struct T));
        assert(uarray2 != NULL);
        uarray2->width = width;
        uarray2->height = height;
        uarray2->size = size;
        uarray2->uarray = outer;

        return uarray2;
}

/******************* UArray2_free **********************
 * Purpose: Frees all memory allocated for the given uarray2
 *
 * Parameters: 
 *		T *uarray2: 	2d array to be freed
 * Return: 
 *	   N/A
 * expects:
 *           uarray2 and *uarray2 are not NULL
 * Notes: 
 *              CRE if uarray2 or *uarray2 are NULL
 *              CRE if the 1d uarray within the uarray2 struct is NULL
 ************************************************/

void UArray2_free(T *uarray2) 
{
        assert(uarray2 != NULL);
        assert(*uarray2 != NULL);

        int length = UArray_length((*uarray2)->uarray);

        /* iterates through the outer uarray and frees each inner array*/
        for(int i = 0; i < length; i++){
                UArray_free(((UArray_T *)UArray_at((*uarray2)->uarray, i)));
        }

        assert((&(*uarray2)->uarray) != NULL);
        UArray_free(&((*uarray2)->uarray));
        FREE(*uarray2);
}

/******************* UArray2_width **********************
 * Purpose: Returns the width of the given uarray2
 *
 * Parameters: 
 *		T uarray2:	2d array to get the width of
 * Return: 
 *	        int containing the width 
 * Expects: 
 *                  uarray2 is not NULL
 * Notes: 
 * 	                CRE if uarray2 is NULL                   
 ************************************************/

int UArray2_width(T uarray2) 
{
        assert(uarray2 != NULL);
        return uarray2->width;
}

/******************* UArray2_height **********************
 * Purpose: Returns the height of the given uarray2
 *
 * Parameters: 
 *		T uarray2:	2d array to get the height of
 * Return: 
 *	        int containing the height 
 * Expects: 
 *                  uarray2 is not NULL
 * Notes: 
 * 	                CRE if uarray2 is NULL 
 ************************************************/

int UArray2_height(T uarray2)
{
        assert(uarray2 != NULL);
        return uarray2->height;
}

/******************* UArray2_size **********************
 * Purpose: Returns the size of each element in the given uarray2
 *
 * Parameters: 
 * 		T uarray2: 2d array to get the size of elements
 *
 * Return: 
 *	   int containing the size of an element 
 * Expects: 
 *                  uarray2 is not NULL
 * Notes: 
 * 	                CRE if uarray2 is NULL 
 ************************************************/

int UArray2_size (T uarray2) 
{
        assert(uarray2 != NULL);
        return uarray2->size;
}

/******************* UArray2_at **********************
 * Purpose: Returns a pointer to the element at a specified (col, row) position
 *
 * Parameters: 
 *		T uarray2: 	2d array to get element from 	
 *		int row:		specific row to get element from
 *		int col:		specific col to get element from
 * Return:  
 *	  A void pointer to the element at a specified (row, col) position
 * 
 * Expects: 
 *                  uarray to be not NULL
 * Notes: 
 *        CRE if uarray2 is NULL
 *	      CRE if row is greater than or equal to the height of the uarray2
 *	      CRE if col is greater than or equal to the width of the uarray2
 *        CRE if the value at [col,row] is NULL
 ************************************************/

void *UArray2_at(T uarray2, int col, int row)
{
        assert(uarray2 != NULL);
        assert(row >= 0 && row < uarray2->height);
        assert(col >= 0 && col < uarray2->width);

        UArray_T temp = *(UArray_T *)UArray_at(uarray2->uarray, col);

        assert(temp != NULL);
        void *curr = UArray_at(temp, row);
        assert(curr != NULL);
        return curr;
}

/*************** Uarray2_map_row_major *******************
 * Purpose: Traverse the given 2d array in row major order
 *
 * Parameters:
 * 		T_uarray2: 2d array to traverse
 * 		void apply(int i, int j, UArray2_T a, void *curr_element, void *cl):
 *			      function to apply in row major order
 * 		void *cl: client-specific void pointer to be used in the apply function
 *
 * Return: N/A
 *
 * Expects: 
 *             uarray2 to not be NULL
 * Notes: 
 *             CRE if uarray2 is NULL
 *	   
 ************************************************/

void UArray2_map_row_major(T uarray2, void apply(int col, int row, 
UArray2_T uarray2, void *curr_element, void *cl), void *cl) 
{
        assert(uarray2 != NULL);

        /*calls the apply function for each position in the uarray2*/
        for (int row = 0; row < uarray2->height; row++) {
                for (int col = 0; col < uarray2->width; col++) {
                        void *curr = UArray2_at(uarray2, col, row);
                        apply(col, row, uarray2, curr, cl);
                }
        }
}

/******************* UArray2_map_col_major **********************
 * Purpose: Traverse the given 2d array in column major order
 *
 * Parameters: 
 *	         T_uarray2: 2d array to traverse
 * 		void apply(int i, int j, UArray2_T a, void *curr_element, void *cl):
 *			      function to apply in row major order
 * 		void *cl: client-specific void pointer to be used in the apply function 
 *
 * Return:  N/A
 * Expects: 
 *           uarray2 to not be NULL
 * Notes: 
 *           CRE if uarray2 is NULL
 ************************************************/

void UArray2_map_col_major(T uarray2, void apply(int col, int row, 
UArray2_T uarray2, void *curr_element, void *cl), void *cl) 
{
        assert(uarray2 != NULL);

        /*calls the apply function for each position in the uarray2 */
        for(int col = 0; col < uarray2->width; col++){
                for(int row = 0; row < uarray2->height; row++){
                        void *curr = UArray2_at(uarray2, col, row);
                        apply(col, row, uarray2, curr, cl);
                }
        }
}