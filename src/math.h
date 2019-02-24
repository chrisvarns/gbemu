#pragma once
#include "types.h"

namespace Math
{
	enum class Flags : u8
	{
		Z = 0x80,
		N = 0x40,
		H = 0x20,
		C = 0x10
	};
	u8 Add(u8 a, u8 b);
	u8 AddWithCarry(u8 a, u8 b);
	u8 Sub(u8 val);
	u16 Add(u16 a, u16 b);

	u8 Inc(u8 a);
	u8 Dec(u8 a);
	u16 Inc(u16 a);
	u16 Dec(u16 a);

	u8 And(u8 a, u8 b);
	u8 Or(u8 a, u8 b);
	u8 Xor(u8 val);

	void Compare(u8 val);
}

