#pragma once
#include "types.h"

namespace Memory
{
	u8 LoadU8(u16 address);
	u16 LoadU16(u16 address);
	s8 LoadS8(u16 address);
	s16 LoadS16(u16 address);

	void StoreU8(u16 address, u8 val);

	void LoadBootRom();
};
