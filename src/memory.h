#pragma once
#include <string>
#include "types.h"

namespace Memory
{
	extern u8 memory[0x10000];

	inline u8 LoadU8(u16 address)
	{
		return memory[address];
	}

	inline void StoreU8(u16 address, u8 val)
	{
		memory[address] = val;
	}

	void Init();
};

