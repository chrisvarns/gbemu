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
	u16 PC = 0x00;
};

extern Registers reg;

class CPU
{
public:
	static void step();
};