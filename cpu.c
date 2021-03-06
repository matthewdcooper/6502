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

			address = read(read(PC++));
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

		case 0x16: // ASL zpg, X
			lo = read(PC++);
			lo += X;
			M = read( (unsigned int) lo );
			set_flag('C', M >> 7);
			M <<= 1;
			set_flag('N', M >> 7);
			set_flag('Z', M == 0);
			write( (unsigned int) lo, M);
			CYCLES = 5;
			break;

		case 0x17: // undefined
			break;

		case 0x18: // CLC impl
			set_flag('C', 0);
			CYCLES = 1;
			break;

		case 0x19: // ORA abs, Y

			lo = read(PC++);
			hi = read(PC++);
			address = hi << 8 + lo + Y;
			M = read(address);
			A |= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 3; // +1 if page boundary crossed
			if (address >> 8 != hi) CYCLES += 1;
			break;

		case 0x1A: // undefined
		case 0x1B:
		case 0x1C:
			break;

		case 0x1D: // ORA abs, X
			lo = read(PC++);
			hi = read(PC++);
			address = hi << 8 + lo + X;
			M = read(address);
			A |= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 3; // +1 if page boundary crossed
			if (address >> 8 != hi) CYCLES += 1;
			break;

		case 0x1E: // ASL abs, X
			lo = read(PC++);
			hi = read(PC++);
			address = hi << 8 + lo + X;
			M = read(address);
			set_flag('C', M >> 1);
			M <<= 1;
			set_flag('N', M >> 7);
			set_flag('Z', M == 0);
			write(address, M);
			CYCLES = 6;
			break;

		case 0x1F: // undefined
			break;

		case 0x20: // JSR abs
			lo = read(PC++);
			hi = read(PC++);
			push(PC);
			PC = hi << 8 + lo;
			CYCLES = 5;
			break;

		case 0x21: // AND X, ind
			lo = read(PC++);
			address = lo + X;
			lo = read(address);
			hi = read(address+1);
			M = read(hi << 8 + lo);
			A &= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 5;
			break;

		case 0x22: // undefined
		case 0x23: 
			break;

		case 0x24: // BIT zpg
			lo = read(PC++);
			M = read( (unsigned int) lo );
			set_flag('Z', A & M == 0);
			set_flag('N', M >> 7);
			set_flag('V', (M >> 6) & 1);
			CYCLES = 2;
			break;

		case 0x25: // AND zpg
			lo = read(PC++);
			M = read( (unsigned int) lo );
			A &= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 2;
			break;

		case 0x26: // ROL zpg
			lo = read(PC++);
			M = read( (unsigned int) lo );
			set_flag('C', M >> 7);
			M <<= 1;
			M += get_flag('C');
			set_flag('N', M >> 7);
			set_flag('Z', M == 0);
			CYCLES = 4;
			break;

		case 0x27: // undefined
			break;

		case 0x28: // SEC impl
			set_flag('C', 1);
			CYCLES = 1;
			break;

		case 0x29: // AND #
			M = read(PC++);
			A &= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 1;
			break;

		case 0x2A: // ROL A
			set_flag('C', A >> 7);
			A <<= 1;
			A += get_flag('C');
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 1;
			break;

		case 0x2B: // undefined
			break;

		case 0x2C: // BIT abs
			lo = read(PC++);
			hi = read(PC++);
			address = hi << 8 + lo;
			M = read(address);
			set_flag('N', M >> 7);
			set_flag('V', (M >> 6) & 1);
			set_flag('Z', A & M == 0);
			CYCLES = 3;
			break;

		case 0x2D: // AND abs
			lo = read(PC++);
			hi = read(PC++);
			address = hi << 8 + lo;
			M = read(address);
			A &= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 3;
			break;

		case 0x2E: // ROL abs
			lo = read(PC++);
			hi = read(PC++);
			address = hi << 8 + lo;
			M = read(address);
			set_flag('C', M >> 7);
			M <<= 1;
			M += get_flag('C');
			set_flag('N', M >> 7);
			set_flag('Z', M == 0);
			CYCLES = 5;
			break;

		case 0x2F: // undefined
			break;

		case 0x30: // BMI rel
			data = read(PC++);
			if (get_flag('N') == 1) {
					hi = PC >> 8;
					PC += data;
					CYCLES = 2; // +1 if page boundary crossed
					if (PC >> 8 != hi) CYCLES += 1;
			} else {
				CYCLES = 1; 
			}
			break;

		case 0x31: // AND ind, Y
			address = read(read(PC++));
			lo = read(address);
			hi = read(address+1);
			address = hi << 8 + lo + Y;
			M = read(address);
			A &= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 4; // : +1 if page boundary crossed
			if (address >> 8 != hi) CYCLES += 1;
			break;

		case 0x32: // undefined
		case 0x33:
		case 0x34:
			break;

		case 0x35: // AND zpg, X
			lo = read(PC++);
			lo += X;
			M = read( (unsigned int) lo );
			A &= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 3;
			break;

		case 0x36: // ROL zpg, X
			lo = read(PC++);
			lo += X;
			M = read( (unsigned int) lo );
			set_flag('C', M >> 7);
			M <<= 1;
			write( (unsigned int) lo, M);
			set_flag('N', M >> 7);
			set_flag('Z', M == 0);
			CYCLES = 5;
			break;

		case 0x37: // undefined
			break;

		case 0x38: // SEC impl
			set_flag('C', 1);
			CYCLES = 1;
			break;

		case 0x39: // AND abs, Y
			lo = read(PC++);
			hi = read(PC++);
			address = hi << 8 + lo + Y;
			M = read(address);
			A &= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 3; // +1 if page boundary crossed
			if (address >> 8 != hi) CYCLES += 1;
			break;

		case 0x3A: // undefined
		case 0x3B:
		case 0x3C:
			break;

		case 0x3D: // AND abs, X
			lo = read(PC++);
			hi = read(PC++);
			address = hi << 8 + lo + X;
			M = read(address);
			A &= M;
			set_flag('N', A >> 7);
			set_flag('Z', A == 0);
			CYCLES = 3; // +1 if page boundary crossed
			if (address >> 8 != hi) CYCLES += 1;
			break;

		case 0x3E: // ROL abs, X
			lo = read(PC++);
			hi = read(PC++);
			address = hi << 8 + lo + X;
			M = read(address);
			set_flag('C', M >> 7);
			M <<= 1;
			write(address, M);
			set_flag('N', M >> 7);
			set_flag('Z', M == 0);
			CYCLES = 6;
			break;

		case 0x3F: // undefined
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

