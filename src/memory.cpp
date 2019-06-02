#include "memory.h"

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