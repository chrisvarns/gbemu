#include <assert.h>

#include "cpu.h"
#include "math.h"
#include "memory.h"
#include "types.h"

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

	// 3.3.2 16-Bit Loads
	// 1. LD n, nn
	LD_BC_NN = 0x01,
	LD_DE_NN = 0x11,
	LD_HL_NN = 0x21,
	LD_SP_NN = 0x31,

	// 3.3.3 8-bit ALU
	// 7. XOR n
	XOR_A = 0xAF,
	XOR_B = 0xA8,
	XOR_C = 0xA9,
	XOR_D = 0xAA,
	XOR_E = 0xAB,
	XOR_H = 0xAC,
	XOR_L = 0xAD,
	XOR_$HL = 0xAE,
	XOR_N = 0xEE,

	// 8. CP n (Compare)
	CP_A = 0xBF,
	CP_B = 0xB8,
	CP_C = 0xB9,
	CP_D = 0xBA,
	CP_E = 0xBB,
	CP_H = 0xBC,
	CP_L = 0xBD,
	CP_$HL = 0xBE,
	CP_N = 0xFE
};

void CPU::step()
{
	// Get opcode from memory
	Opcode opcode = (Opcode)Memory::LoadU8(reg.PC);
	reg.PC++;

	switch (opcode)
	{
	// 3.3.1 8-bit loads
	// 1. LD r1,val (r1 = 8 bit immediate value)
	case Opcode::LD_B_N:
	{
		reg.B = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	case Opcode::LD_C_N:
	{
		reg.C = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	case Opcode::LD_D_N:
	{
		reg.D = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	case Opcode::LD_E_N:
	{
		reg.E = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	case Opcode::LD_H_N:
	{
		reg.H = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	case Opcode::LD_L_N:
	{
		reg.L = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	// 2. LD r1,r2 (r1 = r2)
	case Opcode::LD_A_A:
	{
		reg.A = reg.A;
		break;
	}
	case Opcode::LD_A_B:
	{
		reg.A = reg.B;
		break;
	}
	case Opcode::LD_A_C:
	{
		reg.A = reg.C;
		break;
	}
	case Opcode::LD_A_D:
	{
		reg.A = reg.D;
		break;
	}
	case Opcode::LD_A_E:
	{
		reg.A = reg.E;
		break;
	}
	case Opcode::LD_A_H:
	{
		reg.A = reg.H;
		break;
	}
	case Opcode::LD_A_L:
	{
		reg.A = reg.L;
		break;
	}
	// 3.3.2 16-Bit Loads
	// 1. LD n, nn
	case Opcode::LD_BC_NN:
	{
		assert(false && "Missing opcode");
		break;
	}
	case Opcode::LD_DE_NN:
	{
		assert(false && "Missing opcode");
		break;
	}
	case Opcode::LD_HL_NN:
	{
		assert(false && "Missing opcode");
		break;
	}
	case Opcode::LD_SP_NN:
	{
		reg.SP = Memory::LoadU16(reg.PC);
		reg.PC += 2;
		break;
	}
	// 3.3.3 8-bit ALU
	// 7. XOR n
	case Opcode::XOR_A:
	{
		Math::Xor(reg.A);
		break;
	}
	// 8. CP n (Compare)
	case Opcode::CP_N:
	{
		u8 n = Memory::LoadU8(reg.PC);
		reg.PC++;
		Math::Compare(n);
		break;
	}
	default:
		assert(false && "Unknown opcode");
		break;
	}
}