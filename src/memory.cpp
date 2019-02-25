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

s8 Memory::LoadS8(u16 address)
{
	return (s8)memory[address];
}

s16 Memory::LoadS16(u16 address)
{
	s16 ret = s16(memory[address]);
	ret |= (s16(memory[address + 1]) << 8);
	return ret;
}

void Memory::StoreU8(u16 address, u8 val)
{
	memory[address] = val;
}

void Memory::LoadBootRom()
{
	std::ifstream file;
	file.open("assets\\DMG_ROM.bin", std::ifstream::binary);
	assert(file);
	file.read((char*)memory, 256);
	file.close();
}