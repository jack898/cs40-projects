/**************************************************************
 *
 *                     bit2.c
 *
 *     Assignment: iii
 *     Authors:  Jack Burton, Maurice Jang
 *     Date:     Feb 6, 2023
 *
 *    Summary
 *
 *    This file provides the interface for
 *    a 2 dimensional unboxed array of bits.
 *
 **************************************************************/
#include "bit2.h"

const int PTR_SIZE = 8;

/********** Bit2_new ********
 *
 * Allocates, initializes, and returns a 2D bit array of dimensions 
 * width x height.
 * 
 *
 * Parameters:
 *      int width: Width (i.e. number of columns) of array
 *      int height: Height (i.e. number of rows) of array
 *
 * Return: 2D bit array of dimensions width x height
 *
 * Expects
 *      width to be nonnegative, height to be positive
 *      size to be positive
 *      
 * Notes:
 *      Width can be 0, but using most other Bit2_T functions on a width 0
 *      Bit2_T will cause CRE.
 *      Will also CRE if client's arguments do not otherwise meet expectations.
 * 
 ************************/
Bit2_T Bit2_new(int width, int height) 
{
        Bit2_T array;
        array.majorArray = UArray_new(width, PTR_SIZE);
        array.isInitialized = true;

        for (int i = 0; i < width; i++) {
                Bit_T minorArray = Bit_new(height);
                *((Bit_T*)UArray_at(array.majorArray, i)) = minorArray;
        }
        
        return array; 
}


/********** Bit2_height ********
 *
 * Returns the height of a Bit2_T object.
 * 
 *
 * Parameters:
 *      Bit2_T array: A Bit2_T object.
 *
 * Return: Height (number of rows) in a Bit2_T object.
 *
 * Expects
 *      An initialized, non-null Bit2_T object.
 * 
 * Notes:
 *      CRE if width 0 array is passed
 *      CRE if uninitialized or NULL Bit2_T is passed
 * 
 ************************/
int Bit2_height(Bit2_T array)
{
        assert(array.majorArray && array.isInitialized);
        Bit_T temp = *(Bit_T*)UArray_at(array.majorArray, 0);
        
        return Bit_length(temp);
} 

/********** Bit2_width ********
 *
 * Returns the width of a Bit2_T object.
 * 
 *
 * Parameters:
 *      Bit2_T array: A Bit2_T object.
 *
 * Return: Height (number of columns) in a Bit2_T object.
 *
 * Expects
 *      A valid, initialized Bit2_T object
 * 
 * Notes:
 *      Will CRE if uninitialized or NULL Bit2_T is passed in
 * 
 ************************/
int Bit2_width(Bit2_T array)
{
        assert(array.majorArray && array.isInitialized);

        return UArray_length(array.majorArray);
} 


/********** Bit2_get ********
 *
 * Returns the bit at index (col, row) from a Bit2 array
 * 
 *
 * Parameters:
 *      Bit2_T array: A Bit2_T object.
 *
 * Return: Bit at (col, row) from array
 *
 * Expects
 *      A non-NULL, initialized Bit2_T object
 *      int col and int row within array bounds
 * 
 * Notes:
 *      Will CRE if uninitialized or NULL Bit2_T is passed in
 *      CRE if col and row are greater than array bounds
 * 
 ************************/
int Bit2_get(Bit2_T array, int col, int row)
{
        assert(array.majorArray && array.isInitialized);

        return Bit_get(*(Bit_T*)UArray_at(array.majorArray, col), row);
}

/********** Bit2_put ********
 *
 *   Puts a value in position (col, row) and returns the overwritten value
 * 
 *
 * Parameters:
 *      Bit2_T array: A Bit2_T object.
 *
 * Return: Previous bit from (col,row) in array
 *
 * Expects
 *      A non-NULL initialized Bit2_T object
 *      int row and int col within bounds
 * 
 * Notes:
 *      Will CRE if uninitialized or NULL Bit2_T is passed in
 *      CRE if col and row are greater than array bounds
 * 
 ************************/
int Bit2_put(Bit2_T array, int col, int row, int bit)
{
        assert(array.majorArray && array.isInitialized);
        
        int prev = Bit2_get(array, col, row); 
        
        Bit_put(*(Bit_T*)UArray_at(array.majorArray, col), row, bit); 
        return prev; 
}



/********** Bit2_map_col_major ********
 *
 * Traverses Bit2_T with rows varying faster than columns, applying apply()
 * function to each element.
 * 
 *
 * Parameters:
 *      Bit2_T array: A Bit2_T object.
 *      void apply(int col, int row, Bit2_T array, void *element,
 *                 void *cl): Pointer to a function to execute on each 
 *                            element of array.
 *              - int col: Column 
 *              - int row: Row
 *              - Bit2_T array: Bit2_T object
 *              - void *element: Pointer to element from Bit2_T at (col, row) 
 *              - void *cl: Pointer to client-supplied closure 
 *      void *cl: Pointer to client-supplied closure
 *
 * Return: None
 *
 * Expects
 *      Initialized, non NULL Bit2_T object and 
 *      apply function pointer matching specified parameters.
 * 
 * Notes:
 *      Will CRE if passed NULL or uninitialized Bit2_T object
 * 
 ************************/
void Bit2_map_col_major(Bit2_T array,
        void apply(int col, int row, Bit2_T array, int bit, void *cl), 
        void *cl)
{
        assert(array.majorArray && array.isInitialized && apply);

        for (int col = 0; col < Bit2_width(array); col++) {
                for (int row = 0; row < Bit2_height(array); row++) {
                        apply(col, row, array, Bit2_get(array, col, row), cl);
                }
        }
}
        

/********** Bit2_map_row_major ********
 *
 * Traverses Bit2_T with columns varying faster than rows, applying apply()
 * function to each element.
 *       
 *
 * Parameters:
 *      Bit2_T array: A Bit2_T object.
 *      void apply(int col, int row, Bit2_T array, void *element,
 *                 void *cl): Pointer to a function to execute on each 
 *                            element of array.
 *              - int col: Column 
 *              - int row: Row
 *              - Bit2_T array: Bit2_T object
 *              - void *element: Pointer to element from Bit2_T at (col, row) 
 *              - void *cl: Pointer to client-supplied closure 
 *      void *cl: Pointer to client-supplied closure
 *
 * Return: None
 *
 * Expects
 *      Initialized and non NULL Bit2_T object and 
 *      apply function pointer matching specified parameters.
 * 
 * Notes:
 *      Will CRE if passed uninitialized or NULL Bit2_T
 * 
 ************************/
void Bit2_map_row_major(Bit2_T array,
        void apply(int col, int row, Bit2_T array, int bit, void *cl), 
        void *cl)
{
        assert(array.majorArray && array.isInitialized && apply);

        for (int row = 0; row < Bit2_height(array); row++) {
                for (int col = 0; col < Bit2_width(array); col++) {
                        apply(col, row, array, Bit2_get(array, col, row), cl);
                }
        }
} 

/********** Bit2_free ********
 *
 * Bit2_T free deallocates and clears *Bit2_T
 * 
 *
 * Parameters:
 *      Bit2_T *array: pointer to a 2D array
 *   
 * Return: void

 * Expects
 *      An initialized, non NULL Bit2_T
 * Notes:
 *      Will raise checked runtime error if array or *array are null
 * 
 ************************/
void Bit2_free(Bit2_T *array)
{
        assert(array && array->majorArray && array->isInitialized);

        for (int i = 0; i < Bit2_width(*array); i++) {
                Bit_free(UArray_at(array->majorArray, i));
        }

        UArray_free(&(array->majorArray));
}

