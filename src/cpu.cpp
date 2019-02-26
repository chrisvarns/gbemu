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

	OR_B		= 0xB0,
	OR_C		= 0xB1,
	OR_D		= 0xB2,
	OR_E		= 0xB3,
	OR_H		= 0xB4,
	OR_L		= 0xB5,
	OR_$HL		= 0xB6,
	OR_A		= 0xB7,
	CP_B		= 0xB8,
	CP_C		= 0xB9,
	CP_D		= 0xBA,
	CP_E		= 0xBB,
	CP_H		= 0xBC,
	CP_L		= 0xBD,
	CP_$HL		= 0xBE,
	CP_A		= 0xBF,

	RET_NZ		= 0xC0,
	POP_BC		= 0xC1,
	JP_NZ_NN	= 0xC2,
	JP_NN		= 0xC3,
	CALL_NZ_NN	= 0xC4,
	PUSH_BC		= 0xC5,
	ADD_A_N		= 0xC6,
	RST_00H		= 0xC7,
	RET_Z		= 0xC8,
	RET			= 0xC9,
	JP_Z_NN		= 0xCA,
	PREFIX_CB	= 0xCB,
	CALL_Z_NN	= 0xCC,
	CALL_NN		= 0xCD,
	ADC_A_N		= 0xCE,
	RST_08H		= 0xCF,

	RET_NC		= 0xD0,
	POP_DE		= 0xD1,
	JP_NC_NN	= 0xD2,
	INVALID_D3	= 0xD3,
	CALL_NC_NN	= 0xD4,
	PUSH_DE		= 0xD5,
	SUB_N		= 0xD6,
	RST_10H		= 0xD7,
	RET_C		= 0xD8,
	RETI		= 0xD9,
	JP_C_NN		= 0xDA,
	INVALID_DB	= 0xDB,
	CALL_C_NN	= 0xDC,
	INVALID_DD	= 0xDD,
	SBC_A_N		= 0xDE,
	RST_18H		= 0xDF,

	LDH_$N_A	= 0xE0,
	POP_HL		= 0xE1,
	LD_$C_A		= 0xE2,
	INVALID_E3	= 0xE3,
	INVALID_E4	= 0xE4,
	PUSH_HL		= 0xE5,
	AND_N		= 0xE6,
	RST_20H		= 0xE7,
	ADD_SP_N	= 0xE8,
	JP_$HL		= 0xE9,
	LD_$NN_A	= 0xEA,
	INVALID_EB	= 0xEB,
	INVALID_EC	= 0xEC,
	INVALID_ED	= 0xED,
	XOR_N		= 0xEE,
	RST_28H		= 0xEF,

	LDH_A_$N	= 0xF0,
	POP_AF		= 0xF1,
	LD_A_$C		= 0xF2,
	DI			= 0xF3,
	INVALID_F4	= 0xF4,
	PUSH_AF		= 0xF5,
	OR_N		= 0xF6,
	RST_30H		= 0xF7,
	LD_HL_SPN	= 0xF8,
	LD_SP_HL	= 0xF9,
	LD_A_$NN	= 0xFA,
	EI			= 0xFB,
	INVALID_FC	= 0xFC,
	INVALID_FD	= 0xFD,
	CP_N		= 0xFE,
	RST_38H		= 0xFF,
};

void ProcessOpcodeCB();

