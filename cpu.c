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

unsigned int CYCLES = 0;


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

int flag_to_bit(unsigned char flag) {
	switch (flag) {
		case 'N': return 7;
		case 'V': return 6;
		case 'B': return 4;
		case 'D': return 3;
		case 'I': return 2;
		case 'Z': return 1;
		case 'C': return 0;

		default:
			  printf("Invalid flag: %c\n", flag);
			  return -1;
	}
}

void set_flag(unsigned char flag, bool value)
{
	int bit_n = flag_to_bit(flag);
	if (value == 1) {
		P |= 1 << bit_n;
	} else {
		P &= ~(1 << bit_n);
	}
}


bool get_flag(unsigned char flag) 
{
	int bit_n = flag_to_bit(flag);
	return (P >> bit_n) & 1;
}


/* EMULATOR */
int tick()
// a single clock cycle
{

	unsigned char instruction;
	unsigned char M;
	unsigned char lo;
	unsigned char hi;
	unsigned char data;
	unsigned int address;

	// If an instruction hasn't completed its CYCLES, then tick silently.
	if (CYCLES > 0) {
			CYCLES -= 1;
			return -1; // instruction in progress
	}


	// Fetch and Execute
	instruction = read(PC++);
	switch (instruction) {

		case 0x00: // BRK impl
			set_flag('B', 1);
			push((unsigned char) PC >> 8);
			push((unsigned char) PC & 0xff);
			push(P); 
			lo = read((unsigned int) 0xfffe);
			hi = read((unsigned int) 0xffff);
			PC = (unsigned int) hi << 8 + (unsigned int) lo;
			CYCLES = 6;
			break;

		case 0x01: // ORA X, ind
			hi = (unsigned int) read(PC++) << 8;
			M = read(hi + (unsigned int) X);
			A |= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 5;
			break;

		case 0x02: // undefined
		case 0x03:
		case 0x04:
			break;

		case 0x05: // ORA zpg
			M = read(0x0000 + (unsigned int) read(PC++));
			A |= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 2;
			break;

		case 0x06: // ASL zpg
			address = 0x0000 + (unsigned int) read(PC++);
			M = read(address);
			data = M << 1;
			write(address, data);
			set_flag('N', data >> 7);
			set_flag('Z', M == 0);
			set_flag('C', M >> 7);
			CYCLES = 4;
			break;

		case 0x07: // undefined
			break;

		case 0x08: // PHP impl
			push(P);
			CYCLES = 2;
			break;

		case 0x09: // ORA #
			M = read(PC++);
			A |= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 1;
			break;

		case 0x0A: // ASL A
			set_flag('C', A >> 7);
			A <<= 1;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 1;
			break;

		case 0x0B: // undefined
		case 0x0C:
			break;

		case 0x0D: // ORA abs
			lo = read(PC++);
			hi = read(PC++);
			address = ((unsigned int) hi << 8) + (unsigned int) lo;
			M = read(address);
			A |= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 3;
			break;

		case 0x0E: // ASL abs
			lo = read(PC++);
			hi = read(PC++);
			address = ((unsigned int) hi << 8) + (unsigned int) lo;
			M = read(address);
			data = M << 1;
			write(address, data);
			set_flag('N', data >> 7);
			set_flag('Z', M == 0);
			set_flag('C', M >> 7);
			CYCLES = 5;
			break;

		case 0x0F: // undefined
			break;

		case 0x10: // BPL rel
			data = read(PC++);
			if (get_flag('N') == 0) {
					hi = PC >> 8;
					PC += data;
					CYCLES = 2; // +1 if page boundary crossed
					if (PC >> 8 != hi) CYCLES += 1;
			} else {
				CYCLES = 1; 
			}
			break;

		case 0x11: // ORA ind, Y
			address = read(read(PC++))
			lo = read(address);
			hi = read(address+1);
			address = hi << 8 + lo + Y;
			M = read(address);
			A |= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 4; // : +1 if page boundary crossed
			if (address >> 8 != hi) CYCLES += 1;

		case 0x12: // undefined
		case 0x13:
		case 0x14:
			break;

		case 0x15: // ORA zpg, X
			lo = read(PC++);
			lo += X;
			M = read( (unsigned int) lo );
			A |= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 3;
			break;

		default:
			break;
	}

	return 0; // instruction complete
}


int main(int argc, char *argv[]) {
	initialize_memory();
	if (argc == 2) load_program(argv[1]);
	for (int i = 0; i < 8; i++) {
		printf("%d ", read(i));
	}
	printf("\n");
	return 0;
}

