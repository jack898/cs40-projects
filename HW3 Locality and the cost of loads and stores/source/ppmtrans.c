/**************************************************************
 *
 *                         ppmtrans.c
 *
 *        Assignment: locality
 *        Authors:  Alekha Rao, Jack Burton 
 *        Date:     2/16/24
 *
 *        Transforms a ppm image with specified user rotation
 *
 **************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "cputiming.h"

/* Stores ppm raster after transformation, as well
   as appropriate A2Methods to use for the transformation. */
typedef struct arrayWithMethods {
        A2Methods_UArray2 resultArray;
        A2Methods_T methods;
} arrayWithMethods;

/* Helper function headers follow */
void flipImage(FILE *fp, char *flipDirection, A2Methods_T methods,
A2Methods_mapfun *mapType);
void rotateImage(FILE *ppmImage, int degrees, A2Methods_T methods, 
A2Methods_mapfun *mapType, bool timed, char *time_file_name) ;
void rotate90(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails);
void rotate180(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails);
void rotate270(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails);
void transpose(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails);
void flipVertical(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails);
void flipHorizontal(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails);
void modifyPixels(arrayWithMethods *moddedArrayDetails, Pnm_ppm ppmRaster, 
A2Methods_mapfun *mapType, int rotationDegrees, char *flipDirection);
void timeFile(double timespan, char *time_file_name);

const int UNINITIALIZED = -1;


#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (false)

static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[ -flip {horizontal, vertical}]"
                        "[-transponse]"
                        "[-{row,col,block}-major] "
                        "[-time time_file] "
                        "[filename]\n",
                        progname);
        exit(1);
}

int main(int argc, char *argv[])
{
        char *time_file_name = NULL;
        int   rotation       = 0;
        char *flipDirection = NULL;
        int   i;
        FILE *ppmFile;
        bool timed = false;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods != NULL);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180, or 270\n");
                                usage(argv[0]);
                        } 
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }

                } else if (strcmp(argv[i], "-flip") == 0) {
                         if (!(i + 1 < argc)) {      /* no flip value */
                                usage(argv[0]);
                        }
                        flipDirection = argv[++i];
                        if((strcmp(flipDirection, "horizontal") != 0 && 
                        strcmp(flipDirection, "vertical") != 0)) {
                                fprintf(stderr, 
                                "Flip must be horizontal or vertical.\n");
                                usage(argv[0]);
                        } 
                } else if (strcmp(argv[i], "-transpose") == 0) {
                        flipDirection = "diagonal";
                }
                else if (strcmp(argv[i], "-time") == 0) {
                        if (!(i + 1 < argc)) {      /* no time file */
                                usage(argv[0]);
                        }
                        time_file_name = argv[++i];
                        timed = true;
                        
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                        usage(argv[0]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        break;
                }
        }
        
        /* open file at argv[i] or read from stdin if file not provided */
        if (argv[i] != NULL) {
                ppmFile = fopen(argv[i], "r");
                if (ppmFile == NULL) {
                        fprintf(stderr, "Failed to open file %s\n", argv[i]);
                        exit(EXIT_FAILURE);
                }
        } else {
                ppmFile = stdin; 
        }
        
        if(flipDirection != NULL) {
                flipImage(ppmFile, flipDirection, methods, map);
        } else {
                rotateImage(ppmFile, rotation, methods, map, timed, 
                time_file_name);
        }

        fclose(ppmFile);                                                       
        exit(EXIT_SUCCESS);
}


/******************* rotateImage **********************
 * Purpose: Reads in ppm image, calls helper to rotate it with specified
 * degrees and with/without timing depending on user args
 * 
 * Parameters: 
 *      FILE *ppmImage: Pointer to a file opened for reading
 *      int degrees: Degrees to rotate provided ppm
 *      A2Methods_T methods: Methods to use for rotation
 *      A2Methods_mapfun *mapType: Type of mapping to use for rotation
 *      bool timed: Indicates whether to time rotation or not
 *      char *time_file_name: Filename to write timing output to
 * Return: 
 *      N/A
 * Expects: 
 *      Pointer to valid ppm file opened for reading, valid A2 methods and 
 *      map function
 * Notes: 
 *      CRE from pnm.h if expectations above are not met
 ************************************************/
