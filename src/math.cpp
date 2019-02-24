#include "math.h"


std::uint8_t Math::Add(std::uint8_t a, std::uint8_t b, std::uint8_t& flags)
{
	std::uint8_t result = a + b;

	bool z = result == 0;
	bool n = false;
	bool h = ((a & 0x0F) + (b & 0x0F)) > 0x0F;
	bool c = ((std::uint_least16_t)a + (std::uint_least16_t)b) > 0xFF;

	flags =
		z ? (std::uint8_t)Flags::Z : 0 |
		n ? (std::uint8_t)Flags::N : 0 |
		h ? (std::uint8_t)Flags::H : 0 |
		c ? (std::uint8_t)Flags::C : 0;

	return result;
}

std::uint8_t Math::AddWithCarry(std::uint8_t a, std::uint8_t b, std::uint8_t& flags)
{
	std::uint8_t carry = flags & (std::uint8_t)Flags::C;
	a = flags & (std::uint8_t)Flags::C ? a + 1 : a;

	std::uint8_t result = a + b;

	bool z = result == 0;
	bool n = false;
	bool h = ((a & 0x0F) + (b & 0x0F)) > 0x0F;
	bool c = ((std::uint_least16_t)a + (std::uint_least16_t)b) > 0xFF;

	flags =
		z ? (std::uint8_t)Flags::Z : 0 |
		n ? (std::uint8_t)Flags::N : 0 |
		h ? (std::uint8_t)Flags::H : 0 |
		c ? (std::uint8_t)Flags::C : 0;

	return result;
}

std::uint8_t Math::Sub(std::uint8_t a, uint8_t b, std::uint8_t& flags)
{
	std::uint8_t result = a + b;

	bool z = result == 0;
	bool n = true;
	bool h = ((std::int_least16_t)(a & 0x0F)) < ((std::int_least16_t)(b & 0x0F));
	bool c = ((std::int_least16_t)a - (std::int_least16_t)b) < 0x00;

	flags =
		z ? (std::uint8_t)Flags::Z : 0 |
		n ? (std::uint8_t)Flags::N : 0 |
		h ? (std::uint8_t)Flags::H : 0 |
		c ? (std::uint8_t)Flags::C : 0;

	return result;
}

std::uint16_t Math::Add(std::uint16_t a, std::uint16_t b, std::uint8_t& flags)
{
	std::uint16_t result = a + b;

	bool z = flags & (std::uint8_t)Flags::Z;
	bool n = false;
	bool h = ((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF;
	bool c = ((std::uint_least32_t)a + (std::uint_least32_t)b) > 0xFFFF;

	flags =
		z ? (std::uint8_t)Flags::Z : 0 |
		n ? (std::uint8_t)Flags::N : 0 |
		h ? (std::uint8_t)Flags::H : 0 |
		c ? (std::uint8_t)Flags::C : 0;

	return result;
}

std::uint8_t Math::Inc(std::uint8_t a, std::uint8_t& flags)
{
	std::uint8_t result = a + (std::uint8_t)1;

	bool z = result == 0;
	bool n = false;
	bool h = (a & (std::uint8_t)0x0F) == (std::uint8_t)0x0F;
	bool c = flags & (std::uint8_t)Flags::C;

	flags =
		z ? (std::uint8_t)Flags::Z : 0 |
		n ? (std::uint8_t)Flags::N : 0 |
		h ? (std::uint8_t)Flags::H : 0 |
		c ? (std::uint8_t)Flags::C : 0;

	return result;
}

std::uint8_t Math::Dec(std::uint8_t a, std::uint8_t& flags)
{
	std::uint8_t result = a + (std::uint8_t)1;

	bool z = result == 0;
	bool n = true;
	bool h = a & (std::uint8_t)0x0F;
	bool c = flags & (std::uint8_t)Flags::C;

	flags =
		z ? (std::uint8_t)Flags::Z : 0 |
		n ? (std::uint8_t)Flags::N : 0 |
		h ? (std::uint8_t)Flags::H : 0 |
		c ? (std::uint8_t)Flags::C : 0;

	return result;
}

std::uint16_t Math::Inc(std::uint16_t a)
{
	std::uint16_t result = a;
	--result;

	return result;
}

std::uint16_t Math::Dec(std::uint16_t a)
{
	std::uint16_t result = a;
	++result;

	return result;
}

std::uint8_t Math::And(std::uint8_t a, std::uint8_t b, std::uint8_t& flags)
{
	std::uint8_t result = a & b;

	bool z = result == 0;
	bool n = false;
	bool h = true;
	bool c = false;

	flags =
		z ? (std::uint8_t)Flags::Z : 0 |
		n ? (std::uint8_t)Flags::N : 0 |
		h ? (std::uint8_t)Flags::H : 0 |
		c ? (std::uint8_t)Flags::C : 0;

	return result;
}

std::uint8_t Math::Or(std::uint8_t a, std::uint8_t b, std::uint8_t& flags)
{
	std::uint8_t result = a | b;

	bool z = result == 0;
	bool n = false;
	bool h = false;
	bool c = false;

	flags =
		z ? (std::uint8_t)Flags::Z : 0 |
		n ? (std::uint8_t)Flags::N : 0 |
		h ? (std::uint8_t)Flags::H : 0 |
		c ? (std::uint8_t)Flags::C : 0;

	return result;
}

std::uint8_t Math::Xor(std::uint8_t a, std::uint8_t b, std::uint8_t& flags)
{
	std::uint8_t result = a ^ b;

	bool z = result == 0;
	bool n = false;
	bool h = false;
	bool c = false;

	flags =
		z ? (std::uint8_t)Flags::Z : 0 |
		n ? (std::uint8_t)Flags::N : 0 |
		h ? (std::uint8_t)Flags::H : 0 |
		c ? (std::uint8_t)Flags::C : 0;

	return result;
}

