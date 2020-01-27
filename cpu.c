#include <stdbool.h>
#include <stdio.h>
#include "ram.h"

/* REGISTERS */

// The 6502 has five 8-bit registers (chars) and one 16-bit register (int).

unsigned char A = 0; // accumulator
unsigned char X = 0; // index X
unsigned char Y = 0; // index Y
unsigned char S = 0xFF; // stack pointer
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

unsigned int cycles = 0;


/* STACK HELPERS */
void push(unsigned char data) {
	unsigned int address = 0x0100 + S;
	write(address, data);
	S -= 1;
}


unsigned char pull() {
	unsigned int address = 0x0100 + S;
	unsigned char data = read(address);
	S += 1;
	return data;
}


/* FLAG HELPERS */
void set_flag(unsigned int bit_n, bool value)
{
	if (value == 1) {
		P |= 1 << bit_n;
	} else {
		P &= ~(1 << bit_n);
	}
}


bool get_flag(unsigned int bit_n) 
{
	return (P >> bit_n) & 1;
}


int tick()
// a single clock cycle
{
	if (cycles > 0) {
			cycles -= 1;
			return -1; // instruction in progress
	}
	unsigned char instruction = read(PC);
	unsigned char M;

	switch (instruction) {
		case 0x0000: // BRK impl
			//TODO
			break;
		case 0x0001: // ORA X, ind
			//TODO
			break;
		case 0x0005: // ORA zpg
			//TODO
			break;
		case 0x0006: // ASL zpg
			//TODO
			break;
		case 0x0008: // PHP impl
			//TODO
			break;

		case 0x0009: // ORA #
			increment_PC();
			M = read(PC);
			A |= M;
			set_flag(7, A >> 7);
			set_flag(1, A == 0);
			cycles = 1;
			break;

		case 0x000A: // ASL A
			//TODO
			break;
		case 0x000D: // ORA abs
			//TODO
			break;
		case 0x000E: // ASL abs
			//TODO
			break;
		default:
			break;
	}

	increment_PC();
	return 0; // instruction complete
}


int main() {
	bool b;
	initialize_memory();
	/* Test setting and getting flags */
	b = get_flag(0); // 0
	printf("%d", b);

	b = get_flag(5); // 1
	printf("%d", b);

	set_flag(7, 1);
	b = get_flag(7); // 1
	printf("%d", b);

	set_flag(7, 0);
	b = get_flag(7); // 0
	printf("%d", b);

	// Expected output: 0110


	printf("\n");
	return 0;
}

