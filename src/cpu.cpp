#include <assert.h>

#include "cpu.h"
#include "math.h"
#include "memory.h"
#include "types.h"

Registers reg;

enum class Opcode : u8
{
	NOP			= 0x00,
	LD_BC_NN	= 0x01,
	LD_$BC_A	= 0x02,
	INC_BC		= 0x03,
	INC_B		= 0x04,
	DEC_B		= 0x05,
	LD_B_N		= 0x06,
	RLCA		= 0x07,
	LD_$NN_SP	= 0x08,
	ADD_HL_BC	= 0x09,
	LD_A_$BC	= 0x0A,
	DEC_BC		= 0x0B,
	INC_C		= 0x0C,
	DEC_C		= 0x0D,
	LD_C_N		= 0x0E,
	RRCA		= 0x0F,

	STOP_0		= 0x10,
	LD_DE_NN	= 0x11,
	LD_$DE_A	= 0x12,
	INC_DE		= 0x13,
	INC_D		= 0x14,
	DEC_D		= 0x15,
	LD_D_N		= 0x16,
	RLA			= 0x17,
	JR_N		= 0x18,
	ADD_HL_DE	= 0x19,
	LD_A_$DE	= 0x1A,
	DEC_DE		= 0x1B,
	INC_E		= 0x1C,
	DEC_E		= 0x1D,
	LD_E_N		= 0x1E,
	RRA			= 0x1F,

	JR_NZ_N		= 0x20,
	LD_HL_NN	= 0x21,
	LD_$HLI_A	= 0x22,
	INC_HL		= 0x23,
	INC_H		= 0x24,
	DEC_H		= 0x25,
	LD_H_N		= 0x26,
	DAA			= 0x27,
	JR_Z_N		= 0x28,
	ADD_HL_HL	= 0x29,
	LD_A_$HLI	= 0x2A,
	DEC_HL		= 0x2B,
	INC_L		= 0x2C,
	DEC_L		= 0x2D,
	LD_L_N		= 0x2E,
	CPL			= 0x2F,

	JR_NC_N		= 0x30,
	LD_SP_NN	= 0x31,
	LD_$HLD_A	= 0x32,
	INC_SP		= 0x33,
	INC_$HL		= 0x34,
	DEC_$HL		= 0x35,
	LD_$HL_N	= 0x36,
	SCF			= 0x37,
	JR_C_N		= 0x38,
	ADD_HL_SP	= 0x39,
	LD_A_$HLD	= 0x3A,
	DEC_SP		= 0x3B,
	INC_A		= 0x3C,
	DEC_A		= 0x3D,
	LD_A_N		= 0x3E,
	CCF			= 0x3F,

	LD_B_B		= 0x40,
	LD_B_C		= 0x41,
	LD_B_D		= 0x42,
	LD_B_E		= 0x43,
	LD_B_H		= 0x44,
	LD_B_L		= 0x45,
	LD_B_$HL	= 0x46,
	LD_B_A		= 0x47,
	LD_C_B		= 0x48,
	LD_C_C		= 0x49,
	LD_C_D		= 0x4A,
	LD_C_E		= 0x4B,
	LD_C_H		= 0x4C,
	LD_C_L		= 0x4D,
	LD_C_$HL	= 0x4E,
	LD_C_A		= 0x4F,

	LD_D_B		= 0x50,
	LD_D_C		= 0x51,
	LD_D_D		= 0x52,
	LD_D_E		= 0x53,
	LD_D_H		= 0x54,
	LD_D_L		= 0x55,
	LD_D_$HL	= 0x56,
	LD_D_A		= 0x57,
	LD_E_B		= 0x58,
	LD_E_C		= 0x59,
	LD_E_D		= 0x5A,
	LD_E_E		= 0x5B,
	LD_E_H		= 0x5C,
	LD_E_L		= 0x5D,
	LD_E_$HL	= 0x5E,
	LD_E_A		= 0x5F,

