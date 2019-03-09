#pragma once
#include "types.h"

namespace BootRom
{
	u8 LoadU8(u16 address);

	void LoadFromDisk();

	extern u8 bootrom[0x0100];
}