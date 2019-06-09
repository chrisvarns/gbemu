#pragma once
#include "types.h"

enum class Flags : u8
{
	Z = 0x80,
	N = 0x40,
	H = 0x20,
	C = 0x10
};

namespace Math
{
	u8 Add(u8 a, u8 b);
	u8 AddWithCarry(u8 a, u8 b);
	u16 Add(u16 a, u16 b);

	void SubFromA(u8 val);

	void Inc(u8& a);
	void Dec(u8& a);
	void Inc(u16& a);
	void Dec(u16& a);

	u8 And(u8 a, u8 b);
	u8 Or(u8 a, u8 b);
	void Xor(u8 val);

	void Compare(u8 val);

	void RotateLeft(u8& val);
	void RotateRight(u8& val);
	void RotateLeftThroughCarry(u8& val);
	void RotateRightThroughCarry(u8& val);

	// CB Prefix
	void Bit(u8 reg, u8 bit);
	void RotateLeftThroughCarry_CB(u8& val);
	void RotateRightThroughCarry_CB(u8& val);
}