void rotateImage(FILE *ppmImage, int degrees, A2Methods_T methods, 
A2Methods_mapfun *mapType, bool timed, char *time_file_name) {
        Pnm_ppm ppmRaster = Pnm_ppmread(ppmImage, methods);
        A2Methods_UArray2 rotatedRaster; 
        struct arrayWithMethods moddedArrayDetails; 
        moddedArrayDetails.methods = methods;

        if(degrees == 0) {
                moddedArrayDetails.resultArray = ppmRaster->pixels;
        }
        else if (degrees == 90 || degrees == 270) {
                rotatedRaster = methods->new(ppmRaster->height, 
                ppmRaster->width, methods->size(ppmRaster->pixels));
                moddedArrayDetails.resultArray = rotatedRaster;

        } else if(degrees == 180) {
                rotatedRaster = methods->new(ppmRaster->width, 
                ppmRaster->height, methods->size(ppmRaster->pixels));
                moddedArrayDetails.resultArray = rotatedRaster;
        }
        /* Perform rotation, either timed or not */
        if (timed) {
                double timespan;
                CPUTime_T rotationTimer = CPUTime_New();
                CPUTime_Start(rotationTimer);
                modifyPixels(&moddedArrayDetails, ppmRaster, mapType, 
                degrees, NULL);
                timespan = CPUTime_Stop(rotationTimer);
                timeFile(timespan, time_file_name);
                CPUTime_Free(&rotationTimer);
        } else {
                modifyPixels(&moddedArrayDetails, ppmRaster, mapType, 
                degrees, NULL);
        }

        if(degrees != 0) {
                methods->free(&(ppmRaster->pixels)); //free old uarray
                ppmRaster->pixels = moddedArrayDetails.resultArray; 
        }
        
        Pnm_ppmwrite(stdout, ppmRaster);
        Pnm_ppmfree(&ppmRaster);
}

/******************* modifyPixels **********************
 * Purpose: Modifies provided ppm raster with either specified rotation or
 * specified flip.
 *
 * Parameters: 
 *      arrayWithMethods *moddedArrayDetails: Pointer to struct which will 
 *      contain modified ppm raster, as well as details about A2methods for 
 *      the array
 *      Pnm_ppm ppmRaster: Contains original ppm raster
 *      A2Methods_mapfun *mapType: Pointer to correct map function to use
 *      based on client-specified row/column/block mapping and array type
 *      int rotationDegrees: Degrees to rotate pixels in original ppm
 *      char *flipDirection: Direction to flip ppm raster
 *
 * Return: 
 *      N/A
 * Expects: 
 *      ONE of rotationDegrees or flipDirection to be non-NULL
 *      arrayWithMethods to specify a valid initialized array and A2Methods_T
 *      object
 *      ppmRaster to contain a valid ppm image raster
 ************************************************/
void modifyPixels(arrayWithMethods *moddedArrayDetails, Pnm_ppm ppmRaster, 
A2Methods_mapfun *mapType, int rotationDegrees, char *flipDirection) {
        if (flipDirection == NULL) { // if rotating pixels
                if (rotationDegrees == 90) {
                        (*mapType)(ppmRaster->pixels, rotate90, 
                        moddedArrayDetails);
                        
                } else if (rotationDegrees == 180) {
                        (*mapType)(ppmRaster->pixels, rotate180,
                        moddedArrayDetails);
                
                } else if (rotationDegrees == 270) {
                        (*mapType)(ppmRaster->pixels, rotate270, 
                        moddedArrayDetails);

                }
        } else { // otherwise must be flipping/transposing
                if (strcmp(flipDirection, "horizontal") == 0) {
                        (*mapType)(ppmRaster->pixels, flipHorizontal, 
                        moddedArrayDetails);
                } else if (strcmp(flipDirection, "vertical") == 0){
                        (*mapType)(ppmRaster->pixels, flipVertical, 
                        moddedArrayDetails);
                } else if (strcmp(flipDirection, "diagonal") == 0){
                        (*mapType)(ppmRaster->pixels, transpose, 
                        moddedArrayDetails);
                         
                }
        }
        if(rotationDegrees == 90 || rotationDegrees == 270 || 
        (flipDirection != NULL && strcmp(flipDirection, "diagonal") == 0)) {
                int tempWidth = ppmRaster->width;
                ppmRaster->width = ppmRaster->height;
                ppmRaster->height = tempWidth;  
        }

}

/******************* timeFile **********************
 * Purpose: Prints timing information to provided timing output file 
 *
 * Parameters: 
 *      double timespan: timing information to output
 *      char *time_file_name: name of output file
 * Return: 
 *      N/a 
 * Notes: 
 *      Uses fprintf to print to the output file. 
 ************************************************/
void timeFile(double timespan, char *time_file_name) {
        FILE *timings_file = fopen(time_file_name, "a");
        if(timings_file == NULL) {
                fprintf(stderr, "Error: timing file couldn't be opened");
        }
        fprintf(timings_file, "Rotation Time: %.0f\n", timespan);
}

