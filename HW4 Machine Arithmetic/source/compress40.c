/*
 *     filename: compress40.c
 *     partner 1 name: Jack Burton       Login: jburto05
 *     partner 2 name: James Hartley        Login: jhartl01
 *     date: February 26th, 2024
 *     assignment: hw4
 *
 *     summary: This file implements functions to compress and decompress
 *     a ppm image.
 *     
 */

#include <string.h>
#include <stdlib.h>
#include "compress40.h"
#include "arith40.h"
#include "a2blocked.h"
#include "a2methods.h"
#include "pnm.h"
#include "assert.h"
#include "bitpack.h"

#define UArray2b A2Methods_UArray2
#define aMethods uarray2_methods_blocked
const int PNMRGB_SIZE = 12;
const int MAX_DENOM = 255;
const float NUM_PIXELS = 4;

/* Pnm_video
 *
 * Purpose: Store video component representations of pixels
 * 
 * float Y: Luminance/luma value for the pixel
 * float Pb, float Pr: Chroma (i.e. color-difference) values for the pixel
 *  
 * Usage: Used in pixelRGBToVideo, which converts pixels from RGB
 * representation to video component.
*/
typedef struct Pnm_video {
        float Y, Pb, Pr;
} Pnm_video;


/* Pnm_scaled
 *
 * Purpose: Store scaled representation of a 2x2 block of pixels
 * 
 * unsigned indexPb, indexPr: Indices to retrieve average quantized Pb/Pr
 *          values for 2x2 pixel block from Arith40_chroma_of_index()
 * unsigned a, int b, c, d: Cosine coefficients from DCT on Y
 * 
 *  
 * Usage: During compression, used in scaleValues to store scaled a/b/c/d and 
 *        indices of average Pb/Pr
 *        During decompression, used to store scaled 
 *        values from decoded codeworrd
 *   
*/
typedef struct Pnm_scaled {
        unsigned a, indexPb, indexPr;
        int b, c, d;
} Pnm_scaled;

/* COMPRESSION FUNCTION HEADERS */
void ppmRGBtoVideo(Pnm_ppm ppmImage);
void pixelRGBtoVideo(int col, int row, UArray2b videoPixels, void *currPixel, 
                     void *origPpm);
void makeCodewords(UArray2b ppmRaster, FILE *filePointer, int col, int row);
Pnm_scaled scaleValues(Pnm_video *pixel1, Pnm_video *pixel2, Pnm_video *pixel3,
                       Pnm_video *pixel4);
void packAndPrint(unsigned indexPb, unsigned indexPr, unsigned a, int b, 
                  int c, int d);
int floatToInt(float value);

/* DECOMPRESSION FUNCTION HEADERS */
void ppmVideoToRGB(Pnm_ppm ppmImage);
void pixelVideoToRGB(int col, int row, UArray2b RGBPixels, 
                     void *currPixel, void *vidPpm);
void deCodeword(UArray2b ppmRaster, FILE *filePointer, int col, int row);
Pnm_scaled readCodeword(FILE *filePointer);
void scaledToPixel(Pnm_video *pixel, Pnm_scaled *scaledBlock, int blockPos);
float constrict(float rgbValue);

/* FUNCTIONS USED IN BOTH */
void map2by2(UArray2b ppmRaster, void apply(UArray2b ppmRaster, 
             FILE *filePointer, int col, int row), FILE *filePointer);



/********** compress40 ********
 *
 * Compresses provided ppm image and writes compressed version to stdout
 *
 * Parameters:
 *      FILE *fp: Pointer to ppm image to read and compress
 *                             
 * Return: None
 *
 * Expects
 *      Non-NULL file pointer opened with rb/r
 *      Correctly formatted ppm image
 * 
 * Notes
 *      origPpm is heap allocated but also freed within this function
 *      CRE if ppm is incorrectly formatted
 * 
 ************************/
void compress40(FILE *fp)
{
        Pnm_ppm origPpm;    
        origPpm = Pnm_ppmread(fp, aMethods);
        ppmRGBtoVideo(origPpm);

        printf("COMP40 Compressed image format 2\n%u %u\n", origPpm->width, 
                origPpm->height);
           
        map2by2(origPpm->pixels, makeCodewords, fp);
        
        Pnm_ppmfree(&origPpm);
}

