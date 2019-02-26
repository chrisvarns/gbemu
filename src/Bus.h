#pragma once
#include "types.h"

namespace Bus
{
	u8 LoadU8(u16 address);
	void StoreU8(u16 address, u8 val);
};