/******************* rotate90 **********************
 * Purpose: Rotate pixels in a given image raster by 90 degrees clockwise
 *
 * Parameters: 
 *      int col: Current column index in imgRaster
 *      int row: Current row index in imgRaster
 *      A2Methods_UArray2 imgRaster: Image raster array, either a 
 *      UArray2 or UArray2b
 *      A2Methods_Object *currPixel: Pointer to pixel at (col, row) in 
 *      imgRaster
 *      void *arrayDetails: Pointer to struct with method information about
 *      provided A2Methods_UArray2
 * Return: 
 *      N/A
 * Expects: 
 *      Valid indices in imgRaster, valid pointer to a pixel in imgRaster
 *
 ************************************************/
void rotate90(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails) 
{
        /* Getting appropriate A2Methods for resulting array */
        struct arrayWithMethods methodDetails = 
        *(struct arrayWithMethods*)arrayDetails;
        int newCol = methodDetails.methods->height(imgRaster) - row - 1;
        int newRow = col;

        Pnm_rgb newPixel = (Pnm_rgb)methodDetails.methods->at
        (methodDetails.resultArray, newCol, newRow);
        *newPixel = *(Pnm_rgb)currPixel;
}

/******************* rotate180 **********************
 * Purpose: Rotate pixels in a given image raster by 180 degrees clockwise
 *
 * Parameters: 
 *      int col: Current column index in imgRaster
 *      int row: Current row index in imgRaster
 *      A2Methods_UArray2 imgRaster: Image raster array, either a 
 *      UArray2 or UArray2b
 *      A2Methods_Object *currPixel: Pointer to pixel at (col, row) in 
 *      imgRaster
 *      void *arrayDetails: Pointer to struct with method information about
 *      provided A2Methods_UArray2
 * Return: 
 *      N/A
 * Expects: 
 *      Valid indices in imgRaster, valid pointer to a pixel in imgRaster
 *
 ************************************************/
void rotate180(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails) 
{
        /* Getting appropriate A2Methods for resulting array */
        struct arrayWithMethods methodDetails = 
        *(struct arrayWithMethods*)arrayDetails;
        
        int newCol = methodDetails.methods->width(imgRaster) - col - 1;
        int newRow = methodDetails.methods->height(imgRaster) - row - 1;

        Pnm_rgb newPixel = (Pnm_rgb)methodDetails.methods->at
        (methodDetails.resultArray, newCol, newRow);
        *newPixel = *(Pnm_rgb)currPixel;
}

/******************* rotate270 **********************
 * Purpose: Rotate pixels in a given image raster by 270 degrees clockwise
 *
 * Parameters: 
 *      int col: Current column index in imgRaster
 *      int row: Current row index in imgRaster               
 *      A2Methods_UArray2 imgRaster: Imagge raster array
 *      A2Methods_Object currPixel: Pointer to Current element in 
 *      imgRaster array
 *      void *arrayDetails:  Pointer to struct with methods information about 
 *      provided A2Methods_UArray2
 * 
 * Return: 
 *      N/a
 * Expects: 
 *      Valid indices in the imgRaster, valid pointer to a pixel in imgRaster   
 ************************************************/
void rotate270(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails) 
{
        /* Getting appropriate A2Methods for resulting array */
        struct arrayWithMethods methodDetails = 
        *(struct arrayWithMethods*)arrayDetails;
        
        int newCol = row;
        int newRow = methodDetails.methods->width(imgRaster) - col - 1;

        Pnm_rgb newPixel = (Pnm_rgb)methodDetails.methods->at
        (methodDetails.resultArray, newCol, newRow);
        *newPixel = *(Pnm_rgb)currPixel;
}

/******************* flipImage **********************
 * Purpose: Reads in ppm image, calls helper to flip it
 * with user-specified flip/transposition
 * 
 * Parameters: 
 *      FILE *ppmImage: Pointer to a file opened for reading
 *      char *flipDirection: String containing direction to flip ppm raster
 *      A2Methods_T methods: Methods to use for rotation
 *      A2Methods_mapfun *mapType: Type of mapping to use for rotation
 * Return: 
 *      N/A
 * Expects: 
 *      Pointer to valid ppm file opened for reading, valid A2 methods and 
 *      map function
 * Notes: 
 *      CRE from pnm.h if expectations above are not met
 *      Transposition is essentially a flip across the 
 *      origin, or a "diagonal" flip, so we call it that.
 ************************************************/
