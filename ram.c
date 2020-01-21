
#define SIZE 2000 // How many bytes?
unsigned char memory[SIZE];

void initialize_memory()
{
	for (int i = 0; i < SIZE; i++) {
		memory[i] = 0;
	}
}


unsigned char read(unsigned int address)
{
	return memory[address];
}


void write(unsigned int address, unsigned char value)
{
	memory[address] = value;
}
