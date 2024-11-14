/*
 *     filename: um.c
 *     partner 1 name: Jack Burton       Login: jburto05
 *     partner 2 name: James Hartley        Login: jhartl01
 *     date: April 4th, 2024
 *     assignment: hw6
 *
 *     summary: Implements main universal machine driver
 *     
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include "memload.h"



/********** main ********
 * 
 * Opens file for reading, passes file pointer to memload,
 * then executes instructions with memexec.
 *
 * Parameters:
 *     int argc: Number of arguments passed via stdin
 *     char *argv[]: Arguments from stdin
 *
 * Return: None
 *
 * Expects
 * 	 argc == 2
 *     argv[1] to be a valid, readable .um file   
 * Notes
 *    Provides appropriate error messages and exits with EXIT_FAILURE
 *    if expectations are not met.
 *
************************/
int main(int argc, char *argv[]) 
{
        if (argc != 2) {
                fprintf(stderr, "Incorrect number of arguments\n");
                exit(EXIT_FAILURE);
        }


        FILE *input;
        char *filename = argv[1];
        input = fopen(filename, "rb");
        if (input == NULL) {
                fprintf(stderr, "%s: No such file or directory\n", filename);
                exit(EXIT_FAILURE);
        }

        struct stat st;
        stat(filename, &st);
        Mem_T memory = initMem(st.st_size);
        
        
        //testGetAndSetMem(memory);
        // testGetAndSetReg(memory);
        // //testMapSeg(memory);
        // testUnmapSeg(memory);
        // testMapReuseArea(memory);
        //testRandMemAccess(memory);
        loadInstructions(memory, input);
        //printAllWords(Seq_get(memory->seg, 0));
        execInstructions(memory);

        freeMem(memory);

        
        fclose(input);
        
        return EXIT_SUCCESS;
}


