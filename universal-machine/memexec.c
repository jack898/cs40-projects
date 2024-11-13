/*
 *     filename: memexec.c
 *     partner 1 name: Jack Burton       Login: jburto05
 *     partner 2 name: James Hartley        Login: jhartl01
 *     date: April 8th, 2024
 *     assignment: hw6
 *
 *     summary: Implements functions for executing instructions from memory
 *     
*/

#include "memexec.h"

const int BIT = 1;

/********** execInstructions ********
 * 
 * Read through segment zero, executing each instruction
 * and iterating program counter
 *
 * Parameters:
 *     Mem_T Memory: Pointer to memory struct      		
 *
 * Return: None
 *
 * Expects
 * 	 Non-NULL pointer to initialized memory struct
 * 
 ************************/
void execInstructions(Mem_T mem)
{
        assert(mem != NULL);

        
        uint32_t address = 0;
        int offset = 0;
        int isRunning = true;
        while (isRunning  == true)
        {
                uint32_t encodedWord = getMem(mem, address, offset);
                Um_opcode code = Bitpack_getu(encodedWord, 4*BIT, 28);

                int currOffset = mem->counter->offset;
                isRunning = handleCase(mem, encodedWord, code);
                offset = mem->counter->offset;
                if (offset == currOffset) {
                        offset++;
                }
                address = mem->counter->address;

                shiftProgCounter(mem->counter, address, offset);
        }
         
        

        return;
}

/********** handleCase ********
 * 
 * Executes appropriate instruction based on op code
 *
 * Parameters:
 *     Mem_T mem: Pointer to current um memory
 *     uint32_t encodedWord: Bitpacked instruction word
 *     Um_opcode code: opcode from instruction word  		
 *
 * Return: int indicating SUCCESS (0) or FAILURE (1)
 *
 * Expects
 * 	 Valid opcode 0-13, valid instruction word
 * Notes
 *	Undefined behavior if:
 *              Word does not code for a valid instruction
 *              Segmented load or store refers to unmapped segment
 *              Segment unmap called on $m[0] or an already unmapped segment 
 *              Dividing by zero
 *              Instruction loads program from unmapped segment
 *              Instruction outputs value > 255
 ************************/
int handleCase(Mem_T mem, uint32_t encodedWord, Um_opcode code) {
        if (code == LOADV) {
                int A = Bitpack_getu(encodedWord, 3*BIT, 25);
                int value = Bitpack_getu(encodedWord, 25*BIT, 0);
                loadV(mem, A, value);
        }
        else {
                /* Getting 3 registers */
                int C = Bitpack_getu(encodedWord, 3*BIT, 0);
                int B = Bitpack_getu(encodedWord, 3*BIT, 3);
                int A = Bitpack_getu(encodedWord, 3*BIT, 6);

                switch(code) {
                        
                        case OUT:
                                output(mem, C);
                                break;
                        case IN:
                                input(mem, C);
                                break;
                        case UNMAP:
                                unmapSegment(mem, C);
                                break;
                        case MAP:
                                mapSegment(mem, B, C);
                                break;
                        case LOADP:
                                loadp(mem, B, C);
                                break;
                        case CMOV:
                                condMove(mem, A, B, C);
                                break;
                        case SLOAD:
                                segLoad(mem, A, B, C);
                                break;
                        case SSTORE:
                                segStore(mem, A, B, C);
                                break;
                        case ADD:
                                add(mem, A, B, C);
                                break;
                        case MUL:
                                mult(mem, A, B, C);
                                break;
                        case DIV:
                                divide(mem, A, B, C);
                                break;
                        case NAND:
                                nand(mem, A, B, C);
                                break;
                        case HALT:
                                return false;
                        default:
                                break;
                }
        }
        return true;
}

/********** add ********
 * 
 * Adds registers at index B and C, putting their sum mod 32 into register
 * at index A
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int A: Register A index
 *      int B: Register B index
 *      int C: Register C index  		
 *
 * Return: None
 *
 ************************/
void add(Mem_T mem, int A, int B, int C)
{
        int sum = (getReg(mem, B) + getReg(mem, C));
        setReg(mem, sum, A);
}

/********** mult ********
 * 
 * Multiplies registers at index B and C, putting their product
 * mod 32 into register at index A
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int A: Register A index
 *      int B: Register B index
 *      int C: Register C index  		
 *
 * Return: None
 *
 ************************/
void mult(Mem_T mem, int A, int B, int C)
{
        int prod = getReg(mem, B) * getReg(mem, C);
        setReg(mem, prod, A);
}

