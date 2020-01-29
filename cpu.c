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
	unsigned char instruction = read(PC++);
	unsigned char M;

	switch (instruction) {

		case 0x00: // BRK impl
			set_flag(4, 1);
			push((unsigned char) PC >> 8);
			push((unsigned char) PC & 0xff);
			push(P); 
			unsigned char lo = read((unsigned int) 0xfffe);
			unsigned char hi = read((unsigned int) 0xffff);
			PC = (unsigned int) hi << 8 + (unsigned int) lo;
			cycles = 6;
			break;

		case 0x01: // ORA X, ind
			unsigned int hi = (unsigned int) read(PC++) << 8;
			M = read(hi + (unsigned int) X);
			A = A | M;
			set_flag(7, A >> 7);
			set_flag(1, A == 0);
			cycles = 5;
			break;

		case 0x02: // undefined
		case 0x03:
		case 0x04:
			break;

		case 0x05: // ORA zpg
			M = read(0x0000 + (unsigned int) read(PC++));
			A = A | M;
			set_flag(7, A >> 7);
			set_flag(1, A == 0);
			cycles = 2;
			break;

		case 0x06: // ASL zpg
			unsigned int address = 0x0000 + (unsigned int) read(PC++);
			M = read(address);
			unsigned char data = M << 1;
			write(address, data);
			set_flag(7, data >> 7);
			set_flag(1, M == 0);
			set_flag(0, M >> 7);
			cycles = 4;
			break;

		case 0x07: // undefined
			break;

		case 0x08: // PHP impl
			push(P);
			cycles = 2;
			break;

		case 0x09: // ORA #
			M = read(PC++);
			A |= M;
			set_flag(7, A >> 7);
			set_flag(1, A == 0);
			cycles = 1;
			break;

		case 0x0A: // ASL A
			set_flag(0, A >> 7);
			A = A << 1;
			set_flag(7, A >> 7);
			set_flag(1, A == 0);
			cycles = 1;
			break;

		case 0x0D: // ORA abs
			//TODO
			break;
		case 0x0E: // ASL abs
			//TODO
			break;
		default:
			break;
	}

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

