/*
 *     filename: memload.h
 *     partner 1 name: Jack Burton       Login: jburto05
 *     partner 2 name: James Hartley        Login: jhartl01
 *     date: April 8th, 2024
 *     assignment: hw6
 *
 *     summary: Defines functions for loading instructions from file
 *     into memory.
 *     
*/

#include "memexec.h"

void loadInstructions(Mem_T mem, FILE *fp);
uint32_t packWord(FILE *fp);