/*
 *     filename: bitpack.c
 *     partner 1 name: Jack Burton       Login: jburto05
 *     partner 2 name: James Hartley        Login: jhartl01
 *     date: February 28th, 2024
 *     assignment: hw4
 *
 *     summary: Implements Bitpack interface to perform various packing
 *     and unpacking operations on integers.
 *     
 */

#include "bitpack.h"
#include <stdio.h>
#include "assert.h"

Except_T Bitpack_Overflow = { "Overflow packing bits" };
Except_T Bitpack_Shift64 = { "Attempted to shift by 64" };

const unsigned MAX_INT_LENGTH = 64;

/********** Bitpack_fitsu ********
 *
 * Tests if an unsigned integer can be represented in a specified number
 * of bits
 * 
 * Parameters:
 *      uint64_t n: 64-bit unsigned integer
 *      unsigned width: Number of bits to fit "n" into
 *                             
 * Return: Bool indicating whether n can be represented in width bits.
 *
 * Expects
 *      Valid unsigned 64 bit integer
 *      Width equal or less than 64.
 * 
 * Notes
 *      CRE for width to be greater than 64
 ************************/
bool Bitpack_fitsu(uint64_t n, unsigned width) 
{ 
        assert(width <= MAX_INT_LENGTH);
        if (width == MAX_INT_LENGTH) {
                RAISE(Bitpack_Shift64);
        }

        return (((uint64_t)1 << width) > n);
}

/********** Bitpack_fitss ********
 *
 * Tests if a signed integer can be represented in specified number of bits
 * 
 * Parameters:
 *      int64_t n: 64-bit signed integer
 *      unsigned width: Number of bits to fit "n" into
 *                             
 * Return: Bool indicating whether n can be represented in width bits.
 *
 * Expects
 *      Valid signed 64 bit integer
 *      Width equal or less than 64.
 * 
 * Notes
 *      CRE for width to be greater than 64
 ************************/
bool Bitpack_fitss(int64_t n, unsigned width) 
{
        assert(width <= MAX_INT_LENGTH);
        if (n == 0) {
                return true;
        }

        int64_t highRange = ((1 << (width - 1)) - 1);
        int64_t lowRange = ~highRange;

        return lowRange <= n && n <= highRange;
}

/********** Bitpack_getu ********
 *
 * Returns specified number of bits as an unsigned int, 
 * starting at specified least significant bit in provided word.
 * 
 * Parameters:
 *      uint64_t word: Bit word to read from
 *      unsigned width: Number of bits to read from word
 *      unsigned lsb: Least significant bit to begin reading from
 *                             
 * Return: Unsigned 64-bit int with specified bits from word
 *
 * Expects
 *      Valid 64-bit unsigned word, a width equal to or less than 64,
 *      and a width + least significant bit less than or equal to 64.
 * 
 * Notes
 *      CRE for width to be greater than 64 or for width + lsb to be
 *      greater than 64.
 ************************/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb) 
{
        assert(width + lsb <= MAX_INT_LENGTH);
        assert(width <= MAX_INT_LENGTH);
        if (width == MAX_INT_LENGTH) {
                RAISE(Bitpack_Shift64);
        }

        uint64_t mask = (((uint64_t)1 << width) << lsb) - 1;
        return (word & mask) >> lsb;
}

/********** Bitpack_gets ********
 *
 * Returns specified number of bits as a signed int, 
 * starting at specified least significant bit in provided word.
 * 
 * Parameters:
 *      uint64_t word: Bit word to read from
 *      unsigned width: Number of bits to read from word
 *      unsigned lsb: Least significant bit to begin reading from
 *                             
 * Return: Signed 64-bit int with specified bits from word
 *
 * Expects
 *      Valid 64-bit unsigned word, a width equal to or less than 64,
 *      and a width + least significant bit less than or equal to 64.
 * 
 * Notes
 *      CRE for width to be greater than 64 or for width + lsb to be
 *      greater than 64.
 ************************/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb) 
{
        assert(width <= MAX_INT_LENGTH);
        assert(width + lsb <= MAX_INT_LENGTH);

        uint64_t highOrderBit = ((uint64_t)1 << (width + lsb - 1));
        
        /* Case where desired bits are a negative int */
        if (word & highOrderBit) {
                int64_t shiftedWord = word << (MAX_INT_LENGTH - (width + lsb));
                int64_t fixed = shiftedWord >> (MAX_INT_LENGTH - width);
                return fixed;
        }

        return Bitpack_getu(word, width, lsb);
}

/********** Bitpack_newu ********
 *
 * Modifies bits in word between lsb to lsb + width into an unsigned value.
 * Returns modified word.
 *
 * Parameters:
 *      uint64_t word: unsigned word to be modified
 *      unsigned width: width of bits to be replaced
 *      unsigned lsb: least significant bit to start modification.
 *      uint64_t value: unsigned value to replace bits in word with
 * 
 *                             
 * Return: unsigned word after modification
 *
 * Expects:
 *      width + lsb > 64, value can be represented in width bits
 * 
 ************************/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
        uint64_t value) 
{       
        if (width + lsb > MAX_INT_LENGTH || !Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        } else if (width == MAX_INT_LENGTH) {
                RAISE(Bitpack_Shift64);
        }

        uint64_t mask = (((uint64_t)1 << width) - 1) << lsb;

        value = value << lsb;
        return (word & ~mask) | (value & mask);
}

/********** Bitpack_news ********
 *
 * Modifies bits in word between lsb to lsb + width into a signed value.
 * Returns modified word.
 * 
 *
 * Parameters:
 *      uint64_t word: unsigned word to be modified
 *      unsigned width: width of bits to be replaced
 *      unsigned lsb: least significant bit to start modification.
 *      int64_t value: signed value to replace bits in word with
 *                             
 * Return: unsigned word after modification
 *
 * Expects:
 *      width + lsb > 64, value can be represented in width bits
 * 
 ************************/
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
        int64_t value)
{
        if (width + lsb > MAX_INT_LENGTH || !Bitpack_fitss(value, width)) {
                RAISE(Bitpack_Overflow);
        } else if (width == MAX_INT_LENGTH) {
                RAISE(Bitpack_Shift64);
        }

        uint64_t highOrderBit = ((uint64_t)1 << (width + lsb - 1));
        
        /* Case where value is negative */
        if (value & highOrderBit) {
                uint64_t mask = (((uint64_t)1 << width) - 1);
                value = value << lsb;
                
                return ((word & ~(mask << lsb)) | 
                        (value & (mask << lsb))) | (~mask << lsb);

        }

        return Bitpack_newu(word, width, lsb, value);
        
}
