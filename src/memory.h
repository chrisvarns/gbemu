#pragma once
#include "types.h"

class Memory
{
public:
	static u8 LoadU8(u16 address);
	static void LoadBootRom();
};
