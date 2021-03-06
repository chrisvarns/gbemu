#include <assert.h>

#include "math.h"
#include "cpu.h"

#define SET_FLAG_REG(z, n, h, c)	\
	reg.F =							\
		(z ? (u8)Flags::Z : 0) |	\
		(n ? (u8)Flags::N : 0) |	\
		(h ? (u8)Flags::H : 0) |	\
		(c ? (u8)Flags::C : 0)

namespace Math
{

	u8 Add(u8 a, u8 b)
	{
		u8 result = a + b;

		bool z = result == 0;
		bool n = false;
		bool h = ((a & 0x0F) + (b & 0x0F)) > 0x0F;
		bool c = ((u16)a + (u16)b) > 0xFF;

		SET_FLAG_REG(z, n, h, c);

		return result;
	}

	u8 AddWithCarry(u8 a, u8 b)
	{
		u8 carry = reg.F & (u8)Flags::C;
		a = reg.F & (u8)Flags::C ? a + 1 : a;

		u8 result = a + b;

		bool z = result == 0;
		bool n = false;
		bool h = ((a & 0x0F) + (b & 0x0F)) > 0x0F;
		bool c = ((u16)a + (u16)b) > 0xFF;

		SET_FLAG_REG(z, n, h, c);

		return result;
	}

	void SubFromA(u8 val)
	{
		u8 result = reg.A - val;

		bool z = result == 0;
		bool n = true;
		bool h = ((s16)(reg.A & 0x0F)) >= ((s16)(val & 0x0F)); // Set if no borrow from bit 4
		bool c = ((s16)reg.A - (s16)val) < 0x00;

		SET_FLAG_REG(z, n, h, c);

		reg.A = result;
	}

	void SubWithCarryFromA(u8 val)
	{
		u8 carry = (reg.F & (u8)Flags::C) ? (u8)1U : (u8)0U;
		u8 result = reg.A - (val + carry);

		bool z = result == 0;
		bool n = true;
		bool h = ((s16)(reg.A & 0x0F)) >= ((s16)(val & 0x0F)); // Set if no borrow from bit 4
		bool c = ((s16)reg.A - (s16)val) < 0x00;

		SET_FLAG_REG(z, n, h, c);

		reg.A = result;
	}

	u16 Add(u16 a, u16 b)
	{
		u16 result = a + b;

		bool z = reg.F & (u8)Flags::Z;
		bool n = false;
		bool h = ((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF;
		bool c = ((u32)a + (u32)b) > 0xFFFF;

		SET_FLAG_REG(z, n, h, c);

		return result;
	}

	void Inc(u8& a)
	{
		u8 result = a + 1;

		bool z = result == 0;
		bool n = false;
		bool h = (a & 0x0F) == 0x0F;
		bool c = reg.F & (u8)Flags::C;

		SET_FLAG_REG(z, n, h, c);

		a = result;
	}

	void Dec(u8& a)
	{
		u8 result = a - 1;

		bool z = result == 0;
		bool n = true;
		bool h = (a & 0x1F) == 0x10;
		bool c = reg.F & (u8)Flags::C;

		SET_FLAG_REG(z, n, h, c);

		a = result;
	}

	void Inc(u16& a)
	{
		a++;
	}

	void Dec(u16& a)
	{
		a--;
	}

	u8 And(u8 a, u8 b)
	{
		u8 result = a & b;

		bool z = result == 0;
		bool n = false;
		bool h = true;
		bool c = false;

		SET_FLAG_REG(z, n, h, c);

		return result;
	}

	u8 Or(u8 a, u8 b)
	{
		u8 result = a | b;

		bool z = result == 0;
		bool n = false;
		bool h = false;
		bool c = false;

		SET_FLAG_REG(z, n, h, c);

		return result;
	}

	void Xor(u8 val)
	{
		reg.A ^= val;

		bool z = reg.A == 0;
		bool n = false;
		bool h = false;
		bool c = false;

		SET_FLAG_REG(z, n, h, c);
	}

	void Compare(u8 val)
	{
		u8 result = reg.A - val;

		bool z = result == 0;
		bool n = true;
		bool h = ((s16)(reg.A & 0x0F)) < ((s16)(val & 0x0F));
		bool c = ((s16)reg.A - (s16)val) < 0x00;

		SET_FLAG_REG(z, n, h, c);
	}

	void Bit(u8 val, u8 bit)
	{
		assert(bit >= 0 && bit <= 7);

		u8 bitmask = 1 << bit;

		bool z = !(val & bitmask);
		bool n = false;
		bool h = true;
		bool c = reg.F & (u8)Flags::C;

		SET_FLAG_REG(z, n, h, c);
	}

	void RotateLeft(u8& val)
	{
		bool carry = val & 0x80;
		u8 result = val << 1;
		result |= carry ? 0x1 : 0x0;

		bool z = false;
		bool n = false;
		bool h = false;
		bool c = carry;

		SET_FLAG_REG(z, n, h, c);

		val = result;
	}

	void RotateRight(u8& val)
	{
		bool carry = val & 0x01;
		u8 result = val >> 1;
		result |= carry ? 0x80 : 0x0;

		bool z = false;
		bool n = false;
		bool h = false;
		bool c = carry;

		SET_FLAG_REG(z, n, h, c);

		val = result;
	}

	inline void RotateLeftThroughCarry_Internal(u8& val, bool set_z)
	{
		u8 result = val << 1;
		result |= (reg.F & (u8)Flags::C) ? 0x1 : 0x0;

		bool z = set_z && !result;
		bool n = false;
		bool h = false;
		bool c = val & 0x80;

		SET_FLAG_REG(z, n, h, c);

		val = result;
	}

	inline void RotateRightThroughCarry_Internal(u8& val, bool set_z)
	{
		u8 result = val >> 1;
		result |= (reg.F & (u8)Flags::C) ? 0x80 : 0x0;

		bool z = set_z && !result;
		bool n = false;
		bool h = false;
		bool c = val & 0x01;

		SET_FLAG_REG(z, n, h, c);

		val = result;
	}

	void RotateLeftThroughCarry(u8& val)
	{
		RotateLeftThroughCarry_Internal(val, false);
	}

	void RotateLeftThroughCarry_CB(u8& val)
	{
		RotateLeftThroughCarry_Internal(val, true);
	}

	void RotateRightThroughCarry(u8& val)
	{
		RotateRightThroughCarry_Internal(val, false);
	}

	void RotateRightThroughCarry_CB(u8& val)
	{
		RotateRightThroughCarry_Internal(val, true);
	}
		
}