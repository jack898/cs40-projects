/*
 *     filename: memory.c
 *     partner 1 name: Jack Burton       Login: jburto05
 *     partner 2 name: James Hartley        Login: jhartl01
 *     date: April 4th, 2024
 *     assignment: hw6
 *
 *     summary: Defines functions for a segmented memory and 
 *     registers for a univeral machine.
 *     
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <seq.h>
#include <uarray.h>
#include <mem.h>

typedef UArray_T Reg_T;

/* ProgCounter 
 * Usage: Points to next instruction to execute in form $m[address][offset]
 *
 * Members:
 * 	uint32_t address: Address of segment within memory being executed
 * 	int offset: word within segment to execute
 * 
*/
typedef struct ProgCounter {
	uint32_t address;
	int offset;
} *ProgCounter;


/* Mem_T
 * Usage: Represents a Universal Machine's memory, with segmented memory
 * and registers
 * 
 * Members:
 * 	Seq_T seg: A Hanson sequence of sequences, each containing 32-bit
 * 		instruction words represented as uint32_t's, representing
 * 		the segmented memory.
 * 	Seq_T segMapped: A sequence of booleans keeping track of whether
 * 		a segment is mapped (1) or not (0).
 * 	Reg_T reg: A UArray of 32-bit instruction words, representing the
 * 		8 registers r[0]-r[7].
 * 	ProgCounter counter: Program counter, storing address/offset of next
 * 	instruction to execute
 *
 * A Mem_T object is typedefed to be a pointer to a Mem_T struct instance.
*/
typedef struct Mem_T {
	Seq_T seg;
	Seq_T segMapped;
	Reg_T reg;
	ProgCounter counter;
} *Mem_T;



Mem_T initMem(int size);
void freeMem(Mem_T mem);

/* Memory/register getters and setters */
uint32_t getMem(Mem_T mem, uint32_t address, int offset);
uint32_t getReg(Mem_T mem, int index);
void setMem(Mem_T mem, uint32_t word, uint32_t address, int offset);
void setReg(Mem_T mem, uint32_t word, int index);

/* Segment modifiers */
uint32_t mapSeg(Mem_T mem, int size);
void unmapSeg(Mem_T mem, uint32_t address);
void dupeSeg(Mem_T mem, uint32_t address);

void shiftProgCounter(ProgCounter pg, uint32_t address, int offset);