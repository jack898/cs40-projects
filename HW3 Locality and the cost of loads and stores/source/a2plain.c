/**************************************************************
 *
 *                         a2plain.c
 *
 *        Assignment: locality
 *        Authors:  Alekha Rao, Jack Burton 
 *        Date:     2/16/24
 *
 *        Implements A2Methods abstraction for UArray2
 *
 **************************************************************/

#include <string.h>

#include <a2plain.h>
#include "uarray2.h"

/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/

/******************* new **********************
 * Purpose: Creates a new empty UArray2 object with given dimensions 
 *          and size
 *
 * Parameters: 
 *              int width: Width of the 2d array
 *              int height: Height of the 2d array
 *              int size: size of each element in the array
 * Return:  
 *        Intialized 2d array object
 * Expects: 
 *      Integers for width, height, and size
 ************************************************/
static A2Methods_UArray2 new(int width, int height, int size)
{
        
        return UArray2_new(width, height, size);
}

/******************* new_with_blocksize **********************
 * Purpose: Abstraction that ignores blocksize and creates
 * a new UArray2 of dimensions width, height, and elements
 * of size "size" in bytes.
 *
 * Parameters: 
 *              int width: Width (number of cols) in returned UArray2   
 *              int height: Height (number of rows) in returned UArray2
 *              int size: Size in bytes for elements of returned UArray2
 *              int blocksize: Block dimensions, which is irrelevant for 
 *              UArray2 therefore unused.
 *              
 * Return:  
 *        An initialized UArray2 of width, height, with elements of size 
 *        "size" in bytes.
 * Expects: 
 *         Integers for width, height, size, and blocksize.
 * 
 ************************************************/
static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}

/******************* a2free **********************
 * Purpose: Frees allocated 
 * memory for given A2methods UArray2 object.
 *
 * Parameters: 
 *              A2Methods_UArray2 *array2p: UArray2 object to be freed.
 *              
 * Return:  
 *        N/A
 * Expects: 
 *         Non-NULL pointer to UArray2 object to free.
 * 
 ************************************************/
static void a2free(A2Methods_UArray2 *array2p)
{
        UArray2_free((UArray2_T *) array2p);
}

/******************* width **********************
 * Purpose: Returns the width of the provided UArray2 object
 *
 * Parameters: 
 *      A2Methods_UArray2 array2: 2d array to get width of

 * Return:  
 *       int containing the width
 * Expects: 
 *       array2 is not NULL
 ************************************************/
static int width(A2Methods_UArray2 array2)
{
        return UArray2_width(array2);
}

/******************* a2free **********************
 * Purpose: Fill in abstraction for UArray2, so that
 * A2Methods can represent a UArray2b OR UArray2.
 *
 * Parameters: 
 *              A2Methods_UArray2 array2: Unused.
 *              
 * Return:  
 *        N/A
 * Expects: 
 *         N/A
 * 
 ************************************************/
static int blocksize(A2Methods_UArray2 array2)
{
        (void) array2;
        return 0;
}

/******************* height **********************
 * Purpose: Returns the height of the provided UArray2 object
 *
 * Parameters: 
 *      A2Methods_UArray2 array2: 2d array to get height of

 * Return:  
 *       int containing the height
 * Expects: 
 *       array2 is not NULL
 ************************************************/
static int height(A2Methods_UArray2 array2)
{
        return UArray2_height(array2);
}

/******************* size **********************
 * Purpose: Returns the size of an element in the provided UArray2 object
 *
 * Parameters: 
 *      A2Methods_UArray2 array2: 2d array to get size of

 * Return:  
 *       int containing the size
 * Expects: 
 *       array2 is not NULL
 ************************************************/
static int size(A2Methods_UArray2 array2)
{
        return UArray2_size(array2);
}

/******************* at **********************
 * Purpose: Returns pointer to element at the  specified (col, row) index
 *
 * Parameters: 
 *              A2Methods_UArray2 array2: 2d array to get element frm
 *              int i: specific column to get element from
 *              int j: specific row to get element from
 * Return:  
 *        A2Methods_Object (void) pointer to the element
 *
 * 
 ************************************************/
static A2Methods_Object *at(A2Methods_UArray2 array2, int i, int j)
{
        return UArray2_at(array2, i, j);
}

typedef void applyfun(int i, int j, UArray2_T array2, void *elem, void *cl);

static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (applyfun*)apply, cl);
}

static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (applyfun*)apply, cl);
}

struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}

static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}


static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,                   
        at,
        map_row_major,        
        map_col_major,          
        NULL,                 //map_block_major
        map_row_major,        // map_default
        small_map_row_major,    
        small_map_col_major,     
        NULL,                   //small_map_block_major
        small_map_row_major,  // small_map_default
};

// finally the payoff: here is the exported pointer to the struct

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
