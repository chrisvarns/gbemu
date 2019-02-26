#pragma once
#include "types.h"

struct Registers
{
	union
	{
		struct
		{
			u8 F; // Flag register
			u8 A;
		};
		u16 AF;
	};

	union
	{
		struct
		{
			u8 C;
			u8 B;
		};
		u16 BC;
	};

	union
	{
		struct
		{
			u8 E;
			u8 D;
		};
		u16 DE;
	};

	union
	{
		struct
		{
			u8 L;
			u8 H;
		};
		u16 HL;
	};

	union
	{
		struct
		{
			u8 SP_P;
			u8 SP_S;
		};
		u16 SP;
	};

	union
	{
		struct
		{
			u8 PC_C;
			u8 PC_P;
		};
		u16 PC = 0x00;
	};
};

extern Registers reg;

class CPU
{
public:
	static void step();
};