void flipImage(FILE *ppmImage, char *flipDirection, A2Methods_T methods,
A2Methods_mapfun *mapType) {
        Pnm_ppm ppmRaster = Pnm_ppmread(ppmImage, methods);
        struct arrayWithMethods flipRasterDetails; 
        flipRasterDetails.methods = methods;

        A2Methods_UArray2 flippedRaster; 
        
        
        if(strcmp(flipDirection, "horizontal") == 0 || 
        strcmp(flipDirection, "vertical") == 0) {
                flippedRaster = methods->new(ppmRaster->width, 
                ppmRaster->height, methods->size(ppmRaster->pixels));
                flipRasterDetails.resultArray = flippedRaster;
   
        }  /* "diagonal" flip means transposing */
        else if(strcmp(flipDirection, "diagonal") == 0) {
                flippedRaster = methods->new(ppmRaster->height, 
                ppmRaster->width, methods->size(ppmRaster->pixels));
                flipRasterDetails.resultArray = flippedRaster;
                 
        }
        modifyPixels(&flipRasterDetails, ppmRaster, mapType, 
        UNINITIALIZED, flipDirection);

        methods->free(&(ppmRaster->pixels)); //free old uarray
        ppmRaster->pixels = flipRasterDetails.resultArray; 
        
        Pnm_ppmwrite(stdout, ppmRaster);
        Pnm_ppmfree(&ppmRaster);

}

/******************* flipHorizontal **********************
 * Purpose: Flip pixels in a given image raster horizontally
 *
 * Parameters: 
 *      int col: Current column index in imgRaster
 *      int row: Current row index in imgRaster
 *      A2Methods_UArray2 imgRaster: Image raster array, either a 
 *      UArray2 or UArray2b
 *      A2Methods_Object *currPixel: Pointer to pixel at (col, row) in 
 *      imgRaster
 *      void *arrayDetails: Pointer to struct with method information about
 *      provided A2Methods_UArray2
 * Return: 
 *      N/A
 * Expects: 
 *      Valid indices in imgRaster, valid pointer to a pixel in imgRaster
 *
 ************************************************/
void flipHorizontal(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails) 
{       
        /* Getting appropriate A2Methods for resulting array */
        struct arrayWithMethods methodDetails = 
        *(struct arrayWithMethods*)arrayDetails;
        
        int newCol = methodDetails.methods->width(imgRaster) - col - 1;
        int newRow = row;

        Pnm_rgb newPixel = (Pnm_rgb)methodDetails.methods->at
        (methodDetails.resultArray, newCol, newRow);
        *newPixel = *(Pnm_rgb)currPixel;
}

/******************* flipVertical **********************
 * Purpose: Flip pixels in a given image raster vertically
 *
 * Parameters: 
 *      int col: Current column index in imgRaster
 *      int row: Current row index in imgRaster
 *      A2Methods_UArray2 imgRaster: Image raster array, either a 
 *      UArray2 or UArray2b
 *      A2Methods_Object *currPixel: Pointer to pixel at (col, row) in 
 *      imgRaster
 *      void *arrayDetails: Pointer to struct with method information about
 *      provided A2Methods_UArray2
 * Return: 
 *      N/A
 * Expects: 
 *      Valid indices in imgRaster, valid pointer to a pixel in imgRaster
 *
 ************************************************/
void flipVertical(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails) 
{
        /* Getting appropriate A2Methods for resulting array */
        struct arrayWithMethods methodDetails = 
        *(struct arrayWithMethods*)arrayDetails;
        
        int newCol = col;
        int newRow = methodDetails.methods->height(imgRaster) - row - 1;

        Pnm_rgb newPixel = (Pnm_rgb)methodDetails.methods->at
        (methodDetails.resultArray, newCol, newRow);
        *newPixel = *(Pnm_rgb)currPixel;

}

/******************* transpose **********************
 * Purpose: Transpose pixels in a given image raster across UL-LR axis
 *
 * Parameters: 
 *      int col: Current column index in imgRaster
 *      int row: Current row index in imgRaster
 *      A2Methods_UArray2 imgRaster: Image raster array, either a 
 *      UArray2 or UArray2b
 *      A2Methods_Object *currPixel: Pointer to pixel at (col, row) in 
 *      imgRaster
 *      void *arrayDetails: Pointer to struct with method information about
 *      provided A2Methods_UArray2
 * Return: 
 *      N/A
 * Expects: 
 *      Valid indices in imgRaster, valid pointer to a pixel in imgRaster
 *
 ************************************************/
void transpose(int col, int row, A2Methods_UArray2 imgRaster, 
A2Methods_Object *currPixel, void *arrayDetails) 
{
        /* Getting appropriate A2Methods for resulting array */
        struct arrayWithMethods methodDetails = 
        *(struct arrayWithMethods*)arrayDetails;
        
        int newCol = row;
        int newRow = col;

        Pnm_rgb newPixel = (Pnm_rgb)methodDetails.methods->at
        (methodDetails.resultArray, newCol, newRow);
        *newPixel = *(Pnm_rgb)currPixel;

        (void) imgRaster;
}