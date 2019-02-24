#include <assert.h>

#include "cpu.h"
#include "types.h"
#include "memory.h"

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

Registers reg;

enum class Opcode : u8
{
	// 3.3.1 8-bit loads
	// 1. LD r1,val (r1 = 8 bit immediate value)
	LD_B_N = 0x06,
	LD_C_N = 0x0E,
	LD_D_N = 0x16,
	LD_E_N = 0x1E,
	LD_H_N = 0x26,
	LD_L_N = 0x2E,

	// 2. LD r1,r2 (r1 = r2)
	// @todo don't really understand what any of the immediate loads or (HL) variants are doing here.
	LD_A_A = 0x7F,
	LD_A_B = 0x78,
	LD_A_C = 0x79,
	LD_A_D = 0x7A,
	LD_A_E = 0x7B,
	LD_A_H = 0x7C,
	LD_A_L = 0x7D,
	LD_A_$HL = 0x7E,
	LD_B_B = 0x40,
	LD_B_C = 0x41,
	LD_B_D = 0x42,
	LD_B_E = 0x43,
	LD_B_H = 0x44,
	LD_B_L = 0x45,
	LD_B_$HL = 0x46,
	LD_C_B = 0x48,
	LD_C_C = 0x49,
	LD_C_D = 0x4A,
	LD_C_E = 0x4B,
	LD_C_H = 0x4C,
	LD_C_L = 0x4D,
	LD_C_$HL = 0x4E,
	LD_D_B = 0x50,
	LD_D_C = 0x51,	LD_D_D = 0x52,
	LD_D_E = 0x53,
	LD_D_H = 0x54,
	LD_D_L = 0x55,
	LD_D_$HL = 0x56,
	LD_E_B = 0x58,
	LD_E_C = 0x59,
	LD_E_D = 0x5A,
	LD_E_E = 0x5B,
	LD_E_H = 0x5C,
	LD_E_L = 0x5D,
	LD_E_$HL = 0x5E,
	LD_H_B = 0x60,
	LD_H_C = 0x61,
	LD_H_D = 0x62,
	LD_H_E = 0x63,
	LD_H_H = 0x64,
	LD_H_L = 0x65,
	LD_H_$HL = 0x66,
	LD_L_B = 0x68,
	LD_L_C = 0x69,
	LD_L_D = 0x6A,
	LD_L_E = 0x6B,
	LD_L_H = 0x6C,
	LD_L_L = 0x6D,
	LD_L_$HL = 0x6E,
	LD_$HL_B = 0x70,
	LD_$HL_C = 0x71,
	LD_$HL_D = 0x72,
	LD_$HL_E = 0x73,
	LD_$HL_H = 0x74,
	LD_$HL_L = 0x75,	LD_$HL_N = 0x36,
};

void CPU::step()
{
	// Get opcode from memory
	Opcode opcode = (Opcode)Memory::LoadU8(reg.PC);

	switch (opcode)
	{
	case Opcode::LD_B_N:
	{
		reg.B = Memory::LoadU8(reg.PC);
		reg.PC++;
	}
	break;
	case Opcode::LD_C_N:
	{
		reg.C = Memory::LoadU8(reg.PC);
		reg.PC++;
	}
	break;
	case Opcode::LD_D_N:
	{
		reg.D = Memory::LoadU8(reg.PC);
		reg.PC++;
	}
	break;
	case Opcode::LD_E_N:
	{
		reg.E = Memory::LoadU8(reg.PC);
		reg.PC++;
	}
	break;
	case Opcode::LD_H_N:
	{
		reg.H = Memory::LoadU8(reg.PC);
		reg.PC++;
	}
	break;
	case Opcode::LD_L_N:
	{
		reg.L = Memory::LoadU8(reg.PC);
		reg.PC++;
	}
	break;
	case Opcode::LD_A_A:
	{
		reg.A = reg.A;
	}
	break;
	case Opcode::LD_A_B:
	{
		reg.A = reg.B;
	}
	break;
	case Opcode::LD_A_C:
	{
		reg.A = reg.C;
	}
	break;
	case Opcode::LD_A_D:
	{
		reg.A = reg.D;
	}
	break;
	case Opcode::LD_A_E:
	{
		reg.A = reg.E;
	}
	break;
	case Opcode::LD_A_H:
	{
		reg.A = reg.H;
	}
	break;
	case Opcode::LD_A_L:
	{
		reg.A = reg.L;
	}
	break;
	default:
		assert(false && "Unknown opcode");
		break;
	}
}