/********** decompress40 ********
 *
 * Decompresses provided compressed ppm image
 *
 * Parameters:
 *      FILE *fp: Pointer to ppm image to read and decompress
 *                             
 * Return: None
 *
 * Expects
 *      Non-NULL file pointer opened with rb/r
 *      Valid COMP40 compressed image format file
 * 
 * Notes
 *      ppmRaster UArray2b is allocated and freed within this function.
 *      CRE if file passed in is not valid COMP40 compressed image
 * 
 ************************/
void decompress40(FILE *fp)
{
        unsigned height, width;
        int read = fscanf(fp, "COMP40 Compressed image format 2\n%u %u", 
                          &width, &height);
        assert(read == 2);
        int c = getc(fp);
        assert(c == '\n');
                        
        UArray2b ppmRaster =
        aMethods->new_with_blocksize(width, height, sizeof(Pnm_video), 2);
        struct Pnm_ppm pixmap = { .width = width, .height = height,
                                  .denominator = MAX_DENOM, 
                                  .pixels = ppmRaster, .methods = aMethods
                                };

        map2by2(pixmap.pixels, deCodeword, fp);
        ppmVideoToRGB(&pixmap);

        Pnm_ppmwrite(stdout, &pixmap);
        aMethods->free(&(pixmap.pixels));
}

/********** ppmRGBtoVideo ********
 *
 * Converts RGB values (Pnp_rgb) (Pnm_rgb) of all pixels in image 
 * into video componentvalues (Pnm_video),  (Pnm_video)
 * 
 *
 * Parameters:
 *      Pnm_ppm ppmRaster: Ppm image in RGB to convert to video component.
 *                             
 * Return: None
 *
 * Expects
 *      Valid Ppm image
 * 
 * Notes
 *      Pnm_rgb structs for each pixel are freed in this function when replaced
 *      by Pnm_video structs.
 *      Information may be lost if image is of odd dimensions and gets trimmed.
 * 
 ************************/
void ppmRGBtoVideo(Pnm_ppm ppmImage) 
{
        /* Trimming image to even dimensions */
        if (ppmImage->width % 2 == 1) {
                (ppmImage->width)--;
        }
        if (ppmImage->height % 2 == 1) {
                (ppmImage->height)--;
        }

        /* Creating 2x2 blocked UArray */
        UArray2b videoPixels = aMethods->new_with_blocksize
        (ppmImage->width, ppmImage->height, sizeof(Pnm_video), 2);

        aMethods->map_block_major(videoPixels, pixelRGBtoVideo, 
                                  (void *)ppmImage);
        aMethods->free(&(ppmImage->pixels));

        ppmImage->pixels = videoPixels;
}


/********** ppmVideoToRGB ********
 *
 * Convert video component (Pnm_video) pixels in given ppm to 
 * RGB (Pnm_rgb) pixels.
 * 
 *
 * Parameters:
 *      Pnm_ppm ppmImage: Ppm image to covert pixels
 *                             
 * Return: None
 *
 * Expects
 *      Valid ppm image
 * 
 * Notes
 *      Pnm_video structs for each pixel are freed in this function when
 *      replaced by Pnm_rgb structs.
 * 
 ************************/
void ppmVideoToRGB(Pnm_ppm ppmImage) 
{
        UArray2b RGBPixels = aMethods->new(ppmImage->width, ppmImage->height, 
                                           PNMRGB_SIZE);

        aMethods->map_block_major(RGBPixels, pixelVideoToRGB, (void *)ppmImage);

        aMethods->free(&(ppmImage->pixels));
        ppmImage->pixels = RGBPixels;
}


/********** pixelRGBtoVideo ********
 *
 * Converts RGB pixels from ppm to video component representation.
 * 
 *
 * Parameters:
 *      int col: Current column index in videoPixels
 *      int row: Current row index in videoPixels
 *      UArray2b videoPixels: Blocked 2D array to store video component pixels
 *      (unused)
 *      void *currPixel: Pointer to current pixel in video component (new) 
 *      ppm raster
 *      void *origPpm: Pointer to original ppm read in
 *
 * Return: None
 *
 * Expects
 *      Valid pointer to element in video component pixel raster, valid
 *      pointer to original ppm raster
 * Notes
 *      videoPixels is never directly used in this function but neccessary for
 *      this function to be used during mapping, thus voided
 * 
 ************************/
