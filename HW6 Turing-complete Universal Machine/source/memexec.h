/*
 *     filename: memexec.h
 *     partner 1 name: Jack Burton       Login: jburto05
 *     partner 2 name: James Hartley        Login: jhartl01
 *     date: April 8th, 2024
 *     assignment: hw6
 *
 *     summary: Defines functions for executing instructions from memory
 *     
*/

#include "memory.h"
#include <bitpack.h>

typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, MAP, UNMAP, OUT, IN, LOADP, LOADV
} Um_opcode;

void execInstructions(Mem_T mem);
int handleCase(Mem_T mem, uint32_t encodedWord, Um_opcode code);
int findArgs(Um_opcode code);

/* UM Instruction set */
void loadV(Mem_T mem, int A, int val);
void loadp(Mem_T mem, int B, int C);
void condMove(Mem_T mem, int A, int B, int C);
void add(Mem_T mem, int A, int B, int C);
void mult(Mem_T mem, int A, int B, int C);
void divide(Mem_T mem, int A, int B, int C);
void nand(Mem_T mem, int A, int B, int C);
void input(Mem_T mem, int C);
void output(Mem_T mem, int C);
void mapSegment(Mem_T mem, int B, int C);
void unmapSegment(Mem_T mem, int C);
void segStore(Mem_T mem, int A, int B, int C);
void segLoad(Mem_T mem, int A, int B, int C);