/********** divide ********
 * 
 * Divides registers at index B and C, putting the 
 * result into register at index A
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int A: Register A index
 *      int B: Register B index
 *      int C: Register C index  		
 *
 * Return: None
 *
 ************************/
void divide(Mem_T mem, int A, int B, int C)
{
        int prod = getReg(mem, B) / getReg(mem, C);
        setReg(mem, prod, A);
}

/********** nand ********
 * 
 * Performs bitwise NAND of values from registers at index B and C,
 * and puts result in register at index A.
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int A: Register A index
 *      int B: Register B index
 *      int C: Register C index  		
 *
 * Return: None
 *
 ************************/
void nand(Mem_T mem, int A, int B, int C)
{
        uint32_t result = ~(getReg(mem, B) & getReg(mem, C));
        setReg(mem, result, A);
}

/********** condMove ********
 * 
 * If register at index C is nonzero, moves value from index B into
 * register at index A.
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int A: Register A index
 *      int B: Register B index
 *      int C: Register C index  		
 *
 * Return: None
 *
 ************************/
void condMove(Mem_T mem, int A, int B, int C)
{
        if (getReg(mem, C) != 0) {
                setReg(mem, getReg(mem, B), A);
        }
}

/********** loadV ********
 * 
 * Loads val into register at index A
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int A: Register A index
 *      int val: Value to put in register A
 *            		
 *
 * Return: None
 *
 ************************/
void loadV(Mem_T mem, int A, int val)
{
        setReg(mem, val, A);
}


/********** output ********
 * 
 * Prints value from register at index C to I/O device
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int C Register C index		
 *
 * Return: None
 * 
 * Expects
 *       0 <= value <= 255
 *
 ************************/
void output(Mem_T mem, int C)
{
        int value = getReg(mem, C);
        assert(value >= 0 && value <= 255);
        printf("%c", value);
}

/********** input ********
 * 
 * Loads input value into register at index C
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int C: Register C index  		
 *
 * Return: None
 * 
 * Expects
 *      0 <= input value <= 255
 *
 ************************/
void input(Mem_T mem, int C)
{
        char value = fgetc(stdin);
        setReg(mem, value, C);
}

/********** mapSegment ********
 * 
 * Creates new segment with number of words equal to register at index C,
 * initializes each word to 0, and sets register at index B to new
 * register's address.
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int B: Register B index
 *      int C: Register C index  		
 *
 * Return: None
 *
 ************************/
void mapSegment(Mem_T mem, int B, int C)
{
        uint32_t segAddress = mapSeg(mem, getReg(mem, C));
        setReg(mem, segAddress, B);
}

/********** unmapSegment ********
 * 
 * Unmaps segment at address from register at index C
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int C: Register C index  		
 *
 * Return: None
 * 
 * Expects
 *      Segment 0 never unmapped
 *      C address specifies a currently mapped segment
 * 
 * Notes
 *      Undefined behavior to unmap segment 0,
 *      or to unmap a segment that is not mapped
 *
 ************************/
void unmapSegment(Mem_T mem, int C)
{
        unmapSeg(mem, getReg(mem, C));
}

/********** segStore ********
 * 
 * Gets value from segment at address from register at index A, offset 
 * from register at index B, and puts this value into
 * register at index C.
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int A: Register A index
 *      int B: Register B index
 *      int C: Register C index  		
 *
 * Return: None
 *
 ************************/
void segStore(Mem_T mem, int A, int B, int C)
{
        setMem(mem, getReg(mem, C), getReg(mem, A),  getReg(mem, B));     
}

/********** segLoad ********
 * 
 * Sets register at index A to value in word at
 * segment with address from register at index B and
 * offset from register at index C.
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int A: Register A index
 *      int B: Register B index
 *      int C: Register C index  		
 *
 * Return: None
 *
 ************************/
void segLoad(Mem_T mem, int A, int B, int C)
{
        uint32_t value = getMem(mem, getReg(mem, B), getReg(mem, C));
        setReg(mem, value, A); 
}

/********** loadp ********
 * 
 * Duplicates segment at address from value in register at index B,
 * and replaces segment 0 with this. Resets program counter to
 * segment 0 at offset from value in register at index C.
 *
 * Parameters:
 *      Mem_T Memory: Pointer to memory struct
 *      int B: Register B index
 *      int C: Register C index  		
 *
 * Return: None
 *
 ************************/
void loadp(Mem_T mem, int B, int C)
{
        uint32_t address = getReg(mem, B);
        if (address != 0) {
                dupeSeg(mem, address);
        }

        shiftProgCounter(mem->counter, 0, getReg(mem, C));
}