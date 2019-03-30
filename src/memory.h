#pragma once
#include <string>
#include "types.h"

namespace Memory
{
	u8 LoadU8(u16 address);
	void StoreU8(u16 address, u8 val);

	void Init();
	void LoadGameRom();
	extern std::string gamerom_path;
};