void pixelRGBtoVideo(int col, int row, UArray2b videoPixels, 
                void *currPixel, void *origPpm) 
{ 
        (void) videoPixels;
        Pnm_video *currPixelVideo = (Pnm_video*)currPixel;

        Pnm_rgb currPixelRGB = 
        (Pnm_rgb)aMethods->at(((Pnm_ppm)origPpm)->pixels, col, row);
        
        /* Convert pixel RGB values to floats and scale them */
        float denominator = ((Pnm_ppm)origPpm)->denominator;
        float r = (float)currPixelRGB->red / denominator;
        float g = (float)currPixelRGB->green / denominator;
        float b = (float)currPixelRGB->blue / denominator;

        /* Calculate video components for each RGB pixel */
        currPixelVideo->Y = 0.299 * r + 0.587 * g + 0.114 * b;
        currPixelVideo->Pb = -0.168736 * r - 0.331264 * g + 0.5 * b;
        currPixelVideo->Pr = 0.5 * r - 0.418688 * g - 0.081312 * b;

        currPixel = currPixelVideo;
}

/********** pixelVideoToRGB ********
 *
 * Converts video component pixels to RGB
 * 
 *
 * Parameters:
 *      int col: Current column index in videoPixels
 *      int row: Current row index in videoPixels
 *      UArray2b RGBPixels: Blocked 2D array to store RGB pixels (unused)
 *      void *currPixel: Pointer to current pixel in RGB pixel (new) 
 *      ppm raster
 *      void *vidPpm: Pointer to ppm struct with video pixels
 *
 * Return: None
 *
 * Expects
 *      Valid pointer to element in RGB pixel raster, valid
 *      pointer to video pixel ppm raster
 * 
 * Notes
 *      RGBPixels UArray is never directly used but needed for function
 *      to be called in mapping function
 * 
 ************************/
void pixelVideoToRGB(int col, int row, UArray2b RGBPixels, 
                void *currPixel, void *vidPpm) 
{
        (void) RGBPixels;
        Pnm_rgb currPixelRGB = (Pnm_rgb)currPixel;
        Pnm_video *currPixelVideo = 
        (Pnm_video*)aMethods->at(((Pnm_ppm)vidPpm)->pixels, col, row);

        /* Calculate (scaled) RGB components for each video component pixel */
        float rScaled = 1.0 * currPixelVideo->Y + 0.0 * currPixelVideo->Pb 
                        + 1.402 * currPixelVideo->Pr;
        float gScaled = 1.0 * currPixelVideo->Y - 0.344136 * currPixelVideo->Pb
                        - 0.714136 * currPixelVideo->Pr;
        float bScaled = 1.0 * currPixelVideo->Y + 1.772 * currPixelVideo->Pb 
                        + 0.0 * currPixelVideo->Pr;

        
        /* Convert pixel RGB values to floats and unscale them */
        currPixelRGB->red = constrict(rScaled * MAX_DENOM);
        currPixelRGB->green  = constrict(gScaled * MAX_DENOM);
        currPixelRGB->blue  = constrict(bScaled * MAX_DENOM);

        currPixel = currPixelRGB;
}

/********** constrict ********
 *
 * Constricts RGB values to within range 0..Denominator and scales them
 * up/down to nearest whole number
 * 
 *
 * Parameters:
 *      int col: Current column index in videoPixels
 *      int row: Current row index in videoPixels
 *      UArray2b RGBPixels: Blocked 2D array to store RGB pixels (unused)
 *      void *currPixel: Pointer to current pixel in RGB pixel (new) 
 *      ppm raster
 *      void *vidPpm: Pointer to ppm struct with video pixels
 *
 * Return: None
 *
 * Expects
 *      Valid pointer to element in RGB pixel raster, valid
 *      pointer to video pixel ppm raster
 * 
 * Notes
 *      RGBPixels UArray is never directly used but needed for function
 *      to be called in mapping function
 * 
 ************************/
