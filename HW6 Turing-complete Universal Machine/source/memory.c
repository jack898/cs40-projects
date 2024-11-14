/*
 *     filename: memory.c
 *     partner 1 name: Jack Burton       Login: jburto05
 *     partner 2 name: James Hartley        Login: jhartl01
 *     date: April 4th, 2024
 *     assignment: hw6
 *
 *     summary: Implements functions for segmented memory and registers
 *     for a universal machine.
 *     
*/

#include "memory.h"


const int MAPPED = true;
const int UNMAPPED = false;

/********** initMem ********
 * 
 * Creates new empty Mem_T struct of size 
 *
 * Parameters:
 *     int size: Number of words 
 *
 *
 * Return: pointer to initialized Mem_T struct
 * 
 * Notes
 *      memory is heap allocated--freed at end of execution via freeMem()
 *
 ************************/
Mem_T initMem(int size)
{
        Mem_T memory = ALLOC(sizeof(*memory));
        assert(memory != NULL);
        memory->seg = Seq_new(0);
        memory->reg = UArray_new(8, sizeof(uint32_t));

        /* Keeps track of mapped segments */
        memory->segMapped = Seq_new(0);
        
        /* Creates 0th segment with number of words from input file */
        mapSeg(memory, size / 4);

        /* Initialize program counter to $m[0][0] */
        memory->counter = ALLOC(sizeof(ProgCounter));
        shiftProgCounter(memory->counter, 0, 0);
        
        return memory;
}

/********** freeMem ********
 * 
 * Frees entire segmented memory and registers given a memory object
 *
 * Parameters:
 *     Mem_T mem: Pointer to memory struct
 *
 * Return: None
 *
 ************************/
void freeMem(Mem_T mem)
{
        /* Free words from each segment */
        for (int i = 0; i < Seq_length(mem->seg); i++) {
                unmapSeg(mem, i);
        }
        
        /* Free segment mapped tracker and segments themselves */
        Seq_free(&mem->segMapped);
        Seq_free(&mem->seg);

        /* Free registers and struct */
        UArray_free(&mem->reg);
        free(mem->counter);
        free(mem);
        
}



/********** getMem ********
 * 
 * Gets 32-bit word at $m[address][offset]
 *
 * Parameters:
 *     Mem_T memory: Pointer to memory struct
 *     uint32_t address: specifies segment address in memory
 *     int offset: specifies offset in memory
 *
 *
 * Return: uint32_t word
 *
 *
 ************************/
uint32_t getMem(Mem_T mem, uint32_t address, int offset)
{
        assert(mem != NULL);
        Seq_T segment = Seq_get(mem->seg, address);
        return (uint32_t)(uintptr_t)Seq_get(segment, offset);
}

/********** getReg ********
 * 
 * Gets 32-bit word at $r[offset]
 *
 * Parameters:
 *     Mem_T mem: Pointer to UM memory struct
 *     int index: specifies index in registers uarray
 *
 *
 * Return: uint32_t word
 *
 * Expects
 *      0 <= index <= 7
 * 
 ************************/
uint32_t getReg(Mem_T mem, int index)
{
        assert(mem->reg != NULL);
        return *(uint32_t*)UArray_at(mem->reg, index);
}

/********** setMem ********
 * 
 * Sets $m[address][offset] to “word”
 *
 * Parameters:
 *     Mem_T mem: Pointer to memory struct
 *     uint32_t word: Bitpacked instruction
 *     uint32_t address: specifies segment address in memory
 *     int offset: specifies offset in memory
 *
 * Return: None
 *
 *
 ************************/
void setMem(Mem_T mem, uint32_t word, uint32_t address, int offset) 
{
        assert(mem != NULL);
        Seq_put(Seq_get(mem->seg, address), offset, (void*)(uintptr_t)word);
}

/********** setReg ********
 * 
 * Sets $r[index] to “word”
 *
 * Parameters:
 *     Mem_T mem: Pointer to memory struct
 *     uint32_t word: Bitpacked instruction
 *     int index: index of register in uarray
 *
 *
 * Return: None
 *
 * Expects
 *     0 <= index <= 7 (register to be in range r0-r7)
 *
 ************************/
void setReg(Mem_T mem, uint32_t word, int index) 
{
        assert(index >= 0 && index <= 7);

        uint32_t *regPtr = (uint32_t*)UArray_at(mem->reg, index);
        *regPtr = word;
}


/********** mapSeg ********
 * 
 * Maps a new segment with "size" number of words initialized to 0
 * and returns its address
 *
 * Parameters:
 *     Mem_T mem: Pointer to memory struct
 *     int size: Number of words in new segment
 *
 *
 * Return: address of newly mapped segment
 *
 *
 ************************/
uint32_t mapSeg(Mem_T mem, int size) 
{
        /* Create new segment and initialize all words to 0 */
        Seq_T newSeg = Seq_new(size);
        for (int i = 0; i < size; i++) {
                Seq_addhi(newSeg, 0);
        }
        
        /* Use unmapped segment space if it exists */
        int numSegs = Seq_length(mem->segMapped);
        for (int i = 0; i < numSegs; i++) {
                int isMapped = (int)(uintptr_t)Seq_get(mem->segMapped, i);
                if (isMapped == UNMAPPED) {
                        Seq_put(mem->seg, i, newSeg);
                        Seq_put(mem->segMapped, i, (void*)(uintptr_t)MAPPED);
                        return i;
                }
        }


        /* Otherwise add new segment at end */
        Seq_addhi(mem->seg, newSeg);
        Seq_addhi(mem->segMapped, (void*)(uintptr_t)MAPPED);
        return (Seq_length(mem->segMapped) - 1);
}

/********** unmapSeg ********
 * 
 * Unmaps memory segment at specified address
 *
 * Parameters:
 *     Mem_T mem: Pointer to memory struct
 *     uint32_t address: Address within segmented memory to free
 *
 * Return: None
 *
 ************************/
void unmapSeg(Mem_T mem, uint32_t address) {
        Seq_T segment = Seq_get(mem->seg, address);

        /* If segment is mapped, free it */
        if ((int)(uintptr_t)Seq_get(mem->segMapped, address) == 1) {
                Seq_free(&segment);
        }

        Seq_put(mem->segMapped, address, (void*)(uintptr_t)UNMAPPED);
}

/********** dupeSeg ********
 * 
 * Duplicates memory segment at specified address, placing it into segment
 * 0
 *
 * Parameters:
 *     Mem_T mem: Pointer to memory struct
 *     uint32_t address: Address within segmented memory to free
 *
 * Return: None
 *
 ************************/
void dupeSeg(Mem_T mem, uint32_t address)
{
        int size = Seq_length(Seq_get(mem->seg, address));
        unmapSeg(mem, 0);
        mapSeg(mem, size);

        /* Copy contents of segment into segment zero */
        for (int i = 0; i < size; i++) {
                setMem(mem, getMem(mem, address, i), 0, i);
        }      
}

/********** shiftProgCounter ********
 * 
 * Shifts program counter to specified address and offset
 *
 * Parameters:
 *     ProgCounter pg: Program counter object
 *     uint32_t address: Address of memory segment
 *     int offset: Offset within segment
 *
 * Return: None
 *
 ************************/
void shiftProgCounter(ProgCounter pg, uint32_t address, int offset){
        pg->address = address;
        pg->offset = offset;
}
