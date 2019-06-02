#pragma once
#include "types.h"

#include <string>

namespace Cartridge
{
	u8 LoadU8(u16 address);
	void StoreU8(u16 address, u8 val);

	void LoadGameRom();
	extern std::string rom_path;
}