	LD_H_B		= 0x60,
	LD_H_C		= 0x61,
	LD_H_D		= 0x62,
	LD_H_E		= 0x63,
	LD_H_H		= 0x64,
	LD_H_L		= 0x65,
	LD_H_$HL	= 0x66,
	LD_H_A		= 0x67,
	LD_L_B		= 0x68,
	LD_L_C		= 0x69,
	LD_L_D		= 0x6A,
	LD_L_E		= 0x6B,
	LD_L_H		= 0x6C,
	LD_L_L		= 0x6D,
	LD_L_$HL	= 0x6E,
	LD_L_A		= 0x6F,

	LD_$HL_B	= 0x70,
	LD_$HL_C	= 0x71,
	LD_$HL_D	= 0x72,
	LD_$HL_E	= 0x73,
	LD_$HL_H	= 0x74,
	LD_$HL_L	= 0x75,
	HALT		= 0x76,
	LD_$HL_A	= 0x77,
	LD_A_B		= 0x78,
	LD_A_C		= 0x79,
	LD_A_D		= 0x7A,
	LD_A_E		= 0x7B,
	LD_A_H		= 0x7C,
	LD_A_L		= 0x7D,
	LD_A_$HL	= 0x7E,
	LD_A_A		= 0x7F,

	ADD_A_B		= 0x80,
	ADD_A_C		= 0x81,
	ADD_A_D		= 0x82,
	ADD_A_E		= 0x83,
	ADD_A_H		= 0x84,
	ADD_A_L		= 0x85,
	ADD_A_$HL	= 0x86,
	ADD_A_A		= 0x87,
	ADC_A_B		= 0x88,
	ADC_A_C		= 0x89,
	ADC_A_D		= 0x8A,
	ADC_A_E		= 0x8B,
	ADC_A_H		= 0x8C,
	ADC_A_L		= 0x8D,
	ADC_A_$HL	= 0x8E,
	ADC_A_A		= 0x8F,

	SUB_B		= 0x90,
	SUB_C		= 0x91,
	SUB_D		= 0x92,
	SUB_E		= 0x93,
	SUB_H		= 0x94,
	SUB_L		= 0x95,
	SUB_$HL		= 0x96,
	SUB_A		= 0x97,
	SBC_A_B		= 0x98,
	SBC_A_C		= 0x99,
	SBC_A_D		= 0x9A,
	SBC_A_E		= 0x9B,
	SBC_A_H		= 0x9C,
	SBC_A_L		= 0x9D,
	SBC_A_$HL	= 0x9E,
	SBC_A_A		= 0x9F,

	AND_B		= 0xA0,
	AND_C		= 0xA1,
	AND_D		= 0xA2,
	AND_E		= 0xA3,
	AND_H		= 0xA4,
	AND_L		= 0xA5,
	AND_$HL		= 0xA6,
	AND_A		= 0xA7,
	XOR_B		= 0xA8,
	XOR_C		= 0xA9,
	XOR_D		= 0xAA,
	XOR_E		= 0xAB,
	XOR_H		= 0xAC,
	XOR_L		= 0xAD,
	XOR_$HL		= 0xAE,
	XOR_A		= 0xAF,

	// 8. CP n (Compare)
	CP_A = 0xBF,
	CP_B = 0xB8,
	CP_C = 0xB9,
	CP_D = 0xBA,
	CP_E = 0xBB,
	CP_H = 0xBC,
	CP_L = 0xBD,
	CP_$HL = 0xBE,

	CALL_NN = 0xCD,

	LD_$FF00N_A = 0xE0,
	LD_$FF00C_A = 0xE2,
	XOR_N		= 0xEE,

	LD_A_$NN = 0xFA,
	CP_N = 0xFE,

	// Prefix Bytes
	PREFIX_CB = 0xCB,
	PREFIX_DD = 0xDD,
	PREFIX_ED = 0xED,
	PREFIX_FD = 0xFD,
};

