#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef char s8;
typedef short s16;


union u16split
{
	struct
	{
		u8 L;
		u8 H;
	};
	u16 Full;
};

