#include "memory.h"

u8 memory[0x10000];

u8 Memory::LoadU8(u16 address)
{
	return memory[address];
}
