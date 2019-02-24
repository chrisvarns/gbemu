#pragma once
#include "types.h"

struct Registers
{
	union
	{
		struct
		{
			u8 A;
			u8 F; // Flag register
		};
		u16 AF;
	};

	union
	{
		struct
		{
			u8 B;
			u8 C;
		};
		u16 BC;
	};

	union
	{
		struct
		{
			u8 D;
			u8 E;
		};
		u16 DE;
	};

	union
	{
		struct
		{
			u8 H;
			u8 L;
		};
		u16 HL;
	};

	u16 SP;
	u16 PC;
};

extern Registers reg;

namespace Math
{
	enum class Flags : u8
	{
		Z = 0x80,
		N = 0x40,
		H = 0x20,
		C = 0x10
	};


	u8 Add(u8 a, u8 b, u8& flags);
	u8 AddWithCarry(u8 a, u8 b, u8& flags);
	u8 Sub(u8 a, u8 b, u8& flags);
	u16 Add(u16 a, u16 b, u8& flags);

	u8 Inc(u8 a, u8& flags);
	u8 Dec(u8 a, u8& flags);
	u16 Inc(u16 a);
	u16 Dec(u16 a);

	u8 And(u8 a, u8 b, u8& flags);
	u8 Or(u8 a, u8 b, u8& flags);
	u8 Xor(u8 a, u8 b, u8& flags);


	void Example();
}

