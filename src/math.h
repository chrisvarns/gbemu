#pragma once
#include <cstdint>


struct Registers
{
	union
	{
		struct
		{
			std::uint8_t A;
			std::uint8_t F;
		};
		std::uint16_t AF;
	};

	union
	{
		struct
		{
			std::uint8_t B;
			std::uint8_t C;
		};
		std::uint16_t BC;
	};

	union
	{
		struct
		{
			std::uint8_t D;
			std::uint8_t E;
		};
		std::uint16_t DE;
	};

	union
	{
		struct
		{
			std::uint8_t H;
			std::uint8_t L;
		};
		std::uint16_t HL;
	};

	std::uint64_t SP;
	std::uint16_t PC;
};

extern Registers reg;

namespace Math
{
	enum class Flags : std::uint8_t
	{
		Z = 0x80,
		N = 0x40,
		H = 0x20,
		C = 0x10
	};


	std::uint8_t Add(std::uint8_t a, std::uint8_t b, std::uint8_t& flags);
	std::uint8_t AddWithCarry(std::uint8_t a, std::uint8_t b, std::uint8_t& flags);
	std::uint8_t Sub(std::uint8_t a, uint8_t b, std::uint8_t& flags);
	std::uint16_t Add(std::uint16_t a, std::uint16_t b, std::uint8_t& flags);

	std::uint8_t Inc(std::uint8_t a, std::uint8_t& flags);
	std::uint8_t Dec(std::uint8_t a, std::uint8_t& flags);
	std::uint16_t Inc(std::uint16_t a);
	std::uint16_t Dec(std::uint16_t a);

	std::uint8_t And(std::uint8_t a, std::uint8_t b, std::uint8_t& flags);
	std::uint8_t Or(std::uint8_t a, std::uint8_t b, std::uint8_t& flags);
	std::uint8_t Xor(std::uint8_t a, std::uint8_t b, std::uint8_t& flags);


	void Example();
}

