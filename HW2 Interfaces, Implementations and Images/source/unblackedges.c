/**************************************************************
 *
 *                     unblackedges.c
 *
 *     Assignment: iii
 *     Authors:  Jack Burton, Maurice Jang
 *     Date:     Feb 9, 2023
 *
 *    Summary
 *
 *    Provides implementation for unblackedges, which removes black
 *    edges from a provided PBM image.
 *
 **************************************************************/


#include "unblackedges.h"

const int BLACK = 1;
const int WHITE = 0;

int main(int argc, char *argv[])
{
        assert(argc <= 2);
        
        FILE *filePointer;
        if (argc == 1) {
                filePointer = stdin; 
        } else {
                filePointer = fopen(argv[1], "rb");
        }
        assert(filePointer);
        
        Pnmrdr_T pbmImage = Pnmrdr_new(filePointer);


        removeEdges(pbmImage); 

        fclose(filePointer);
} 

/********** removeEdges ********
 *
 * Change all black edge pixels of a pbm to white
 *
 * Parameters:
 *      Pnmrdr_T pbmImage: An Pnmrdr-opened PBM file from user. 
 *
 * Return: None
 *
 * Expects
 *      Properly formatted and non-NULL PBM image
 *      
 * Notes:
 *      Will CRE if header is improperly constructed or pbmImage
 *      does not otherwise follow standard PBM formatting (seen on "man pbm")
 *      CRE if PBM or header is NULL
 * 
 ************************/
void removeEdges(Pnmrdr_T pbmImage) 
{
        assert(pbmImage);
        Pnmrdr_mapdata pbmDetails = Pnmrdr_data(pbmImage);
        assert(validateHeader(pbmDetails));

        
        Bit2_T pbmRaster = Bit2_new(pbmDetails.width, pbmDetails.height); 

        /* Three map calls to read in orig raster, write unblackededge 
        raster, and print unblacked pbm. */
        Bit2_map_row_major(pbmRaster, pbmRead, &pbmImage);
        int pbmDimensions[2] = {pbmDetails.width - 1, pbmDetails.height - 1};
        Bit2_map_row_major(pbmRaster, pbmWrite, pbmDimensions);
        
        printf("P1\n%d %d\n", pbmDetails.width, pbmDetails.height);
        Bit2_map_row_major(pbmRaster, printRaster, &pbmDetails);

        freeMemory(&pbmRaster, &pbmImage);
}

/********** validateHeader ********
 *
 * Validates the header on client-provided file
 * 
 *
 * Parameters:
 *      Pnmrdr_mapdata pbmDetails: Pnmrdr map struct with details about 
 *      client-passed file
 *      
 *
 * Return: Bool indicating whether pbm header is valid--i.e. width and
 * height are > 0, file type is pbm
 *
 * Expects
 *      Initialized Pnmrdr_mapdata struct passed in
 *      
 * Notes:
 *      Pnmrdr will throw CRE if NULL/invalid Pnmrdr_mapdata struct is passed
 *      (i.e. if user provided file is not a pbm or pbm or ppm)  
 *      
 ************************/
bool validateHeader(Pnmrdr_mapdata pbmDetails) 
{
        if (pbmDetails.width == 0 || 
        pbmDetails.height == 0 || pbmDetails.type != 1) {
                return false;
        }

        return true;
}


/********** pbmRead ********
 *
 * Reads each pixel from raster into pbmRaster bit array
 * IF no invalid pixels are found.
 * 
 * If an invalid pixel is found, function will just
 * return until end of mapping.
 *
 * Parameters:
 *      int col: Column in pbmRaster
 *      int row: Row in pbmRaster
 *      Bit2_T pbmRaster: 2D Bit array to store raster from pbm
 *      int pixel: Pixel at (col, row) in pbmRaster
 *      void *pbmImage: Pointer to the pbm image
 *
 * Return: None
 *
 * Expects: A valid pointer to a pbm image
 *      
 *      
 * Notes:  
 *         Will CRE if pbm image raster is improperly formatted (not abiding
 *         by "man pbm" specifications)
 *         pixel is unused and therefore voided
 * 
 ************************/
void pbmRead(int col, int row, Bit2_T pbmRaster, int pixel, void *pbmImage)
{       
        int pixelValue = Pnmrdr_get(*(Pnmrdr_T*)pbmImage); 
        (void) Bit2_put(pbmRaster, col, row, pixelValue);
        (void) pixel; 
      
}

/********** pbmWrite ********
 *
 * Converts black edges and neighboring black pixels to white
 *
 * Parameters:
 *      int col: Column in pbmRaster
 *      int row: Row in pbmRaster
 *      Bit2_T pbmRaster: 2D Bit array to store raster from pbm
 *      int pixel: Pixel at (col, row) in pbmRaster (unused)
 *      void *rasterDim: Pointer to a
 *              2-element array containing the raster width and height
 *
 * Return: None
 *
 * Expects
 *      Properly formatted PBM image along with its width and height contained
 *      in a void pointer
 * 
 * Notes
 *      pixel is unused, thus voided at beginning
 * 
 *      edgePixelIndex, currPixelIndex, and pixelIndexStack are heap-allocated 
 *      in pbmWrite, but also freed within the same loop of this function.
 ************************/
