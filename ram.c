#include<stdio.h>

#define SIZE 2000 // How many bytes?
unsigned char memory[SIZE];

void initialize_memory()
{
	for (int i = 0; i < SIZE; i++) {
		memory[i] = 0;
	}
}

void load_program(char *name)
{
	FILE *fp;
	fp = fopen(name, "r");
	fread(memory, 1, SIZE, fp);
	fclose(fp);
}


unsigned char read(unsigned int address)
{
	return memory[address];
}


void write(unsigned int address, unsigned char value)
{
	memory[address] = value;
}