void ProcessOpcodeCB();

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
	
	// 3. LD A,n
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
	case Opcode::LD_A_$BC:
	{
		reg.A = Memory::LoadU8(reg.BC);
		break;
	}
	case Opcode::LD_A_$DE:
	{
		reg.A = Memory::LoadU8(reg.DE);
		break;
	}
	case Opcode::LD_A_$HL:
	{
		reg.A = Memory::LoadU8(reg.HL);
		break;
	}
	case Opcode::LD_A_$NN:
	{
		u16 addr = Memory::LoadU16(reg.PC);
		reg.PC += 2;
		reg.A = Memory::LoadU8(addr);
		break;
	}
	case Opcode::LD_A_N:
	{
		reg.A = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	// 4. LD n,A
	case Opcode::LD_$HL_A:
	{
		Memory::StoreU8(reg.HL, reg.A);
		break;
	}
	// 6. LD (C), A		Put A into address ($FF00 + register C)
	case Opcode::LD_$FF00C_A:
	{
		u16 addr = 0xFF00 + reg.C;
		Memory::StoreU8(addr, reg.A);
		break;
	}
	// 10/11/12 Load A into HL and decrement HL
	case Opcode::LD_$HLD_A:
	{
		Memory::StoreU8(reg.HL, reg.A);
		reg.HL--;
		break;
	}
	// 19 LDH (n),A
	case Opcode::LD_$FF00N_A:
	{
		u8 n = Memory::LoadU8(reg.PC);
		reg.PC++;
		u16 addr = 0xFF00 + n;
		Memory::StoreU8(addr, reg.A);
		break;
	}
	// 3.3.2 16-Bit Loads
	// 1. LD n, nn
	case Opcode::LD_BC_NN:
	{
		reg.BC = Memory::LoadU16(reg.PC);
		reg.PC += 2;
		break;
	}
	case Opcode::LD_DE_NN:
	{
		reg.DE = Memory::LoadU16(reg.PC);
		reg.PC += 2;
		break;
	}
	case Opcode::LD_HL_NN:
	{
		reg.HL = Memory::LoadU16(reg.PC);
		reg.PC += 2;
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
	// 9. INC n
	case Opcode::INC_A:
	{
		Math::Inc(reg.A);
		break;
	}
	case Opcode::INC_B:
	{
		Math::Inc(reg.B);
		break;
	}
	case Opcode::INC_C:
	{
		Math::Inc(reg.C);
		break;
	}
	case Opcode::INC_D:
	{
		Math::Inc(reg.D);
		break;
	}
	case Opcode::INC_E:
	{
		Math::Inc(reg.E);
		break;
	}
	case Opcode::INC_H:
	{
		Math::Inc(reg.H);
		break;
	}
	case Opcode::INC_L:
	{
		Math::Inc(reg.L);
		break;
	}
	case Opcode::INC_$HL:
	{
		u8 value = Memory::LoadU8(reg.HL);
		Math::Inc(value);
		Memory::StoreU8(reg.HL, value);
		break;
	}
	// 3.3.8 Jumps
	// 5. JR cc,n
	case Opcode::JR_NZ_N:
	{
		s8 n = Memory::LoadS8(reg.PC);
		reg.PC++; // The immediate value is part of the instruction
		if (!(reg.F & (u8)Flags::Z))
		{
			reg.PC += n;
		}
		break;
	}
	// 3.3.9 Calls
	// 1. CALL nn
	case Opcode::CALL_NN:
	{
		u16 addr = Memory::LoadU16(reg.PC); // Grab immediate value
		reg.PC += 2;
		Memory::StoreU16(reg.SP, reg.PC); // Store PC on stack
		reg.SP -= 2;
		reg.PC = addr; // "Call"
		break;
	}
	case Opcode::PREFIX_CB:
	case Opcode::PREFIX_DD:
	case Opcode::PREFIX_ED:
	case Opcode::PREFIX_FD:
	{
		ProcessOpcodeCB();
		break;
	}
	default:
		assert(false && "Unknown opcode");
		break;
	}
}

enum class Opcode_CB : u8
{
	// BIT
	BIT_7_H = 0x7C,
};

void ProcessOpcodeCB()
{
	Opcode_CB opcode_cb = (Opcode_CB)Memory::LoadU8(reg.PC);
	reg.PC++;
	switch (opcode_cb)
	{
	case Opcode_CB::BIT_7_H:
	{
		Math::Bit(reg.H, 7);
		break;
	}
	default:
		assert(false && "Unknown opcode");
		break;
	}
}