/*
 *     filename: memload.c
 *     partner 1 name: Jack Burton       Login: jburto05
 *     partner 2 name: James Hartley        Login: jhartl01
 *     date: April 8th, 2024
 *     assignment: hw6
 *
 *     summary: Implements functions for loading instructions from file
 *     into memory.
 *     
*/

#include "memload.h"

const int BYTE = 8 /* bits */;

/********** loadInstructions ********
 * 
 * Read through .um file, pushing each instruction into segment zero
 *
 * Parameters:
 *     Mem_T memory: Pointer to initialized empty memory
 *     FILE *fp: Pointer to opened .um file
 *
 * Return: None
 *
 * Expects
 * 	 Non-NULL pointer to valid .um file
 *
 ************************/
void loadInstructions(Mem_T mem, FILE *fp)
{
        assert(fp != NULL);

        int segZeroLength = Seq_length(Seq_get(mem->seg, 0));
        for (int i = 0; i < segZeroLength; i++) {
                setMem(mem, packWord(fp), 0, i);
        }
}

/********** packWord ********
 * 
 * Reads instruction from file and packs it into 32-bit word in big endian
 * order
 *
 * Parameters:
 *     FILE *fp: Pointer to opened .um file
 *
 * Return: bitpacked instruction word
 *
 ************************/
uint32_t packWord(FILE *fp) {
        uint32_t word;

        /* Grab one word (4 chars) */
        for (int i = 3; i >= 0; i--) {
                word = Bitpack_newu(word, BYTE, BYTE*i, getc(fp));
        }

        return word;
}