void CPU::step()
{
	// Get opcode from memory
	Opcode opcode = (Opcode)Memory::LoadU8(reg.PC);
	reg.PC++;

	switch (opcode)
	{
	// 0x00
	{
	case Opcode::NOP:
	{
		// Do nothing
		break;
	}
	case Opcode::LD_BC_NN:
	{
		reg.BC = Memory::LoadU16(reg.PC);
		reg.PC += 2;
		break;
	}
	case Opcode::INC_B:
	{
		Math::Inc(reg.B);
		break;
	}
	case Opcode::LD_B_N:
	{
		reg.B = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	case Opcode::LD_A_$BC:
	{
		reg.A = Memory::LoadU8(reg.BC);
		break;
	}
	case Opcode::INC_C:
	{
		Math::Inc(reg.C);
		break;
	}
	case Opcode::LD_C_N:
	{
		reg.C = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	}
	// 0x10
	{
	case Opcode::LD_DE_NN:
	{
		reg.DE = Memory::LoadU16(reg.PC);
		reg.PC += 2;
		break;
	}
	case Opcode::INC_D:
	{
		Math::Inc(reg.D);
		break;
	}
	case Opcode::LD_D_N:
	{
		reg.D = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	case Opcode::LD_A_$DE:
	{
		reg.A = Memory::LoadU8(reg.DE);
		break;
	}
	case Opcode::INC_E:
	{
		Math::Inc(reg.E);
		break;
	}
	case Opcode::LD_E_N:
	{
		reg.E = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	}
	// 0x20
	{
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
	case Opcode::LD_HL_NN:
	{
		reg.HL = Memory::LoadU16(reg.PC);
		reg.PC += 2;
		break;
	}
	case Opcode::INC_H:
	{
		Math::Inc(reg.H);
		break;
	}
	case Opcode::LD_H_N:
	{
		reg.H = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	case Opcode::INC_L:
	{
		Math::Inc(reg.L);
		break;
	}
	case Opcode::LD_L_N:
	{
		reg.L = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	}
	// 0x30
	{
	case Opcode::LD_SP_NN:
	{
		reg.SP = Memory::LoadU16(reg.PC);
		reg.PC += 2;
		break;
	}
	case Opcode::LD_$HLD_A:
	{
		Memory::StoreU8(reg.HL, reg.A);
		reg.HL--;
		break;
	}
	case Opcode::INC_$HL:
	{
		u8 value = Memory::LoadU8(reg.HL);
		Math::Inc(value);
		Memory::StoreU8(reg.HL, value);
		break;
	}
	case Opcode::INC_A:
	{
		Math::Inc(reg.A);
		break;
	}
	case Opcode::LD_A_N:
	{
		reg.A = Memory::LoadU8(reg.PC);
		reg.PC++;
		break;
	}
	}
	// 0x40
	{
	case Opcode::LD_C_A:
	{
		reg.C = reg.A;
		break;
	}
	}
	// 0x50
	{
	}
	// 0x60
	{
	}
	// 0x70
	{
	case Opcode::LD_$HL_A:
	{
		Memory::StoreU8(reg.HL, reg.A);
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
	case Opcode::LD_A_$HL:
	{
		reg.A = Memory::LoadU8(reg.HL);
		break;
	}
	case Opcode::LD_A_A:
	{
		reg.A = reg.A;
		break;
	}
	}
	// 0x80
	{
	}
	// 0x90
	{
	}
	// 0xA0
	{
	case Opcode::XOR_A:
	{
		Math::Xor(reg.A);
		break;
	}
	}
	// 0xB0
	{
	}
	// 0xC0
	{
	case Opcode::PUSH_BC:
	{
		Memory::StoreU16(reg.SP, reg.BC);
		reg.SP -= 2;
		break;
	}
	case Opcode::PREFIX_CB:
	{
		ProcessOpcodeCB();
		break;
	}
	case Opcode::CALL_NN:
	{
		u16 addr = Memory::LoadU16(reg.PC); // Grab immediate value
		reg.PC += 2;
		Memory::StoreU16(reg.SP, reg.PC); // Store PC on stack
		reg.SP -= 2;
		reg.PC = addr; // "Call"
		break;
	}
	}
	// 0xD0
	{
	}
	// 0xE0
	{
	case Opcode::LDH_$N_A:
	{
		u8 n = Memory::LoadU8(reg.PC);
		reg.PC++;
		u16 addr = 0xFF00 + n;
		Memory::StoreU8(addr, reg.A);
		break;
	}
	case Opcode::LD_$C_A:
	{
		u16 addr = 0xFF00 + reg.C;
		Memory::StoreU8(addr, reg.A);
		break;
	}
	}
	// 0xF0
	{
	case Opcode::LD_A_$NN:
	{
		u16 addr = Memory::LoadU16(reg.PC);
		reg.PC += 2;
		reg.A = Memory::LoadU8(addr);
		break;
	}
	case Opcode::CP_N:
	{
		u8 n = Memory::LoadU8(reg.PC);
		reg.PC++;
		Math::Compare(n);
		break;
	}
	}
	default:
		assert(false && "Unknown opcode");
		break;
	}
}

enum class Opcode_CB : u8
{
	RLC_B		= 0x00,
	RLC_C		= 0x01,
	RLC_D		= 0x02,
	RLC_E		= 0x03,
	RLC_H		= 0x04,
	RLC_L		= 0x05,
	RLC_$HL		= 0x06,
	RLC_A		= 0x07,
	RRC_B		= 0x08,
	RRC_C		= 0x09,
	RRC_D		= 0x0A,
	RRC_E		= 0x0B,
	RRC_H		= 0x0C,
	RRC_L		= 0x0D,
	RRC_$HL		= 0x0E,
	RRC_A		= 0x0F,

	RL_B		= 0x10,
	RL_C		= 0x11,
	RL_D		= 0x12,
	RL_E		= 0x13,
	RL_H		= 0x14,
	RL_L		= 0x15,
	RL_$HL		= 0x16,
	RL_A		= 0x17,
	RR_B		= 0x18,
	RR_C		= 0x19,
	RR_D		= 0x1A,
	RR_E		= 0x1B,
	RR_H		= 0x1C,
	RR_L		= 0x1D,
	RR_$HL		= 0x1E,
	RR_A		= 0x1F,

	SLA_B		= 0x20,
	SLA_C		= 0x21,
	SLA_D		= 0x22,
	SLA_E		= 0x23,
	SLA_H		= 0x24,
	SLA_L		= 0x25,
	SLA_$HL		= 0x26,
	SLA_A		= 0x27,
	SRA_B		= 0x28,
	SRA_C		= 0x29,
	SRA_D		= 0x2A,
	SRA_E		= 0x2B,
	SRA_H		= 0x2C,
	SRA_L		= 0x2D,
	SRA_$HL		= 0x2E,
	SRA_A		= 0x2F,

	SWAP_B		= 0x30,
	SWAP_C		= 0x31,
	SWAP_D		= 0x32,
	SWAP_E		= 0x33,
	SWAP_H		= 0x34,
	SWAP_L		= 0x35,
	SWAP_$HL	= 0x36,
	SWAP_A		= 0x37,
	SRL_B		= 0x38,
	SRL_C		= 0x39,
	SRL_D		= 0x3A,
	SRL_E		= 0x3B,
	SRL_H		= 0x3C,
	SRL_L		= 0x3D,
	SRL_$HL		= 0x3E,
	SRL_A		= 0x3F,

	BIT_0_B		= 0x40,
	BIT_0_C		= 0x41,
	BIT_0_D		= 0x42,
	BIT_0_E		= 0x43,
	BIT_0_H		= 0x44,
	BIT_0_L		= 0x45,
	BIT_0_$HL	= 0x46,
	BIT_0_A		= 0x47,
	BIT_1_B		= 0x48,
	BIT_1_C		= 0x49,
	BIT_1_D		= 0x4A,
	BIT_1_E		= 0x4B,
	BIT_1_H		= 0x4C,
	BIT_1_L		= 0x4D,
	BIT_1_$HL	= 0x4E,
	BIT_1_A		= 0x4F,

	BIT_2_B		= 0x50,
	BIT_2_C		= 0x51,
	BIT_2_D		= 0x52,
	BIT_2_E		= 0x53,
	BIT_2_H		= 0x54,
	BIT_2_L		= 0x55,
	BIT_2_$HL	= 0x56,
	BIT_2_A		= 0x57,
	BIT_3_B		= 0x58,
	BIT_3_C		= 0x59,
	BIT_3_D		= 0x5A,
	BIT_3_E		= 0x5B,
	BIT_3_H		= 0x5C,
	BIT_3_L		= 0x5D,
	BIT_3_$HL	= 0x5E,
	BIT_3_A		= 0x5F,

	BIT_4_B		= 0x60,
	BIT_4_C		= 0x61,
	BIT_4_D		= 0x62,
	BIT_4_E		= 0x63,
	BIT_4_H		= 0x64,
	BIT_4_L		= 0x65,
	BIT_4_$HL	= 0x66,
	BIT_4_A		= 0x67,
	BIT_5_B		= 0x68,
	BIT_5_C		= 0x69,
	BIT_5_D		= 0x6A,
	BIT_5_E		= 0x6B,
	BIT_5_H		= 0x6C,
	BIT_5_L		= 0x6D,
	BIT_5_$HL	= 0x6E,
	BIT_5_A		= 0x6F,

	BIT_6_B		= 0x70,
	BIT_6_C		= 0x71,
	BIT_6_D		= 0x72,
	BIT_6_E		= 0x73,
	BIT_6_H		= 0x74,
	BIT_6_L		= 0x75,
	BIT_6_$HL	= 0x76,
	BIT_6_A		= 0x77,
	BIT_7_B		= 0x78,
	BIT_7_C		= 0x79,
	BIT_7_D		= 0x7A,
	BIT_7_E		= 0x7B,
	BIT_7_H		= 0x7C,
	BIT_7_L		= 0x7D,
	BIT_7_$HL	= 0x7E,
	BIT_7_A		= 0x7F,

	RES_0_B		= 0x80,
	RES_0_C		= 0x81,
	RES_0_D		= 0x82,
	RES_0_E		= 0x83,
	RES_0_H		= 0x84,
	RES_0_L		= 0x85,
	RES_0_$HL	= 0x86,
	RES_0_A		= 0x87,
	RES_1_B		= 0x88,
	RES_1_C		= 0x89,
	RES_1_D		= 0x8A,
	RES_1_E		= 0x8B,
	RES_1_H		= 0x8C,
	RES_1_L		= 0x8D,
	RES_1_$HL	= 0x8E,
	RES_1_A		= 0x8F,

	RES_2_B		= 0x90,
	RES_2_C		= 0x91,
	RES_2_D		= 0x92,
	RES_2_E		= 0x93,
	RES_2_H		= 0x94,
	RES_2_L		= 0x95,
	RES_2_$HL	= 0x96,
	RES_2_A		= 0x97,
	RES_3_B		= 0x98,
	RES_3_C		= 0x99,
	RES_3_D		= 0x9A,
	RES_3_E		= 0x9B,
	RES_3_H		= 0x9C,
	RES_3_L		= 0x9D,
	RES_3_$HL	= 0x9E,
	RES_3_A		= 0x9F,

	RES_4_B		= 0xA0,
	RES_4_C		= 0xA1,
	RES_4_D		= 0xA2,
	RES_4_E		= 0xA3,
	RES_4_H		= 0xA4,
	RES_4_L		= 0xA5,
	RES_4_$HL	= 0xA6,
	RES_4_A		= 0xA7,
	RES_5_B		= 0xA8,
	RES_5_C		= 0xA9,
	RES_5_D		= 0xAA,
	RES_5_E		= 0xAB,
	RES_5_H		= 0xAC,
	RES_5_L		= 0xAD,
	RES_5_$HL	= 0xAE,
	RES_5_A		= 0xAF,

	RES_6_B		= 0xB0,
	RES_6_C		= 0xB1,
	RES_6_D		= 0xB2,
	RES_6_E		= 0xB3,
	RES_6_H		= 0xB4,
	RES_6_L		= 0xB5,
	RES_6_$HL	= 0xB6,
	RES_6_A		= 0xB7,
	RES_7_B		= 0xB8,
	RES_7_C		= 0xB9,
	RES_7_D		= 0xBA,
	RES_7_E		= 0xBB,
	RES_7_H		= 0xBC,
	RES_7_L		= 0xBD,
	RES_7_$HL	= 0xBE,
	RES_7_A		= 0xBF,

	SET_0_B		= 0xC0,
	SET_0_C		= 0xC1,
	SET_0_D		= 0xC2,
	SET_0_E		= 0xC3,
	SET_0_H		= 0xC4,
	SET_0_L		= 0xC5,
	SET_0_$HL	= 0xC6,
	SET_0_A		= 0xC7,
	SET_1_B		= 0xC8,
	SET_1_C		= 0xC9,
	SET_1_D		= 0xCA,
	SET_1_E		= 0xCB,
	SET_1_H		= 0xCC,
	SET_1_L		= 0xCD,
	SET_1_$HL	= 0xCE,
	SET_1_A		= 0xCF,

	SET_2_B		= 0xD0,
	SET_2_C		= 0xD1,
	SET_2_D		= 0xD2,
	SET_2_E		= 0xD3,
	SET_2_H		= 0xD4,
	SET_2_L		= 0xD5,
	SET_2_$HL	= 0xD6,
	SET_2_A		= 0xD7,
	SET_3_B		= 0xD8,
	SET_3_C		= 0xD9,
	SET_3_D		= 0xDA,
	SET_3_E		= 0xDB,
	SET_3_H		= 0xDC,
	SET_3_L		= 0xDD,
	SET_3_$HL	= 0xDE,
	SET_3_A		= 0xDF,

	SET_4_B		= 0xE0,
	SET_4_C		= 0xE1,
	SET_4_D		= 0xE2,
	SET_4_E		= 0xE3,
	SET_4_H		= 0xE4,
	SET_4_L		= 0xE5,
	SET_4_$HL	= 0xE6,
	SET_4_A		= 0xE7,
	SET_5_B		= 0xE8,
	SET_5_C		= 0xE9,
	SET_5_D		= 0xEA,
	SET_5_E		= 0xEB,
	SET_5_H		= 0xEC,
	SET_5_L		= 0xED,
	SET_5_$HL	= 0xEE,
	SET_5_A		= 0xEF,

	SET_6_B		= 0xF0,
	SET_6_C		= 0xF1,
	SET_6_D		= 0xF2,
	SET_6_E		= 0xF3,
	SET_6_H		= 0xF4,
	SET_6_L		= 0xF5,
	SET_6_$HL	= 0xF6,
	SET_6_A		= 0xF7,
	SET_7_B		= 0xF8,
	SET_7_C		= 0xF9,
	SET_7_D		= 0xFA,
	SET_7_E		= 0xFB,
	SET_7_H		= 0xFC,
	SET_7_L		= 0xFD,
	SET_7_$HL	= 0xFE,
	SET_7_A		= 0xFF,
};

void ProcessOpcodeCB()
{
	Opcode_CB opcode_cb = (Opcode_CB)Memory::LoadU8(reg.PC);
	reg.PC++;
	switch (opcode_cb)
	{
	// 0x00
	{
	}
	// 0x10
	{
	case Opcode_CB::RL_B:
	{
		Math::RotateLeft(reg.B);
		break;
	}
	case Opcode_CB::RL_C:
	{
		Math::RotateLeft(reg.C);
		break;
	}
	case Opcode_CB::RL_D:
	{
		Math::RotateLeft(reg.D);
		break;
	}
	case Opcode_CB::RL_E:
	{
		Math::RotateLeft(reg.E);
		break;
	}
	case Opcode_CB::RL_H:
	{
		Math::RotateLeft(reg.H);
		break;
	}
	case Opcode_CB::RL_L:
	{
		Math::RotateLeft(reg.L);
		break;
	}
	case Opcode_CB::RL_$HL:
	{
		u8 val = Memory::LoadU8(reg.HL);
		Math::RotateLeft(val);
		Memory::StoreU8(reg.HL, val);
		break;
	}
	case Opcode_CB::RL_A:
	{
		Math::RotateLeft(reg.A);
		break;
	}
	}
	// 0x20
	{
	}
	// 0x30
	{
	}
	// 0x40
	{
	}
	// 0x50
	{
	}
	// 0x60
	{
	}
	// 0x70
	{
	case Opcode_CB::BIT_7_H:
	{
		Math::Bit(reg.H, 7);
		break;
	}
	}
	// 0x80
	{
	}
	// 0x90
	{
	}
	// 0xA0
	{
	}
	// 0xB0
	{
	}
	// 0xC0
	{
	}
	// 0xD0
	{
	}
	// 0xE0
	{
	}
	// 0xF0
	{
	}
	default:
		assert(false && "Unknown opcode");
		break;
	}
}