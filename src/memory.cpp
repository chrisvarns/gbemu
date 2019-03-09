#include "memory.h"

#include <fstream>
#include <assert.h>

u8 memory[0x10000];

u8 Memory::LoadU8(u16 address)
{
	return memory[address];
}

//u16 Memory::LoadU16(u16 address)
//{
//	u16 ret = memory[address];
//	ret |= (memory[address + 1] << 8);
//	return ret;
//}

//s8 Memory::LoadS8(u16 address)
//{
//	return reinterpret_cast<s8>(memory[address]);
//	//return (s8)memory[address];
//}

//s16 Memory::LoadS16(u16 address)
//{
//	s16 ret = s16(memory[address]);
//	ret |= (s16(memory[address + 1]) << 8);
//	return ret;
//}

void Memory::StoreU8(u16 address, u8 val)
{
	memory[address] = val;
}

//void Memory::StoreU16(u16 address, u16 val)
//{
//	u8 lsb = u8(val & 0xFF);
//	u8 msb = u8(val >> 8);
//	memory[address] = lsb;
//	memory[address + 1] = msb;
//}