float constrict(float rgbValue)
{
        
        if (rgbValue < 0.0) {
                return 0.0;
        } else if (rgbValue > MAX_DENOM) {
                return MAX_DENOM;
        }
        
        return (rgbValue + 0.5);
}

/********** map2by2 ********
 *
 * Maps over ppm in 2x2 blocks, running provided apply function on each pixel
 * in a block.
 * 
 *
 * Parameters:
 *      UArray2b ppmRaster: UArray2b to map over.
 *      void apply(UArray2b ppmRaster, 
 *                 FILE *filePointer, int col, int row):
 *      Function to work on 2x2 block of pixels from ppmRaster
 *              - UArray2b ppmRaster: UArray2b being mapped over
 *              - FILE *filePointer: Pointer to opened ppm (optional)
 *              - int col: Current column index in ppmRaster
 *              - int row: Current row index in ppmRaster
 *      FILE *filePointer: Pointer to opened ppm for reading (optional)
 *                             
 * Return: None
 *
 * Expects
 *      Valid UArray2b
 * 
 ************************/
void map2by2(UArray2b ppmRaster, void apply(UArray2b ppmRaster, 
             FILE *filePointer, int col, int row), FILE *filePointer)
{
        for (int row = 0; row < (aMethods->height(ppmRaster) - 1); row += 2) {
                for (int col = 0; col < (aMethods->width(ppmRaster) - 1); 
                col += 2) 
                {
                        apply(
                                ppmRaster,
                                filePointer,
                                col,
                                row
                        );
                }
        }
}

/********** makeCodewords ********
 *
 * Maps over ppm in 2x2 blocks, creating DCT values and computing average
 * Pb/Pr, then calling helper to make codeword with scaled DCT values.
 * 
 * 
 *
 * Parameters:
 *      UArray2b ppmRaster: Image raster to map over
 *      FILE *filePointer: (unused in this function) pointer to opened ppm
 *      int col: Current column index in ppmRaster
 *      int row: Current row index in ppmRaster
 *                             
 * Return: None
 *
 * Expects
 *      Valid ppm raster as uarray2b, valid col and row indices in that
 *      array.
 * 
 * Notes
 *      Information may be lost when averaging Pr/Pb values and when
 *      scaling a/b/c/d values from floats to ints.
 ************************/
void makeCodewords(UArray2b ppmRaster, FILE *filePointer, int col, int row)
{                  
        (void) filePointer;        
        Pnm_video *pixel1 = (Pnm_video*)aMethods->at(ppmRaster, col, row);
        Pnm_video *pixel2 = (Pnm_video*)aMethods->at(ppmRaster, col + 1, row);
        Pnm_video *pixel3 = (Pnm_video*)aMethods->at(ppmRaster, col, row + 1);
        Pnm_video *pixel4 = (Pnm_video*)aMethods->at(ppmRaster, col + 1, 
        row + 1);

        Pnm_scaled scaledBlock = scaleValues(pixel1, pixel2, pixel3, pixel4);
        
        packAndPrint(scaledBlock.indexPb, 
                     scaledBlock.indexPr, 
                     scaledBlock.a, 
                     scaledBlock.b, 
                     scaledBlock.c, 
                     scaledBlock.d);
}


/********** deCodeword ********
 *
 * Maps over ppm in 2x2 blocks, creating video component pixels from
 * codewords
 * 
 * 
 *
 * Parameters:
 *      UArray2b ppmRaster: Image raster to map over
 *      FILE *filePointer: (unused in this function) pointer to opened ppm
 *      int col: Current column index in ppmRaster
 *      int row: Current row index in ppmRaster
 *                             
 * Return: None
 *
 * Expects
 *      Valid ppm raster as uarray2b, valid col and row indices in that
 *      array.
 * 
 * Notes
 *      Information may be lost when averaging Pr/Pb values and when
 *      scaling a/b/c/d values from floats to ints.
 ************************/
