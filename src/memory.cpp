#include "memory.h"

#include <fstream>
#include <assert.h>

u8 memory[0x10000];

u8 Memory::LoadU8(u16 address)
{
	return memory[address];
}

u16 Memory::LoadU16(u16 address)
{
	u16 ret = memory[address];
	ret |= (memory[address + 1] << 8);
	return ret;
}

void Memory::LoadBootRom()
{
	std::ifstream file;
	file.open("assets\\DMG_ROM.bin");
	assert(file);
	file.read((char*)&memory[0], 256);
	file.close();
}