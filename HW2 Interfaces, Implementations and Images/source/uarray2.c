/**************************************************************
 *
 *                     uarray2.c
 *
 *     Assignment: iii
 *     Authors:  Jack Burton, Maurice Jang
 *     Date:     Feb 5, 2023
 *
 *    Summary
 *
 *    This file provides the interface for
 *    a 2 dimensional polymorphic unboxed array.
 *
 **************************************************************/
#include "uarray2.h"

const int PTR_SIZE = 8;

/********** UArray2_new ********
 *
 * Allocates, initializes, and returns a polymorphic 2D array of dimensions 
 * width x height, with each element occupying size bytes.
 * 
 *
 * Parameters:
 *      int width: Width (i.e. number of columns) of array
 *      int height: Height (i.e. number of rows) of array
 *      int size: Size of each element, in bytes
 *
 * Return: 2D array of dimensions width x height, elements of "size" bytes
 *
 * Expects
 *      width to be nonnegative, height to be positive
 *      size to be positive
 *      width, height, and size to be non-NULL
 *      
 * Notes:
 *      Width can be 0, but using most other UArray2 functions on a width 0
 *      UArray2 will cause CRE.
 *      Will also CRE if client's arguments do not otherwise meet expectations.
 * 
 ************************/
UArray2_T UArray2_new(int width, int height, int size) 
{
        UArray2_T array;
        array.majorArray = UArray_new(width, PTR_SIZE);
        array.elemSize = size;
        array.isInitialized = true;

        for (int i = 0; i < width; i++) {
                UArray_T minorArray = UArray_new(height, size);
                *((UArray_T*)UArray_at(array.majorArray, i)) = minorArray;
        }
        

        return array; 
}


/********** UArray2_height ********
 *
 * Returns the height of a UArray2 object.
 * 
 *
 * Parameters:
 *      UArray2_T array: A Uarray2 object.
 *
 * Return: Height (number of rows) in a UArray2 object.
 *
 * Expects
 *      An initialized, non-null UArray2 object.
 * 
 * Notes:
 *      CRE if width 0 array is passed
 *      CRE if uninitialized or NULL UArray2 is passed
 * 
 ************************/
int UArray2_height(UArray2_T array)
{
        assert(array.majorArray && array.isInitialized);

        return UArray_length(*(UArray_T*)UArray_at(array.majorArray, 0));
}

/********** UArray2_width ********
 *
 * Returns the width of a UArray2 object.
 * 
 *
 * Parameters:
 *      UArray2_T array: A Uarray2 object.
 *
 * Return: Height (number of columns) in a UArray2 object.
 *
 * Expects
 *      A valid, initialized UArray2 object
 * 
 * Notes:
 *      Will CRE if uninitialized or NULL UArray2 is passed in
 * 
 ************************/
int UArray2_width(UArray2_T array) 
{
        assert(array.majorArray && array.isInitialized);

        return UArray_length(array.majorArray);
}

/********** UArray2_size ********
 *
 * Returns the size (in bytes) of each element of a UArray2 object.
 * 
 *
 * Parameters:
 *      UArray2_T array: A Uarray2 object.
 *
 * Return: Size, in bytes, of the elements for array.
 *
 * Expects
 *      A valid, initialized UArray2 object.
 * 
 * Notes:
 *      Will CRE if uninitialized or NULL UArray2 passed in
 ************************/
int UArray2_size(UArray2_T array) 
{
        assert(array.majorArray && array.isInitialized);

        return array.elemSize; 
}

/********** UArray2_at ********
 *
 * Returns a pointer to element at index (col, row)
 * 
 *
 * Parameters:
 *      UArray2_T array: A UArray2 object.
        int col: Column index in array
 *      int row: Row index in array
 *     
 *
 * Return: Pointer to element at index (col, row)
 *
 * Expects
 *      col and row to be less than the width and height, respectively,
 *      of array.
 *      array to be not NULL and initialized
 * Notes:
 *      Will CRE if client's arguments do not meet expectations.
 * 
 ************************/
void *UArray2_at(UArray2_T array, int col, int row) 
{
        assert(array.majorArray && array.isInitialized);


        UArray_T *columnArray = UArray_at(array.majorArray, col); 
        return UArray_at(*columnArray, row); 
}

/********** UArray2_map_col_major ********
 *
 * Traverses UArray2 with rows varying faster than columns, applying apply()
 * function to each element.
 * 
 *
 * Parameters:
 *      UArray2_T array: A Uarray2 object.
 *      void apply(int col, int row, UArray2_T array, void *element,
 *                 void *cl): Pointer to a function to execute on each 
 *                            element of array.
 *              - int col: Column 
 *              - int row: Row
 *              - UArray2_T array: UArray2 object
 *              - void *element: Pointer to element from UArray2 at (col, row) 
 *              - void *cl: Pointer to client-supplied closure 
 *      void *cl: Pointer to client-supplied closure
 *
 * Return: None
 *
 * Expects
 *      Initialized, non NULL uarray2 object and 
 *      apply function pointer matching specified parameters.
 * 
 * Notes:
 *      Will CRE if passed NULL or uninitialized UArray2 object, or NULL
 *      apply function
 * 
 ************************/
void UArray2_map_col_major(UArray2_T array,
        void apply(int col, int row, UArray2_T array, void *element, void *cl),
        void *cl)
{
        assert(array.majorArray && array.isInitialized && apply);

        for (int col = 0; col < UArray2_width(array); col++) {
                for (int row = 0; row < UArray2_height(array); row++) {
                        apply(col, row, array, UArray2_at(array, col, row), 
                        cl);
                }
        }
 }
        

/********** UArray2_map_row_major ********
 *
 * Traverses UArray2 with columns varying faster than rows, applying apply()
 * function to each element.
 *       
 *
 * Parameters:
 *      UArray2_T array: A Uarray2 object.
 *      void apply(int col, int row, UArray2_T array, void *element,
 *                 void *cl): Pointer to a function to execute on each 
 *                            element of array.
 *              - int col: Column 
 *              - int row: Row
 *              - UArray2_T array: UArray2 object
 *              - void *element: Pointer to element from UArray2 at (col, row) 
 *              - void *cl: Pointer to client-supplied closure 
 *      void *cl: Pointer to client-supplied closure
 *
 * Return: None
 *
 * Expects
 *      Initialized and non NULL uarray2 object and 
 *      apply function pointer matching specified parameters.
 * 
 * Notes:
 *      Will CRE if passed uninitialized or NULL UArray2, or NULL apply func
 * 
 ************************/
void UArray2_map_row_major(UArray2_T array,
        void apply(int col, int row, UArray2_T array, void *element, void *cl),
        void *cl)
{
        assert(array.majorArray && array.isInitialized && apply);


        for (int row = 0; row < UArray2_height(array); row++) {
                for (int col = 0; col < UArray2_width(array); col++) {
                        apply(col, row, array, UArray2_at(array, col, row), 
                        cl); 
                }
        }
} 

/********** UArray2_free ********
 *
 * UArray2 free deallocates and clears *UArray2_T
 * 
 *
 * Parameters:
 *      UArray2_T *array: pointer to a 2D array
 *   
 * Return: void

 * Expects
 *      An initialized, non NULL UArray2
 * Notes:
 *      Will raise checked runtime error if array or *array are null
 * 
 ************************/
void UArray2_free(UArray2_T *array)
{
        assert(array && array->majorArray && array->isInitialized);

        for (int i = 0; i < UArray2_width(*array); i++) {
                UArray_free(UArray_at(array->majorArray, i));
        }

        UArray_free(&(array->majorArray));
}





