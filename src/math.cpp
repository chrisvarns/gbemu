#include "math.h"
#include "cpu.h"

namespace Math
{

	u8 Math::Add(u8 a, u8 b)
	{
		u8 result = a + b;

		bool z = result == 0;
		bool n = false;
		bool h = ((a & 0x0F) + (b & 0x0F)) > 0x0F;
		bool c = ((u16)a + (u16)b) > 0xFF;

		reg.F =
			z ? (u8)Flags::Z : 0 |
			n ? (u8)Flags::N : 0 |
			h ? (u8)Flags::H : 0 |
			c ? (u8)Flags::C : 0;

		return result;
	}

	u8 Math::AddWithCarry(u8 a, u8 b)
	{
		u8 carry = reg.F & (u8)Flags::C;
		a = reg.F & (u8)Flags::C ? a + 1 : a;

		u8 result = a + b;

		bool z = result == 0;
		bool n = false;
		bool h = ((a & 0x0F) + (b & 0x0F)) > 0x0F;
		bool c = ((u16)a + (u16)b) > 0xFF;

		reg.F =
			z ? (u8)Flags::Z : 0 |
			n ? (u8)Flags::N : 0 |
			h ? (u8)Flags::H : 0 |
			c ? (u8)Flags::C : 0;

		return result;
	}

	u8 Sub(u8 val)
	{
		u8 result = reg.A - val;

		bool z = result == 0;
		bool n = true;
		bool h = ((s16)(reg.A & 0x0F)) < ((s16)(val & 0x0F));
		bool c = ((s16)reg.A - (s16)val) < 0x00;

		reg.F =
			z ? (u8)Flags::Z : 0 |
			n ? (u8)Flags::N : 0 |
			h ? (u8)Flags::H : 0 |
			c ? (u8)Flags::C : 0;

		return result;
	}

	u16 Add(u16 a, u16 b)
	{
		u16 result = a + b;

		bool z = reg.F & (u8)Flags::Z;
		bool n = false;
		bool h = ((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF;
		bool c = ((u32)a + (u32)b) > 0xFFFF;

		reg.F =
			z ? (u8)Flags::Z : 0 |
			n ? (u8)Flags::N : 0 |
			h ? (u8)Flags::H : 0 |
			c ? (u8)Flags::C : 0;

		return result;
	}

	u8 Inc(u8 a)
	{
		u8 result = a + 1;

		bool z = result == 0;
		bool n = false;
		bool h = (a & (u8)0x0F) == (u8)0x0F;
		bool c = reg.F & (u8)Flags::C;

		reg.F =
			z ? (u8)Flags::Z : 0 |
			n ? (u8)Flags::N : 0 |
			h ? (u8)Flags::H : 0 |
			c ? (u8)Flags::C : 0;

		return result;
	}

	u8 Dec(u8 a)
	{
		u8 result = a - 1;

		bool z = result == 0;
		bool n = true;
		bool h = a & (u8)0x0F;
		bool c = reg.F & (u8)Flags::C;

		reg.F =
			z ? (u8)Flags::Z : 0 |
			n ? (u8)Flags::N : 0 |
			h ? (u8)Flags::H : 0 |
			c ? (u8)Flags::C : 0;

		return result;
	}

	u16 Inc(u16 a)
	{
		u16 result = a;
		++result;

		return result;
	}

	u16 Dec(u16 a)
	{
		u16 result = a;
		--result;

		return result;
	}

	u8 And(u8 a, u8 b)
	{
		u8 result = a & b;

		bool z = result == 0;
		bool n = false;
		bool h = true;
		bool c = false;

		reg.F =
			z ? (u8)Flags::Z : 0 |
			n ? (u8)Flags::N : 0 |
			h ? (u8)Flags::H : 0 |
			c ? (u8)Flags::C : 0;

		return result;
	}

	u8 Or(u8 a, u8 b)
	{
		u8 result = a | b;

		bool z = result == 0;
		bool n = false;
		bool h = false;
		bool c = false;

		reg.F =
			z ? (u8)Flags::Z : 0 |
			n ? (u8)Flags::N : 0 |
			h ? (u8)Flags::H : 0 |
			c ? (u8)Flags::C : 0;

		return result;
	}

	u8 Xor(u8 val)
	{
		u8 result = reg.A ^ val;

		bool z = result == 0;
		bool n = false;
		bool h = false;
		bool c = false;

		reg.F =
			z ? (u8)Flags::Z : 0 |
			n ? (u8)Flags::N : 0 |
			h ? (u8)Flags::H : 0 |
			c ? (u8)Flags::C : 0;

		return result;
	}

	void Compare(u8 val)
	{
		u8 result = reg.A - val;

		bool z = result == 0;
		bool n = true;
		bool h = ((s16)(reg.A & 0x0F)) < ((s16)(val & 0x0F));
		bool c = ((s16)reg.A - (s16)val) < 0x00;

		reg.F =
			z ? (u8)Flags::Z : 0 |
			n ? (u8)Flags::N : 0 |
			h ? (u8)Flags::H : 0 |
			c ? (u8)Flags::C : 0;
	}

	void Example()
	{
		// ADD A,d8
		// reg.A = Add(reg.A, u8(0x3F), reg.F);
	}

}