void pbmWrite(int col, int row, Bit2_T pbmRaster, int pixel, void *rasterDim)
{
        (void) pixel;
        int *maxDim = (int*)rasterDim; 
        
        Stack_T pixelIndexStack = Stack_new();  

        if (col == 0 || row == 0 || col == maxDim[0] || row == maxDim[1]) {
                int *edgePixelIndex = CALLOC(2, 4);
                edgePixelIndex[0] = col; 
                edgePixelIndex[1] = row; 
                Stack_push(pixelIndexStack, edgePixelIndex); 
        }
        
        while(!Stack_empty(pixelIndexStack)) {
                int *currPixelIndex = (int*)Stack_pop(pixelIndexStack); 
                if (Bit2_get(pbmRaster, currPixelIndex[0], currPixelIndex[1]) 
                == BLACK) {
                        if (currPixelIndex[0] + 1 <= maxDim[0]) {
                                /* Right neighbor case */
                                Stack_push(pixelIndexStack, 
                                neighborIndex(currPixelIndex, 1, 0));
                        }
                        if (currPixelIndex[1] - 1 >= 0) {
                                /* Lower neighbor case */
                                Stack_push(pixelIndexStack, 
                                neighborIndex(currPixelIndex, 0, -1));
                        }
                        if (currPixelIndex[0] - 1  >= 0) {
                                /* Left neighbor case */
                                Stack_push(pixelIndexStack, 
                                neighborIndex(currPixelIndex, -1, 0));
                        }
                        if (currPixelIndex[1] + 1 <= maxDim[1]) {
                                /* Upper neighbor case */
                                Stack_push(pixelIndexStack, 
                                neighborIndex(currPixelIndex, 0, 1));
                        } 
                        (void) Bit2_put(pbmRaster, currPixelIndex[0], 
                        currPixelIndex[1], WHITE); 
                }
                FREE(currPixelIndex);
        }
        Stack_free(&pixelIndexStack);
}

/********** neighborIndex ********
 *
 * Create and return array storing col and row indices of a neighbor pixel
 * based on provided offset.
 *
 * Parameters:
 *      int *currPixelIndex: Pointer to current pixel indices
 *      int horizOffset: Horizontal offset of selected neighbor pixel
 *      int vertOffset: Vertical offset of selected neighbor pixel
 *      
 *
 * Return: Pointer to array with indices of neighbor pixel
 *
 * Expects
 *      Pointer to an array of indices for a current pixel
 *      horizOffset and vertOffset that are either -1, 0, or 1 for each
 * 
 * Notes
 *     neighborIndices is heap-allocated here, BUT freed by pbmWrite (
 *     specifically by freeing the stack in which it is stored)
 ************************/
int *neighborIndex(int *currPixelIndex, int horizOffset, int vertOffset)
{
      /* Allocating array to store col and row indices of a pixel in raster */
        int *neighborIndices = CALLOC(2, 4);

        neighborIndices[0] = currPixelIndex[0] + horizOffset;
        neighborIndices[1] = currPixelIndex[1] + vertOffset;
        
        return neighborIndices;
}




/********** printRaster ********
 *
 * Print the corrected PBM file raster
 *
 * Parameters:
 *      int col: Column in pbmRaster
 *      int row: Row in pbmRaster
 *      Bit2_T pbmRaster: 2D Bit array to store raster from pbm
 *      int pixel: Pixel at (col, row) in pbmRaster
 *      void *cl: closure variables (set to NULL)
 * 
 * Return: None
 *
 * Expects
 *      Properly formatted PBM image raster
 * 
 * Notes:
 *      Row and closure not used, thus voided at beginning of function.
 * 
 ************************/
void printRaster(int col, int row, Bit2_T pbmRaster, int pixel, void *cl) 
{
        (void) row;
        (void) cl;
        if (col == (Bit2_width(pbmRaster) - 1)) {
                printf("%d ", pixel); 
                printf("\n");
        } else {
                printf("%d ", pixel); 
        }

}




/********** freeMemory ********
 * 
 * Frees non-null array and Pnmrdr object memories
 * 
 *
 * Parameters:
 *      Bit2_T *bitArray: Pointer to a Bit2 object
 *      Pnmrdr_T *pbmImage: Pointer to a Pnmrdr object
 *      
 *
 * Return: None
 *
 * Expects
 *      Pointer to Bit2 and Pnmrdr object (or NULL, in which case the arg
 *      is ignored)
 * 
************************/
void freeMemory(Bit2_T *bitArray, Pnmrdr_T *pbmImage) 
{
        if (bitArray) {
                Bit2_free(bitArray);
        }
        if (pbmImage) {
               Pnmrdr_free(pbmImage);
        }
}

