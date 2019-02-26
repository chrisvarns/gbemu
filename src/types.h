#pragma once
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;


union u16split
{
	struct
	{
		u8 L;
		u8 H;
	};
	uint16_t Full;
};