void deCodeword(UArray2b ppmRaster, FILE *filePointer, int col, int row)
{
        (void) col;
        (void) row;
        (void)ppmRaster;
        
        Pnm_scaled scaledBlock = readCodeword(filePointer);

        scaledToPixel((Pnm_video*)aMethods->at(ppmRaster, col, row), 
                      &scaledBlock, 1);
        scaledToPixel((Pnm_video*)aMethods->at(ppmRaster, col + 1, row), 
                      &scaledBlock, 2);
        scaledToPixel((Pnm_video*)aMethods->at(ppmRaster, col, row + 1), 
                      &scaledBlock, 3);
        scaledToPixel((Pnm_video*)aMethods->at(ppmRaster, col + 1, row + 1), 
                      &scaledBlock, 4);
}

/********** readCodeword ********
 *
 * 
 * Reads in one codeword from the file, returning a struct of the unpacked data
 * contained. 
 *
 * Parameters:
 *      FILE *filePointer: pointer to file to read codewords from
 *                             
 * Return: 
 *       Pnm_scaled struct containing indexPr and Pb values, a, b, c, and d
 *       values.
 *
 * Expects
 *      Valid file containing codewords.
 * 
 * Notes
 *      none
 ************************/
Pnm_scaled readCodeword(FILE *filePointer) 
{
        uint32_t currCodeword = 0;

        /* Get each 8 bit portion of codeword */
        for (int lsb = 0; lsb <= 24; lsb += 8) {
                currCodeword = Bitpack_newu(currCodeword, 8, lsb, 
                                            fgetc(filePointer));
        }

        /* Retrieve average Pr/Pb indices and scaled a, b, c, d values */
        uint64_t indexPr = Bitpack_getu(currCodeword, 4, 0);
        uint64_t indexPb = Bitpack_getu(currCodeword, 4, 4);
        int64_t d = Bitpack_gets(currCodeword, 5, 8);
        int64_t c = Bitpack_gets(currCodeword, 5, 13);
        int64_t b = Bitpack_gets(currCodeword, 5, 18);
        uint64_t a = Bitpack_getu(currCodeword, 9, 23);

        Pnm_scaled scaledBlock = { .indexPb = indexPb,
                .indexPr = indexPr,
                .a = a, .b = b, .c = c, 
                .d = d
        };

        return scaledBlock;
}


/********** scaleValues ********
 *
 * Converts video component values of 2x2 pixel block into 5-bit signed cosine
 * coefficients a, b, c and d, and 4-bit unsigned index values indexPb and  
 * indexPr
 * 
 * Parameters:
 *      Pnm_video *pixel1: top-left pixel in 2x2 block
 *      Pnm_video *pixel2: top-right pixel in 2x2 block
 *      Pnm_video *pixel3: bottom-left pixel in 2x2 block
 *      Pnm_video *pixel4: bottom-right pixel in 2x2 block
 *                             
 * Return: 
 *       Pnm_scaled struct containing indexPr and Pb values, a, b, c, and d
 *       values.
 *
 * Expects
 *      None
 * 
 * Notes
 *      Information is lost as floats are divided for average and values outside
 *      range.
 ************************/
Pnm_scaled scaleValues(Pnm_video *pixel1, Pnm_video *pixel2, Pnm_video *pixel3,
                       Pnm_video *pixel4) 
{
        
        float avgPr = (pixel1->Pr + pixel2->Pr + pixel3->Pr + pixel4->Pr) 
                       / NUM_PIXELS;
        float avgPb = (pixel1->Pb + pixel2->Pb + pixel3->Pb + pixel4->Pb) 
                       / NUM_PIXELS;

        unsigned aScaled = 511 * ((pixel4->Y + pixel3->Y + pixel2->Y
                                   + pixel1->Y) / NUM_PIXELS);
        float b = (pixel4->Y + pixel3->Y - pixel2->Y - pixel1->Y) 
                   / NUM_PIXELS;
        float c = (pixel4->Y - pixel3->Y + pixel2->Y - pixel1->Y)
                   / NUM_PIXELS;
        float d = (pixel4->Y - pixel3->Y - pixel2->Y + pixel1->Y)
                   / NUM_PIXELS;
        
        Pnm_scaled scaledBlock = { .indexPb = Arith40_index_of_chroma(avgPb),
                 .indexPr = Arith40_index_of_chroma(avgPr),
                 .a = aScaled, .b = floatToInt(b), .c = floatToInt(c), 
                 .d = floatToInt(d)
               };

        return scaledBlock;
}

