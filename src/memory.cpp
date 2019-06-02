#include "memory.h"

u8 Memory::memory[0x10000];

void Memory::Init()
{
	memset(memory, 0, 0x10000);
}