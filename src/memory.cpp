#include "memory.h"

#include <fstream>
#include <assert.h>

std::string Memory::gameromPath = "";
u8 memory[0x10000];

u8 Memory::LoadU8(u16 address)
{
	return memory[address];
}

void Memory::StoreU8(u16 address, u8 val)
{
	memory[address] = val;
}

void Memory::Init()
{
	memset(memory, 0, 0x10000);
}

void Memory::LoadGameRom()
{
	assert(!gameromPath.empty() && "Specify \"-gamerom something\" on cmdline");
	std::ifstream file;
	file.open(gameromPath, std::ifstream::binary);
	assert(file && "Can't find gamerom");
	file.read((char*)memory, 0x8000);
	file.close();
}