#include <stdio.h>
#include "ram.h"

/* REGISTERS */

// The 6502 has five 8-bit registers (chars) and one 16-bit register (int).

unsigned char A = 0; // accumulator
unsigned char X = 0; // index X
unsigned char Y = 0; // index Y
unsigned char S = 0; // stack pointer
unsigned char P = 32; // status (uses only 7 bits)
unsigned int PC = 0; // program counter

/* The status register begins at 32 because bit 5 is always 1.
 * The other 7 bits carry specific meanings about the status of the cpu.
 *
 * See: https://www.atarimagazines.com/compute/issue53/047_1_All_About_The_Status_Register.php
 *
 * bits:    76543210
 * meaning: NV_BDIZC
 * (Negative, oVerflow, unused, Break, Decimal, Interrupt, Zero, Carry)
 */


int tick()
{

	return 0;
}


int main() {
	initialize_memory();
	printf("hello\n");
	return 0;
}