/********** scaledToPixel ********
 *
 * Creates an individual video pixel based on values from a scaled
 * 2x2 block of pixels.
 * 
 *
 * Parameters:
 *      Pnm_video *pixel: Pointer to a Pnm_video pixel
 *      Pnm_scaled *scaledBlock: Pointer to a scaled block of pixels
 *      int blockPos: Indicates index within 2x2 block (1 through 4)
 * 
 * Return: None
 *
 * Expects
 *      Valid pointer to a Pnm_video struct to modify
 *      Valid pointer to a scaled block to modify
 * 
 ************************/
void scaledToPixel(Pnm_video *pixel, Pnm_scaled *scaledBlock, int blockPos)
{
        float Pr = Arith40_chroma_of_index(scaledBlock->indexPr);
        float Pb = Arith40_chroma_of_index(scaledBlock->indexPb);
        float a = (float)scaledBlock->a / 511;
        float b = (float)(scaledBlock->b) / 50.0;
        float c = (float)(scaledBlock->c) / 50.0;
        float d = (float)(scaledBlock->d) / 50.0;

        float Y = 0;
        if (blockPos == 1) {
                Y = a - b - c + d;
        }
        else if (blockPos == 2) {
                Y = a - b + c - d;
        }
        else if (blockPos == 3) {
                Y = a + b - c - d;
        }
        else if (blockPos == 4) {
                Y = a + b + c + d;
        }
        
        pixel->Y = Y;
        pixel->Pr = Pr;
        pixel->Pb = Pb;
}

/********** packAndPrint ********
 *
 * Packs avg Pb, avg Pr, and scaled int a/b/c/d values into codeword and
 * prints that word to stdout
 * 
 *
 * Parameters:
 *      unsigned indexPb: Index in table for average Pb value for a 2x2 pixel
 *      block
 *      unsigned indexPr: Index in table for average Pr value for a 2x2 pixel
 *      block
 *      unsigned a: Scaled representation of avg brightness
 *      int b: Scaled representation of degree of image getting brighter as
 *      you move from top to bottom
 *      int c: Scaled representation of degree of image getting brighter as
 *      you move from right to left
 *      int d: Scaled representation of degree to which pixels on one diaganol
 *      are brighter than pixels on other diagonal
 *                             
 * Return: None
 *
 * Expects
 *      indexPb and indexPr to be unsigned representable in 4 bits (i.e. 0-15)
 *      a to be unsigned representable in 9 bits (i.e. 0-511)
 *      b, c, d to be signed representable in 5 bits (i.e. -15 to 15)     
 *    
 ************************/
void packAndPrint(unsigned indexPb, unsigned indexPr, unsigned a, int b, 
        int c, int d)
{
        uint32_t codeword = 0;
        codeword = Bitpack_newu(codeword, 4, 0, indexPr);
        codeword = Bitpack_newu(codeword, 4, 4, indexPb);
        codeword = Bitpack_news(codeword, 5, 8, d);
        codeword = Bitpack_news(codeword, 5, 13, c);
        codeword = Bitpack_news(codeword, 5, 18, b);
        codeword = Bitpack_newu(codeword, 9, 23, a);

        /* print codeword to stdout in Big Endian order */
        for (int i = 0; i < 32; i+=8) {
               putchar(Bitpack_getu(codeword, 8, i));
        }
}

/********** floatToInt ********
 *
 * Converts float cosine coefficients to quantized int representation.
 * 
 *
 * Parameters:
 *      Float value to convert to int
 *                             
 * Return: Quantized int
 *
 * Expects
 *      NA
 * 
 * Notes:
        Information is discarded in this step as floats are rounded down when
        returned as ints.
 ************************/
int floatToInt(float value)
{
        if (value < -0.3) {
                return -15;
        } else if (value > 0.3) {
                return 15;
        }

        return value * 50;
}






#undef UArray2b