#include "memory.h"

#include <fstream>
#include <assert.h>

u8 memory[0x10000];

u8 Memory::LoadU8(u16 address)
{
	return memory[address];
}

void Memory::LoadBootRom()
{
	std::ifstream file;
	file.open("assets\\DMG_ROM.bin");
	assert(file);
	file.read((char*)&memory[0], 256);
	file.close();
}