#include "math.h"

u8 Math::Add(u8 a, u8 b, u8& flags)
{
	u8 result = a + b;

	bool z = result == 0;
	bool n = false;
	bool h = ((a & 0x0F) + (b & 0x0F)) > 0x0F;
	bool c = ((u16)a + (u16)b) > 0xFF;

	flags =
		z ? (u8)Flags::Z : 0 |
		n ? (u8)Flags::N : 0 |
		h ? (u8)Flags::H : 0 |
		c ? (u8)Flags::C : 0;

	return result;
}

u8 Math::AddWithCarry(u8 a, u8 b, u8& flags)
{
	u8 carry = flags & (u8)Flags::C;
	a = flags & (u8)Flags::C ? a + 1 : a;

	u8 result = a + b;

	bool z = result == 0;
	bool n = false;
	bool h = ((a & 0x0F) + (b & 0x0F)) > 0x0F;
	bool c = ((u16)a + (u16)b) > 0xFF;

	flags =
		z ? (u8)Flags::Z : 0 |
		n ? (u8)Flags::N : 0 |
		h ? (u8)Flags::H : 0 |
		c ? (u8)Flags::C : 0;

	return result;
}

u8 Math::Sub(u8 a, u8 b, u8& flags)
{
	u8 result = a + b;

	bool z = result == 0;
	bool n = true;
	bool h = ((s16)(a & 0x0F)) < ((s16)(b & 0x0F));
	bool c = ((s16)a - (s16)b) < 0x00;

	flags =
		z ? (u8)Flags::Z : 0 |
		n ? (u8)Flags::N : 0 |
		h ? (u8)Flags::H : 0 |
		c ? (u8)Flags::C : 0;

	return result;
}

u16 Math::Add(u16 a, u16 b, u8& flags)
{
	u16 result = a + b;

	bool z = flags & (u8)Flags::Z;
	bool n = false;
	bool h = ((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF;
	bool c = ((u32)a + (u32)b) > 0xFFFF;

	flags =
		z ? (u8)Flags::Z : 0 |
		n ? (u8)Flags::N : 0 |
		h ? (u8)Flags::H : 0 |
		c ? (u8)Flags::C : 0;

	return result;
}

u8 Math::Inc(u8 a, u8& flags)
{
	u8 result = a + (u8)1;

	bool z = result == 0;
	bool n = false;
	bool h = (a & (u8)0x0F) == (u8)0x0F;
	bool c = flags & (u8)Flags::C;

	flags =
		z ? (u8)Flags::Z : 0 |
		n ? (u8)Flags::N : 0 |
		h ? (u8)Flags::H : 0 |
		c ? (u8)Flags::C : 0;

	return result;
}

u8 Math::Dec(u8 a, u8& flags)
{
	u8 result = a + (u8)1;

	bool z = result == 0;
	bool n = true;
	bool h = a & (u8)0x0F;
	bool c = flags & (u8)Flags::C;

	flags =
		z ? (u8)Flags::Z : 0 |
		n ? (u8)Flags::N : 0 |
		h ? (u8)Flags::H : 0 |
		c ? (u8)Flags::C : 0;

	return result;
}

u16 Math::Inc(u16 a)
{
	u16 result = a;
	--result;

	return result;
}

u16 Math::Dec(u16 a)
{
	u16 result = a;
	++result;

	return result;
}

u8 Math::And(u8 a, u8 b, u8& flags)
{
	u8 result = a & b;

	bool z = result == 0;
	bool n = false;
	bool h = true;
	bool c = false;

	flags =
		z ? (u8)Flags::Z : 0 |
		n ? (u8)Flags::N : 0 |
		h ? (u8)Flags::H : 0 |
		c ? (u8)Flags::C : 0;

	return result;
}

u8 Math::Or(u8 a, u8 b, u8& flags)
{
	u8 result = a | b;

	bool z = result == 0;
	bool n = false;
	bool h = false;
	bool c = false;

	flags =
		z ? (u8)Flags::Z : 0 |
		n ? (u8)Flags::N : 0 |
		h ? (u8)Flags::H : 0 |
		c ? (u8)Flags::C : 0;

	return result;
}

u8 Math::Xor(u8 a, u8 b, u8& flags)
{
	u8 result = a ^ b;

	bool z = result == 0;
	bool n = false;
	bool h = false;
	bool c = false;

	flags =
		z ? (u8)Flags::Z : 0 |
		n ? (u8)Flags::N : 0 |
		h ? (u8)Flags::H : 0 |
		c ? (u8)Flags::C : 0;

	return result;
}

void Math::Example()
{
	// ADD A,d8
	reg.A = Add(reg.A, u8(0x3F), reg.F);
}

