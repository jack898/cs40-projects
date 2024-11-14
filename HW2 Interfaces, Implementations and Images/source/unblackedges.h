/**************************************************************
 *
 *                     unblackedges.h
 *
 *     Assignment: iii
 *     Authors:  Jack Burton, Maurice Jang
 *     Date:     Feb 9, 2023
 *
 *    Summary
 *
 *   Provides interface for functions for unblackedges, which
 *   removes black edges from a PBM image
 *
 **************************************************************/


#include "bit2.h"
#include <pnmrdr.h>
#include <stack.h>
#include <mem.h>


int main(int argc, char *argv[]);
void removeEdges(Pnmrdr_T pbmImage);
bool validateHeader(Pnmrdr_mapdata pbmDetails);
void freeMemory(Bit2_T *bitArray, Pnmrdr_T *pbmImage);
void pbmRead(int col, int row, Bit2_T pbmRaster, int pixel, void *pbmImage);
void pbmWrite(int col, int row, Bit2_T pbmRaster, int pixel, void *rasterDim); 
void printRaster(int col, int row, Bit2_T pbmRaster, int pixel, void *cl);
int *neighborIndex(int *currPixelIndex, int horizOffset, int vertOffset);