## CS40 Projects
A few of my projects from [CS 40 - Machine Structure and Assembly Language Programming](https://www.cs.tufts.edu/comp/40/) in Spring 2024 at Tufts University.
Each HW folder contains the official specifciation (spec.pdf), and a source folder with the source code, a Makefile, and a design doc when applicable (design.pdf).

### Summaries
#### HW2 Interfaces, Implementations and Images
This project contained 4 sub-projects:
1. An abstraction for a 2-dimensional, polymorphic unboxed array (UArray2), utilizing a 1D array implementation.
2. An abstraction for a 2D unboxed array of bits (Bit2)
3. A tool to find solved sudoku puzzles in a pgm image, utilizing our UArray2 abstraction
4. A tool to remove black edges from pbm images, utilizing our Bit2 abstraction

#### HW4 Machine Arithmetic
Performs lossy compression and decompression of ppm images using quantization of chroma.

#### HW6 Turing-complete Universal Machine
A turing complete virtual machine implementation in C with:
  * 8 32-bit general purpose registers
  * A segmented, word-oriented memory space
  * An I/O device to display ASCII characters and perform input and output of unsigned 8-bit characters
  * A 32-bit program counter
  * 14 unique instructions

_"tests" contains UM binaries, which can be executed using the um compiled by the Makefile in the source_

#### HW8 Assembly-Language Programming
A stack-based reverse polish notation calculator, implemented in Universal Machine assembly language (so executable by the project in HW6!)
