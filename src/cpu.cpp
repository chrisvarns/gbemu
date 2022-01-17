#include <assert.h>
#include <queue>

#include "cpu.h"

#include "Bus.h"
#include "constants.h"
#include "math.h"
#include "types.h"

Registers reg;
std::queue<void(*)(void)> instructions;

static bool bHalted = false;
static bool bRepeatPCPostHalt = false;

static int interruptDisableDelay = -1;
static int interruptEnableDelay = -1;

static bool interruptMasterEnable = false;

enum class Opcode : u8
{
	NOP = 0x00,
	LD_BC_NN = 0x01,
	LD_$BC_A = 0x02,
	INC_BC = 0x03,
	INC_B = 0x04,
	DEC_B = 0x05,
	LD_B_N = 0x06,
	RLCA = 0x07,
	LD_$NN_SP = 0x08,
	ADD_HL_BC = 0x09,
	LD_A_$BC = 0x0A,
	DEC_BC = 0x0B,
	INC_C = 0x0C,
	DEC_C = 0x0D,
	LD_C_N = 0x0E,
	RRCA = 0x0F,

	STOP_0 = 0x10,
	LD_DE_NN = 0x11,
	LD_$DE_A = 0x12,
	INC_DE = 0x13,
	INC_D = 0x14,
	DEC_D = 0x15,
	LD_D_N = 0x16,
	RLA = 0x17,
	JR_N = 0x18,
	ADD_HL_DE = 0x19,
	LD_A_$DE = 0x1A,
	DEC_DE = 0x1B,
	INC_E = 0x1C,
	DEC_E = 0x1D,
	LD_E_N = 0x1E,
	RRA = 0x1F,

	JR_NZ_N = 0x20,
	LD_HL_NN = 0x21,
	LD_$HLI_A = 0x22,
	INC_HL = 0x23,
	INC_H = 0x24,
	DEC_H = 0x25,
	LD_H_N = 0x26,
	DAA = 0x27,
	JR_Z_N = 0x28,
	ADD_HL_HL = 0x29,
	LD_A_$HLI = 0x2A,
	DEC_HL = 0x2B,
	INC_L = 0x2C,
	DEC_L = 0x2D,
	LD_L_N = 0x2E,
	CPL = 0x2F,

	JR_NC_N = 0x30,
	LD_SP_NN = 0x31,
	LD_$HLD_A = 0x32,
	INC_SP = 0x33,
	INC_$HL = 0x34,
	DEC_$HL = 0x35,
	LD_$HL_N = 0x36,
	SCF = 0x37,
	JR_C_N = 0x38,
	ADD_HL_SP = 0x39,
	LD_A_$HLD = 0x3A,
	DEC_SP = 0x3B,
	INC_A = 0x3C,
	DEC_A = 0x3D,
	LD_A_N = 0x3E,
	CCF = 0x3F,

	LD_B_B = 0x40,
	LD_B_C = 0x41,
	LD_B_D = 0x42,
	LD_B_E = 0x43,
	LD_B_H = 0x44,
	LD_B_L = 0x45,
	LD_B_$HL = 0x46,
	LD_B_A = 0x47,
	LD_C_B = 0x48,
	LD_C_C = 0x49,
	LD_C_D = 0x4A,
	LD_C_E = 0x4B,
	LD_C_H = 0x4C,
	LD_C_L = 0x4D,
	LD_C_$HL = 0x4E,
	LD_C_A = 0x4F,

	LD_D_B = 0x50,
	LD_D_C = 0x51,
	LD_D_D = 0x52,
	LD_D_E = 0x53,
	LD_D_H = 0x54,
	LD_D_L = 0x55,
	LD_D_$HL = 0x56,
	LD_D_A = 0x57,
	LD_E_B = 0x58,
	LD_E_C = 0x59,
	LD_E_D = 0x5A,
	LD_E_E = 0x5B,
	LD_E_H = 0x5C,
	LD_E_L = 0x5D,
	LD_E_$HL = 0x5E,
	LD_E_A = 0x5F,

	LD_H_B = 0x60,
	LD_H_C = 0x61,
	LD_H_D = 0x62,
	LD_H_E = 0x63,
	LD_H_H = 0x64,
	LD_H_L = 0x65,
	LD_H_$HL = 0x66,
	LD_H_A = 0x67,
	LD_L_B = 0x68,
	LD_L_C = 0x69,
	LD_L_D = 0x6A,
	LD_L_E = 0x6B,
	LD_L_H = 0x6C,
	LD_L_L = 0x6D,
	LD_L_$HL = 0x6E,
	LD_L_A = 0x6F,

	LD_$HL_B = 0x70,
	LD_$HL_C = 0x71,
	LD_$HL_D = 0x72,
	LD_$HL_E = 0x73,
	LD_$HL_H = 0x74,
	LD_$HL_L = 0x75,
	HALT = 0x76,
	LD_$HL_A = 0x77,
	LD_A_B = 0x78,
	LD_A_C = 0x79,
	LD_A_D = 0x7A,
	LD_A_E = 0x7B,
	LD_A_H = 0x7C,
	LD_A_L = 0x7D,
	LD_A_$HL = 0x7E,
	LD_A_A = 0x7F,

	ADD_A_B = 0x80,
	ADD_A_C = 0x81,
	ADD_A_D = 0x82,
	ADD_A_E = 0x83,
	ADD_A_H = 0x84,
	ADD_A_L = 0x85,
	ADD_A_$HL = 0x86,
	ADD_A_A = 0x87,
	ADC_A_B = 0x88,
	ADC_A_C = 0x89,
	ADC_A_D = 0x8A,
	ADC_A_E = 0x8B,
	ADC_A_H = 0x8C,
	ADC_A_L = 0x8D,
	ADC_A_$HL = 0x8E,
	ADC_A_A = 0x8F,

	SUB_B = 0x90,
	SUB_C = 0x91,
	SUB_D = 0x92,
	SUB_E = 0x93,
	SUB_H = 0x94,
	SUB_L = 0x95,
	SUB_$HL = 0x96,
	SUB_A = 0x97,
	SBC_A_B = 0x98,
	SBC_A_C = 0x99,
	SBC_A_D = 0x9A,
	SBC_A_E = 0x9B,
	SBC_A_H = 0x9C,
	SBC_A_L = 0x9D,
	SBC_A_$HL = 0x9E,
	SBC_A_A = 0x9F,

	AND_B = 0xA0,
	AND_C = 0xA1,
	AND_D = 0xA2,
	AND_E = 0xA3,
	AND_H = 0xA4,
	AND_L = 0xA5,
	AND_$HL = 0xA6,
	AND_A = 0xA7,
	XOR_B = 0xA8,
	XOR_C = 0xA9,
	XOR_D = 0xAA,
	XOR_E = 0xAB,
	XOR_H = 0xAC,
	XOR_L = 0xAD,
	XOR_$HL = 0xAE,
	XOR_A = 0xAF,

	OR_B = 0xB0,
	OR_C = 0xB1,
	OR_D = 0xB2,
	OR_E = 0xB3,
	OR_H = 0xB4,
	OR_L = 0xB5,
	OR_$HL = 0xB6,
	OR_A = 0xB7,
	CP_B = 0xB8,
	CP_C = 0xB9,
	CP_D = 0xBA,
	CP_E = 0xBB,
	CP_H = 0xBC,
	CP_L = 0xBD,
	CP_$HL = 0xBE,
	CP_A = 0xBF,

	RET_NZ = 0xC0,
	POP_BC = 0xC1,
	JP_NZ_NN = 0xC2,
	JP_NN = 0xC3,
	CALL_NZ_NN = 0xC4,
	PUSH_BC = 0xC5,
	ADD_A_N = 0xC6,
	RST_00H = 0xC7,
	RET_Z = 0xC8,
	RET = 0xC9,
	JP_Z_NN = 0xCA,
	PREFIX_CB = 0xCB,
	CALL_Z_NN = 0xCC,
	CALL_NN = 0xCD,
	ADC_A_N = 0xCE,
	RST_08H = 0xCF,

	RET_NC = 0xD0,
	POP_DE = 0xD1,
	JP_NC_NN = 0xD2,
	INVALID_D3 = 0xD3,
	CALL_NC_NN = 0xD4,
	PUSH_DE = 0xD5,
	SUB_N = 0xD6,
	RST_10H = 0xD7,
	RET_C = 0xD8,
	RETI = 0xD9,
	JP_C_NN = 0xDA,
	INVALID_DB = 0xDB,
	CALL_C_NN = 0xDC,
	INVALID_DD = 0xDD,
	SBC_A_N = 0xDE,
	RST_18H = 0xDF,

	LDH_$N_A = 0xE0,
	POP_HL = 0xE1,
	LD_$C_A = 0xE2,
	INVALID_E3 = 0xE3,
	INVALID_E4 = 0xE4,
	PUSH_HL = 0xE5,
	AND_N = 0xE6,
	RST_20H = 0xE7,
	ADD_SP_N = 0xE8,
	JP_$HL = 0xE9,
	LD_$NN_A = 0xEA,
	INVALID_EB = 0xEB,
	INVALID_EC = 0xEC,
	INVALID_ED = 0xED,
	XOR_N = 0xEE,
	RST_28H = 0xEF,

	LDH_A_$N = 0xF0,
	POP_AF = 0xF1,
	LD_A_$C = 0xF2,
	DI = 0xF3,
	INVALID_F4 = 0xF4,
	PUSH_AF = 0xF5,
	OR_N = 0xF6,
	RST_30H = 0xF7,
	LD_HL_SPN = 0xF8,
	LD_SP_HL = 0xF9,
	LD_A_$NN = 0xFA,
	EI = 0xFB,
	INVALID_FC = 0xFC,
	INVALID_FD = 0xFD,
	CP_N = 0xFE,
	RST_38H = 0xFF,
};

struct OpcodeTiming
{
	s8 ifTaken = 0;
	s8 ifNotTaken = 0;

	OpcodeTiming(s8 a) : ifTaken(a), ifNotTaken(-1) {};
	OpcodeTiming(s8 a, s8 b) : ifTaken(a), ifNotTaken(b) {};
};

static OpcodeTiming OpcodeTimings[0x100]
{
	// 0x00
	OpcodeTiming(4),
	OpcodeTiming(12),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(20),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0x10
	OpcodeTiming(4),
	OpcodeTiming(12),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(12),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0x20
	OpcodeTiming(12, 8),
	OpcodeTiming(12),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(12, 8),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0x30
	OpcodeTiming(12, 8),
	OpcodeTiming(12),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(12),
	OpcodeTiming(12),
	OpcodeTiming(12),
	OpcodeTiming(4),
	OpcodeTiming(12, 8),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0x40
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0x50
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0x60
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0x70
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0x80
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0x90
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0xA0
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0xB0
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(4),
	OpcodeTiming(8),
	OpcodeTiming(4),
	// 0xC0
	OpcodeTiming(20, 8),
	OpcodeTiming(12),
	OpcodeTiming(16, 12),
	OpcodeTiming(16),
	OpcodeTiming(24, 12),
	OpcodeTiming(16),
	OpcodeTiming(8),
	OpcodeTiming(16),
	OpcodeTiming(20, 8),
	OpcodeTiming(16),
	OpcodeTiming(16, 12),
	OpcodeTiming(4),
	OpcodeTiming(24, 12),
	OpcodeTiming(24),
	OpcodeTiming(8),
	OpcodeTiming(16),
	// 0xD0
	OpcodeTiming(20, 8),
	OpcodeTiming(12),
	OpcodeTiming(16, 12),
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(24, 12),
	OpcodeTiming(16),
	OpcodeTiming(8),
	OpcodeTiming(16),
	OpcodeTiming(20, 8),
	OpcodeTiming(16),
	OpcodeTiming(16, 12),
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(24, 12),
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(8),
	OpcodeTiming(16),
	// 0xE0
	OpcodeTiming(12),
	OpcodeTiming(12),
	OpcodeTiming(8),
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(16),
	OpcodeTiming(8),
	OpcodeTiming(16),
	OpcodeTiming(16),
	OpcodeTiming(4),
	OpcodeTiming(16),
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(8),
	OpcodeTiming(16),
	//0xF0
	OpcodeTiming(12),
	OpcodeTiming(12),
	OpcodeTiming(8),
	OpcodeTiming(4),
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(16),
	OpcodeTiming(8),
	OpcodeTiming(16),
	OpcodeTiming(12),
	OpcodeTiming(8),
	OpcodeTiming(16),
	OpcodeTiming(4),
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(-1), // invalid instruction
	OpcodeTiming(8),
	OpcodeTiming(17),
};

void ProcessOpcode(Opcode opcode);
void ProcessOpcodeCB();

void CheckTiming(Opcode opcode, bool conditionalActionTaken)
{
	if (opcode != Opcode::PREFIX_CB)
	{
		OpcodeTiming opTiming = OpcodeTimings[(u8)opcode];

		s8 cycles = conditionalActionTaken ? opTiming.ifTaken : opTiming.ifNotTaken;
		assert(cycles >= 0 && "Negative cycles");

		int expectedInstructions = (cycles) / 4;
		--expectedInstructions; // We effectively spent 4 cycles when we loaded the opcode initially.
		assert(instructions.size() == expectedInstructions && "Unexpected instruction count");
	}
}

void HandleHaltInstructionSideEffects()
{
	// emulate the "PC repeat" that occurs when HALT occurs while interrupts are disabled
	if (bRepeatPCPostHalt)
	{
		bRepeatPCPostHalt = false;
		reg.PC--; // Note if the code contains 2 HALTs in a row this hangs the CPU, which is correct emulation!
	}
}

void HandleIMEFlagChange()
{
	// Enable/Disable the master interrupt enable flag after a 1 opcode delay
	if (interruptEnableDelay > -1)
	{
		if (interruptEnableDelay-- == 0)
		{
			interruptMasterEnable = true;
		}
	}
	if (interruptDisableDelay > -1)
	{
		if (interruptDisableDelay-- == 0)
		{
			interruptMasterEnable = false;
		}
	}
}

void CPU::RaiseInterrupt(INTERRUPT_FLAGS interrupt)
{
	u8 interruptFlagRegister = Bus::LoadU8((u16)SpecialRegister::INTERRUPT_FLAG);
	interruptFlagRegister |= (u8)interrupt;
	Bus::StoreU8((u16)SpecialRegister::INTERRUPT_FLAG, interruptFlagRegister);
}

void HandlePendingInterrupt()
{
	if (interruptMasterEnable)
	{
		u8 interruptFlagRegister = Bus::LoadU8((u16)SpecialRegister::INTERRUPT_FLAG);
		if (interruptFlagRegister & (u8)INTERRUPT_FLAGS::ANY)
		{
			u16 interruptAddress = 0;

			u8 interruptEnableRegister = Bus::LoadU8((u16)SpecialRegister::INTERRUPT_ENABLE);

			// Go in priority order
			for (u8 i = 0; i < 5; i++)
			{
				u8 flagBit = 1 << i;

				if (interruptFlagRegister & flagBit
					&& interruptEnableRegister & flagBit)
				{
					interruptAddress = (u16)AddressRegion::INTERRUPT_VECTOR_START + (0x08 * i);
					interruptFlagRegister = interruptFlagRegister & ~flagBit;
					Bus::StoreU8((u16)SpecialRegister::INTERRUPT_FLAG, interruptFlagRegister);
					break;
				}
			}

			if (interruptAddress)
			{
				interruptMasterEnable = false;
				Bus::StoreU8(--reg.SP, reg.PC_P);
				Bus::StoreU8(--reg.SP, reg.PC_C);
				reg.PC = interruptAddress;

				bHalted = false;
			}
		}
	}
}

void CPU::Step()
{
	if (!bHalted)
	{
		// note : each cpu sub operation takes 4 clock cycles
		if (instructions.empty())
		{
			// note :	some docs suggest the change happens after the next machine cycle, some suggest after the next opcode is executed.
			//			After the next machine cycle means reading the next opcode, but not the operands (if there are any) before handling the interrupt.
			//			I can't reason how this would be safe, so i am taking the "after the next opcode is executed" reading
			//			There's no _logical_ difference between the 2 options, but there is a slight _timing_ difference.
			HandleIMEFlagChange();

			HandlePendingInterrupt();

			// note : read the next opcode from memory, 1 cpu cycle
			Opcode opcode = (Opcode)Bus::LoadU8(reg.PC++);

			HandleHaltInstructionSideEffects();

			ProcessOpcode(opcode);
		}
		else
		{
			// note : execute the next sub operation.
			auto instruction = instructions.front();
			instructions.pop();
			instruction();
		}
	}
};

void ProcessOpcode(Opcode opcode)
{
	//printf("PC = 0x%x, 0x%x\n", reg.PC - 1, (u8)opcode);
	bool conditionalActionTaken = true;
	switch (opcode)
	{
		// Group 0x00
		{
			// 0x00 4
	case Opcode::NOP:
	{
		// note : push nothing, the cpu cycle to read the op code is the delay.
		break;
	}

	// 0x01 12
	case Opcode::LD_BC_NN:
	{
		instructions.push([]() { reg.C = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.B = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x02 8
	case Opcode::LD_$BC_A:
	{
		instructions.push([]() { Bus::StoreU8(reg.BC, reg.A); });
		break;
	}

	// 0x03 8
	case Opcode::INC_BC:
	{
		instructions.push([]() { Math::Inc(reg.BC); });
		break;
	}

	// 0x04 4
	case Opcode::INC_B:
	{
		// note : increment register is free operation.
		Math::Inc(reg.B);
		break;
	}

	// 0x05 4
	case Opcode::DEC_B:
	{
		// note : decrement register is free operation.
		Math::Dec(reg.B);
		break;
	}

	// 0x06 8
	case Opcode::LD_B_N:
	{
		instructions.push([]() { reg.B = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x07 4
	case Opcode::RLCA:
	{
		Math::RotateLeft(reg.A);
		break;
	}

	// 0x08 20
	case Opcode::LD_$NN_SP:
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.temp.H = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { Bus::StoreU8(reg.temp.Full, reg.SP_P); });
		instructions.push([]() { Bus::StoreU8(reg.temp.Full + 1, reg.SP_S); });
		break;
	}

	//0x09 8
	case Opcode::ADD_HL_BC:
	{
		instructions.push([]() { reg.BC = Math::Add(reg.BC, reg.HL); });
		break;
	}

	// 0x0A 8
	case Opcode::LD_A_$BC:
	{
		instructions.push([] { reg.A = Bus::LoadU8(reg.BC); });
		break;
	}

	// 0x0B 8
	case Opcode::DEC_BC:
	{
		instructions.push([]() { Math::Dec(reg.BC); });
		break;
	}

	// 0x0C 4
	case Opcode::INC_C:
	{
		// note : increment register is a free operation.
		Math::Inc(reg.C);
		break;
	}

	// 0x0D 4
	case Opcode::DEC_C:
	{
		Math::Dec(reg.C);
		break;
	}

	// 0x0E 8
	case Opcode::LD_C_N:
	{
		instructions.push([]() { reg.C = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x0F 4
	case Opcode::RRCA:
	{
		Math::RotateRight(reg.A);
		break;
	}
		}

		// Group 0x10
		{
			// 0x10 4
	case Opcode::STOP_0:
	{
		assert(false);		// todo : this still needs implementation. guess the cpu needs some kind of tracked state.
		break;
	}

	// 0x11 12
	case Opcode::LD_DE_NN:
	{
		instructions.push([]() { reg.E = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.D = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x12 8
	case Opcode::LD_$DE_A:
	{
		instructions.push([]() { Bus::StoreU8(reg.DE, reg.A); });
		break;
	}

	// 0x13 8
	case Opcode::INC_DE:
	{
		instructions.push([]() { Math::Inc(reg.DE); });
		break;
	}

	// 0x14 4
	case Opcode::INC_D:
	{
		// note : increment register is a free operation.
		Math::Inc(reg.D);
		break;
	}

	// 0x15 4
	case Opcode::DEC_D:
	{
		// note : decrement register is a free operation.
		Math::Dec(reg.D);
		break;
	}

	// 0x16 8
	case Opcode::LD_D_N:
	{
		instructions.push([]() { reg.D = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x17 4
	case Opcode::RLA:
	{
		Math::RotateLeftThroughCarry(reg.A);
		break;
	}

	// 0x18 12
	case Opcode::JR_N:
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.PC += s8(reg.temp.L); });	// todo : verify signed number casting is performing correct arithmetic
		break;
	}

	// 0x19 8
	case Opcode::ADD_HL_DE:
	{
		instructions.push([]() { reg.HL = Math::Add(reg.HL, reg.DE); });
		break;
	}

	// 0x1A 8
	case Opcode::LD_A_$DE:
	{
		instructions.push([]() { reg.A = Bus::LoadU8(reg.DE); });
		break;
	}

	// 0x01B 8
	case Opcode::DEC_DE:
	{
		instructions.push([]() { Math::Dec(reg.DE); });
		break;
	}

	// 0x1C 4
	case Opcode::INC_E:
	{
		// note : increment register is free operation.
		Math::Inc(reg.E);
		break;
	}

	// 0x1D 4
	case Opcode::DEC_E:
	{
		Math::Dec(reg.E);
		break;
	}

	// 0x1E 8
	case Opcode::LD_E_N:
	{
		instructions.push([]() { reg.E = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x1F 4
	case Opcode::RRA:
	{
		Math::RotateRightThroughCarry(reg.A);
		break;
	}
		}

		// Group 0x20
		{
			// 0x20
	case Opcode::JR_NZ_N:		// 12 / 8
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		conditionalActionTaken = (reg.F & (u8)Flags::Z) == 0;
		if (conditionalActionTaken)
		{
			instructions.push([]() { reg.PC += s8(reg.temp.L); });
		}
		break;
	}

	// 0x21
	case Opcode::LD_HL_NN:		// 12
	{
		instructions.push([]() { reg.L = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.H = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x22
	case Opcode::LD_$HLI_A:
	{
		instructions.push([]() {
			Bus::StoreU8(reg.HL, reg.A);
			Math::Inc(reg.HL);
			});
		break;
	}

	// 0x23
	case Opcode::INC_HL:
	{
		instructions.push([]() { Math::Inc(reg.HL); });
		break;
	}

	// 0x24
	case Opcode::INC_H:			// 4
	{
		// note : incrementing register is a free operation.
		Math::Inc(reg.H);
		break;
	}

	// 0x25
	case Opcode::DEC_H:			// 4
	{
		Math::Dec(reg.H);
		break;
	}

	// 0x26
	case Opcode::LD_H_N:		// 8
	{
		instructions.push([]() { reg.H = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x27
	case Opcode::DAA:			// 4
	{
		assert(false && "Opcode not yet implemented: DAA.");
		break;
	}

	// 0x28
	case Opcode::JR_Z_N:		// 12/8
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		conditionalActionTaken = reg.F & (u8)Flags::Z;
		if (conditionalActionTaken)
		{
			instructions.push([]() { reg.PC += reg.temp.L; });
		}
		break;
	}

	// 0x29
	case Opcode::ADD_HL_HL:		// 8
	{
		instructions.push([]() { reg.HL = Math::Add(reg.HL, reg.HL); });
		break;
	}

	// 0x2A
	case Opcode::LD_A_$HLI:		// 8
	{
		instructions.push([]() { reg.A = Bus::LoadU8(reg.HL++); });
		break;
	}

	// 0x2B
	case Opcode::DEC_HL:		// 8
	{
		instructions.push([]() { Math::Dec(reg.HL); });
	}

	// 0x2C
	case Opcode::INC_L:			// 4
	{
		// note : incrementing register is a free operation.
		Math::Inc(reg.L);
		break;
	}

	// 0x2D
	case Opcode::DEC_L:			// 4
	{
		// note : decrementing register is a free operation.
		Math::Dec(reg.L);
		break;
	}

	// 0x2E
	case Opcode::LD_L_N:		// 8
	{
		instructions.push([]() { reg.L = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x2F
	case Opcode::CPL:			// 4
	{
		reg.A = ~reg.A;

		bool z = reg.F & (u8)Flags::Z;
		bool n = true;
		bool h = true;
		bool c = reg.F & (u8)Flags::C;

		reg.F =
			(z ? (u8)Flags::Z : 0) |
			(n ? (u8)Flags::N : 0) |
			(h ? (u8)Flags::H : 0) |
			(c ? (u8)Flags::C : 0);

		break;
	}
		}

		// Group 0x30
		{
			// 0x30
	case Opcode::JR_NC_N:		// 12/8
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		conditionalActionTaken = (reg.C & (u8)Flags::Z) == 0;
		if (conditionalActionTaken)
		{
			instructions.push([]() { reg.PC += s8(reg.temp.L); });
		}
		break;
	}

	// 0x31
	case Opcode::LD_SP_NN:		// 12
	{
		instructions.push([]() { reg.SP_P = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.SP_S = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x32
	case Opcode::LD_$HLD_A:		// 8
	{
		instructions.push([]() { Bus::StoreU8(reg.HL--, reg.A); });
		break;
	}

	// 0x33
	case Opcode::INC_SP:		// 8
	{
		instructions.push([]() { Math::Inc(reg.SP); });
		break;
	}

	// 0x34
	case Opcode::INC_$HL:		// 12
	{
		instructions.push([]() { reg.temp.H = Bus::LoadU8(reg.HL); });
		instructions.push([]()
			{
				Math::Inc(reg.temp.H);
				Bus::StoreU8(reg.HL, reg.temp.H);
			});
		break;
	}

	// 0x35
	case Opcode::DEC_$HL:		// 12
	{
		instructions.push([]() { reg.temp.H = Bus::LoadU8(reg.HL); });
		instructions.push([]()
			{
				Math::Dec(reg.temp.H);
				Bus::StoreU8(reg.HL, reg.temp.H);
			});
		break;
	}

	// 0x36
	case Opcode::LD_$HL_N:		// 12
	{
		instructions.push([]() { reg.temp.H = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { Bus::StoreU8(reg.HL, reg.temp.H); });
		break;
	}

	// 0x37
	case Opcode::SCF:			// 4
	{
		bool z = reg.F & (u8)Flags::Z;
		bool n = false;
		bool h = false;
		bool c = true;

		reg.F =
			(z ? (u8)Flags::Z : 0) |
			(n ? (u8)Flags::N : 0) |
			(h ? (u8)Flags::H : 0) |
			(c ? (u8)Flags::C : 0);

		break;
	}

	// 0x38
	case Opcode::JR_C_N:		// 12/8
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		conditionalActionTaken = (reg.F & (u8)Flags::N) == 0;
		if (conditionalActionTaken)
		{
			instructions.push([]() { reg.PC += s8(reg.temp.L); });
		}
		break;
	}

	// 0x39
	case Opcode::ADD_HL_SP:		// 8
	{
		instructions.push([]() { reg.HL = Math::Add(reg.HL, reg.SP); });
		break;
	}

	// 0x3A
	case Opcode::LD_A_$HLD:		// 8
	{
		instructions.push([]() { reg.A = Bus::LoadU8(reg.HL--); });		// is this decrement ok?
	}

	// 0x3B
	case Opcode::DEC_SP:		// 8
	{
		instructions.push([]() { Math::Dec(reg.SP); });
	}

	// 0x3C
	case Opcode::INC_A:
	{
		// note : incrementing register is a free operation.
		Math::Inc(reg.A);
		break;
	}

	// 0x3D
	case Opcode::DEC_A:
	{
		// note : decrementing register is a free operation.
		Math::Dec(reg.A);
		break;
	}

	// 0x3E
	case Opcode::LD_A_N:		// 8
	{
		instructions.push([]() { reg.A = Bus::LoadU8(reg.PC++); });
		break;
	}

	// 0x3F
	case Opcode::CCF:			// 4
	{
		bool z = reg.F & (u8)Flags::Z;
		bool n = false;
		bool h = false;
		bool c = !(reg.F & (u8)Flags::C);

		reg.F =
			(z ? (u8)Flags::Z : 0) |
			(n ? (u8)Flags::N : 0) |
			(h ? (u8)Flags::H : 0) |
			(c ? (u8)Flags::C : 0);

		break;
	}
		}

		// Group 0x40
		{
			// 0x40
	case Opcode::LD_B_B:		// 4
	{
		// effectivly a noop.
		break;
	}

	// 0x41
	case Opcode::LD_B_C:		// 4
	{
		reg.B = reg.C;
		break;
	}

	// 0x42
	case Opcode::LD_B_D:		// 4
	{
		reg.B = reg.D;
		break;
	}

	// 0x43
	case Opcode::LD_B_E:		// 4
	{
		reg.B = reg.E;
		break;
	}

	// 0x44
	case Opcode::LD_B_H:		// 4
	{
		reg.B = reg.H;
		break;
	}

	// 0x45
	case Opcode::LD_B_L:		// 4
	{
		reg.B = reg.L;
		break;
	}

	// 0x46
	case Opcode::LD_B_$HL:		// 8
	{
		instructions.push([]() { reg.B = Bus::LoadU8(reg.HL); });
		break;
	}

	// 0x47
	case Opcode::LD_B_A:		// 4
	{
		reg.B = reg.A;
		break;
	}

	// 0x48
	case Opcode::LD_C_B:		// 4
	{
		reg.C = reg.B;
		break;
	}

	// 0x49
	case Opcode::LD_C_C:		// 4
	{
		break;
	}

	// 0x4A
	case Opcode::LD_C_D:		// 4
	{
		reg.C = reg.D;
		break;
	}

	// 0x4B
	case Opcode::LD_C_E:		// 4
	{
		reg.C = reg.E;
		break;
	}

	// 0x4C
	case Opcode::LD_C_H:		// 4
	{
		reg.C = reg.H;
		break;
	}

	// 0x4D
	case Opcode::LD_C_L:		// 4
	{
		reg.C = reg.L;
		break;
	}

	// 0x4E
	case Opcode::LD_C_$HL:		// 8
	{
		instructions.push([]() { reg.C = Bus::LoadU8(reg.HL); });
		break;
	}

	// 0x4F
	case Opcode::LD_C_A:		// 4
	{
		reg.C = reg.A;
		break;
	}
		}

		// Group 0x50
		{
			// 0x50
	case Opcode::LD_D_B:		// 4
	{
		reg.D = reg.B;
		break;
	}

	// 0x51
	case Opcode::LD_D_C:		// 4
	{
		reg.D = reg.C;
		break;
	}

	// 0x52
	case Opcode::LD_D_D:		// 4
	{
		break;
	}

	// 0x53
	case Opcode::LD_D_E:		// 4
	{
		reg.D = reg.E;
		break;
	}

	// 0x54
	case Opcode::LD_D_H:		// 4
	{
		reg.D = reg.H;
		break;
	}

	// 0x55
	case Opcode::LD_D_L:		// 4
	{
		reg.D = reg.L;
		break;
	}

	// 0x56
	case Opcode::LD_D_$HL:		// 8
	{
		instructions.push([]() { reg.D = Bus::LoadU8(reg.HL); });
		break;
	}

	// 0x57
	case Opcode::LD_D_A:		// 4
	{
		reg.D = reg.A;
		break;
	}

	// 0x58
	case Opcode::LD_E_B:		// 4
	{
		reg.E = reg.B;
		break;
	}

	// 0x59
	case Opcode::LD_E_C:		// 4
	{
		reg.E = reg.C;
		break;
	}

	// 0x5A
	case Opcode::LD_E_D:		// 4
	{
		reg.E = reg.D;
		break;
	}

	// 0x5B
	case Opcode::LD_E_E:		// 4
	{
		break;
	}

	// 0x5C
	case Opcode::LD_E_H:		// 4
	{
		reg.E = reg.H;
		break;
	}

	// 0x5D
	case Opcode::LD_E_L:		// 4
	{
		reg.E = reg.L;
		break;
	}

	// 0x5E
	case Opcode::LD_E_$HL:		// 8
	{
		instructions.push([]() { reg.E = Bus::LoadU8(reg.HL); });
		break;
	}

	// 0x5F
	case Opcode::LD_E_A:		// 4
	{
		reg.E = reg.A;
		break;
	}
		}

		// Group 0x60
		{
			// 0x60
	case Opcode::LD_H_B:		// 4
	{
		reg.H = reg.B;
		break;
	}

	// 0x61
	case Opcode::LD_H_C:		// 4
	{
		reg.H = reg.C;
		break;
	}

	// 0x62
	case Opcode::LD_H_D:		// 4
	{
		reg.H = reg.D;
		break;
	}

	// 0x63
	case Opcode::LD_H_E:		// 4
	{
		reg.H = reg.E;
		break;
	}

	// 0x64
	case Opcode::LD_H_H:		// 4
	{
		break;
	}

	// 0x65
	case Opcode::LD_H_L:		// 4
	{
		reg.H = reg.L;
		break;
	}

	// 0x66
	case Opcode::LD_H_$HL:		// 8
	{
		instructions.push([]() { reg.H = Bus::LoadU8(reg.HL); });
		break;
	}

	// 0x67
	case Opcode::LD_H_A:		// 4
	{
		reg.H = reg.A;
		break;
	}

	// 0x68
	case Opcode::LD_L_B:		// 4
	{
		reg.L = reg.B;
		break;
	}

	// 0x69
	case Opcode::LD_L_C:		// 4
	{
		reg.L = reg.C;
		break;
	}

	// 0x6A
	case Opcode::LD_L_D:		// 4
	{
		reg.L = reg.D;
		break;
	}

	// 0x6B
	case Opcode::LD_L_E:		// 4
	{
		reg.L = reg.E;
		break;
	}

	// 0x6C
	case Opcode::LD_L_H:		// 4
	{
		reg.L = reg.H;
		break;
	}

	// 0x6D
	case Opcode::LD_L_L:		// 4
	{
		break;
	}

	// 0x6E
	case Opcode::LD_L_$HL:		// 8
	{
		instructions.push([]() { reg.L = Bus::LoadU8(reg.HL); });
		break;
	}

	// 0x6F
	case Opcode::LD_L_A:		// 4
	{
		reg.L = reg.A;
		break;
	}
		}

		// Group 0x70
		{
			// 0x70
	case Opcode::LD_$HL_B:		// 8
	{
		instructions.push([]() { Bus::StoreU8(reg.HL, reg.B); });
		break;
	}

	// 0x71
	case Opcode::LD_$HL_C:		// 8
	{
		instructions.push([]() { Bus::StoreU8(reg.HL, reg.C); });
		break;
	}

	// 0x72
	case Opcode::LD_$HL_D:		// 8
	{
		instructions.push([]() { Bus::StoreU8(reg.HL, reg.D); });
		break;
	}

	// 0x73
	case Opcode::LD_$HL_E:		// 8
	{
		instructions.push([]() { Bus::StoreU8(reg.HL, reg.E); });
		break;
	}

	// 0x74
	case Opcode::LD_$HL_H:		// 8
	{
		instructions.push([]() { Bus::StoreU8(reg.HL, reg.H); });
		break;
	}

	// 0x75
	case Opcode::LD_$HL_L:		// 8
	{
		instructions.push([]() { Bus::StoreU8(reg.HL, reg.L); });
		break;
	}

	// 0x76
	case Opcode::HALT:			// 4
	{
		if (interruptMasterEnable)
		{
			bHalted = true;
		}
		else
		{
			// todo if (GB/SGB/GBP)
			bRepeatPCPostHalt = true;
		}
		break;
	}

	// 0x77
	case Opcode::LD_$HL_A:		// 8
	{
		instructions.push([]() { Bus::StoreU8(reg.HL, reg.A); });
		break;
	}

	// 0x78
	case Opcode::LD_A_B:		// 4
	{
		reg.A = reg.B;
		break;
	}

	// 0x79
	case Opcode::LD_A_C:		// 4
	{
		reg.A = reg.C;
		break;
	}

	// 0x7A
	case Opcode::LD_A_D:		// 4
	{
		reg.A = reg.D;
		break;
	}

	// 0x7B
	case Opcode::LD_A_E:		// 4
	{
		reg.A = reg.E;
		break;
	}

	// 0x7C
	case Opcode::LD_A_H:		// 4
	{
		reg.A = reg.H;
		break;
	}

	// 0x7D
	case Opcode::LD_A_L:		// 4
	{
		reg.A = reg.L;
		break;
	}

	// 0x7E
	case Opcode::LD_A_$HL:		// 8
	{
		instructions.push([]() { reg.A = Bus::LoadU8(reg.HL); });
		break;
	}

	// 0x7F
	case Opcode::LD_A_A:		// 4
	{
		// note : move between registers is a free operation.
		reg.A = reg.A;
		break;
	}
		}

		// Group 0x80
		{
			// 0x80
	case Opcode::ADD_A_B:		// 4
	{
		reg.A = Math::Add(reg.A, reg.B);
		break;
	}

	// 0x81
	case Opcode::ADD_A_C:		// 4
	{
		reg.A = Math::Add(reg.A, reg.C);
		break;
	}

	// 0x82
	case Opcode::ADD_A_D:		// 4
	{
		reg.A = Math::Add(reg.A, reg.D);
		break;
	}

	// 0x83
	case Opcode::ADD_A_E:		// 4
	{
		reg.A = Math::Add(reg.A, reg.E);
		break;
	}

	// 0x84
	case Opcode::ADD_A_H:		// 4
	{
		reg.A = Math::Add(reg.A, reg.H);
		break;
	}

	// 0x85
	case Opcode::ADD_A_L:		// 4
	{
		reg.A = Math::Add(reg.A, reg.L);
		break;
	}

	// 0x86
	case Opcode::ADD_A_$HL:		// 8
	{
		instructions.push([]() { reg.A = Math::Add(reg.A, Bus::LoadU8(reg.HL)); });
		break;
	}

	// 0x87
	case Opcode::ADD_A_A:		// 4
	{
		reg.A = Math::Add(reg.A, reg.A);
		break;
	}

	// 0x88
	case Opcode::ADC_A_B:		// 4
	{
		reg.A = Math::AddWithCarry(reg.A, reg.B);
		break;
	}

	// 0x89
	case Opcode::ADC_A_C:		// 4
	{
		reg.A = Math::AddWithCarry(reg.A, reg.C);
		break;
	}

	// 0x8A
	case Opcode::ADC_A_D:		// 4
	{
		reg.A = Math::AddWithCarry(reg.A, reg.D);
		break;
	}

	// 0x8B
	case Opcode::ADC_A_E:		// 4
	{
		reg.A = Math::AddWithCarry(reg.A, reg.E);
		break;
	}

	// 0x8C
	case Opcode::ADC_A_H:		// 4
	{
		reg.A = Math::AddWithCarry(reg.A, reg.H);
		break;
	}

	// 0x8D
	case Opcode::ADC_A_L:		// 4
	{
		reg.A = Math::AddWithCarry(reg.A, reg.L);
		break;
	}

	// 0x88
	case Opcode::ADC_A_$HL:		// 8
	{
		instructions.push([]() { reg.A = Math::AddWithCarry(reg.A, Bus::LoadU8(reg.HL)); });
		break;
	}

	// 0x8F
	case Opcode::ADC_A_A:		// 4
	{
		reg.A = Math::AddWithCarry(reg.A, reg.A);
		break;
	}
		}

		// Group 0x90
		{
			// 0x090
	case Opcode::SUB_B:			// 4
	{
		Math::SubFromA(reg.B);
		break;
	}

	// 0x091
	case Opcode::SUB_C:			// 4
	{
		Math::SubFromA(reg.C);
		break;
	}

	// 0x092
	case Opcode::SUB_D:			// 4
	{
		Math::SubFromA(reg.D);
		break;
	}

	// 0x093
	case Opcode::SUB_E:			// 4
	{
		Math::SubFromA(reg.E);
		break;
	}

	// 0x094
	case Opcode::SUB_H:			// 4
	{
		Math::SubFromA(reg.H);
		break;
	}

	// 0x095
	case Opcode::SUB_L:			// 4
	{
		Math::SubFromA(reg.L);
		break;
	}

	// 0x096
	case Opcode::SUB_$HL:		// 8
	{
		instructions.push([]() { Math::SubFromA(Bus::LoadU8(reg.HL)); });
		break;
	}

	// 0x097
	case Opcode::SUB_A:			// 4
	{
		Math::SubFromA(reg.A);
		break;
	}

	// 0x098
	case Opcode::SBC_A_B:		// 4
	{
		Math::SubWithCarryFromA(reg.B);
		break;
	}

	// 0x099
	case Opcode::SBC_A_C:		// 4
	{
		Math::SubWithCarryFromA(reg.C);
		break;
	}

	// 0x09A
	case Opcode::SBC_A_D:		// 4
	{
		Math::SubWithCarryFromA(reg.D);
		break;
	}

	// 0x09B
	case Opcode::SBC_A_E:		// 4
	{
		Math::SubWithCarryFromA(reg.E);
		break;
	}

	// 0x09C
	case Opcode::SBC_A_H:		// 4
	{
		Math::SubWithCarryFromA(reg.H);
		break;
	}

	// 0x09D
	case Opcode::SBC_A_L:		// 4
	{
		Math::SubWithCarryFromA(reg.L);
		break;
	}

	// 0x098
	case Opcode::SBC_A_$HL:		// 8
	{
		instructions.push([]() { Math::SubWithCarryFromA(Bus::LoadU8(reg.HL)); });
		break;
	}

	// 0x09F
	case Opcode::SBC_A_A:		// 4
	{
		Math::SubWithCarryFromA(reg.A);
		break;
	}
		}

		// Group 0xA0
		{
			// 0xA0
	case Opcode::AND_B:			// 4
	{
		reg.A = Math::And(reg.A, reg.B);
		break;
	}

	// 0xA1
	case Opcode::AND_C:			// 4
	{
		reg.A = Math::And(reg.A, reg.C);
		break;
	}

	// 0xA2
	case Opcode::AND_D:			// 4
	{
		reg.A = Math::And(reg.A, reg.D);
		break;
	}

	// 0xA3
	case Opcode::AND_E:			// 4
	{
		reg.A = Math::And(reg.A, reg.E);
		break;
	}

	// 0xA4
	case Opcode::AND_H:			// 4
	{
		reg.A = Math::And(reg.A, reg.H);
		break;
	}

	// 0xA5
	case Opcode::AND_L:			// 4
	{
		reg.A = Math::And(reg.A, reg.L);
		break;
	}

	// 0xA0
	case Opcode::AND_$HL:		// 8
	{
		instructions.push([]() { reg.A = Math::And(reg.A, Bus::LoadU8(reg.HL)); });
		break;
	}

	// 0xA7
	case Opcode::AND_A:			// 4
	{
		reg.A = Math::And(reg.A, reg.A);
		break;
	}

	// 0xA8
	case Opcode::XOR_B:			// 4
	{
		Math::Xor(reg.B);
		break;
	}

	// 0xA9
	case Opcode::XOR_C:			// 4
	{
		Math::Xor(reg.C);
		break;
	}

	// 0xAA
	case Opcode::XOR_D:			// 4
	{
		Math::Xor(reg.D);
		break;
	}

	// 0xAB
	case Opcode::XOR_E:			// 4
	{
		Math::Xor(reg.E);
		break;
	}

	// 0xAC
	case Opcode::XOR_H:			// 4
	{
		Math::Xor(reg.H);
		break;
	}

	// 0xAD
	case Opcode::XOR_L:			// 4
	{
		Math::Xor(reg.L);
		break;
	}

	// 0xAE
	case Opcode::XOR_$HL:		// 8
	{
		instructions.push([]() { Math::Xor(Bus::LoadU8(reg.HL)); });
		break;
	}

	// 0xAF
	case Opcode::XOR_A:			// 4
	{
		Math::Xor(reg.A);
		break;
	}
		}

		// Group 0xB0
		{
			// 0xB0
	case Opcode::OR_B:			// 4
	{
		reg.A = Math::Or(reg.A, reg.B);
		break;
	}

	// 0xB1
	case Opcode::OR_C:			// 4
	{
		reg.A = Math::Or(reg.A, reg.C);
		break;
	}

	// 0xB2
	case Opcode::OR_D:			// 4
	{
		reg.A = Math::Or(reg.A, reg.D);
		break;
	}

	// 0xB3
	case Opcode::OR_E:			// 4
	{
		reg.A = Math::Or(reg.A, reg.E);
		break;
	}

	// 0xB4
	case Opcode::OR_H:			// 4
	{
		reg.A = Math::Or(reg.A, reg.H);
		break;
	}

	// 0xB5
	case Opcode::OR_L:			// 4
	{
		reg.A = Math::Or(reg.A, reg.L);
		break;
	}

	// 0xB6
	case Opcode::OR_$HL:		// 8
	{
		instructions.push([]() { reg.A = Math::Or(reg.A, Bus::LoadU8(reg.HL)); });
		break;
	}

	// 0xB7
	case Opcode::OR_A:			// 4
	{
		reg.A = Math::Or(reg.A, reg.A);
		break;
	}

	// 0xB8
	case Opcode::CP_B:			// 4
	{
		Math::Compare(reg.B);
		break;
	}

	// 0xB9
	case Opcode::CP_C:			// 4
	{
		Math::Compare(reg.C);
		break;
	}

	// 0xBA
	case Opcode::CP_D:			// 4
	{
		Math::Compare(reg.D);
		break;
	}

	// 0xBB
	case Opcode::CP_E:			// 4
	{
		Math::Compare(reg.E);
		break;
	}

	// 0xBC
	case Opcode::CP_H:			// 4
	{
		Math::Compare(reg.H);
		break;
	}

	// 0xBD
	case Opcode::CP_L:			// 4
	{
		Math::Compare(reg.L);
		break;
	}

	// 0xBE
	case Opcode::CP_$HL:		// 8
	{
		instructions.push([]() { Math::Compare(Bus::LoadU8(reg.HL)); });
		break;
	}

	// 0xBF
	case Opcode::CP_A:			// 4
	{
		Math::Compare(reg.A);
		break;
	}
		}

		// 0xC0
		{
	case Opcode::POP_BC:		// 12
	{
		instructions.push([]() { reg.C = Bus::LoadU8(reg.SP++); });
		instructions.push([]() { reg.B = Bus::LoadU8(reg.SP++); });
		break;
	}
	case Opcode::JP_NN:			// 16
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.temp.H = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.PC = reg.temp.Full; }); // todo(luke) : this should be conditional, should skip if condition is not met and dont advance timer
		break;
	}
	case Opcode::PUSH_BC:		// 16
	{
		instructions.push([]() { Bus::StoreU8(--reg.SP, reg.B); });
		instructions.push([]() { Bus::StoreU8(--reg.SP, reg.C); });
		instructions.push([]() { /* Do nothing */});
		break;
	}
	case Opcode::RET:			// 16
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.SP++); });
		instructions.push([]() { reg.temp.H = Bus::LoadU8(reg.SP++); });
		instructions.push([]() { reg.PC = reg.temp.Full; });
		break;
	}
	case Opcode::PREFIX_CB:		// 4
	{
		// note : evaluate the extended opcode table.
		instructions.push([]() { ProcessOpcodeCB(); });
		break;
	}
	case Opcode::CALL_NN:		// 24
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.temp.H = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { Bus::StoreU8(--reg.SP, reg.PC_P); });
		instructions.push([]() { Bus::StoreU8(--reg.SP, reg.PC_C); });
		instructions.push([]() { reg.PC = reg.temp.Full; });
		break;
	}
		}
		// 0xD0
		{
	case Opcode::RETI:			// 16
	{
		instructions.push([]() { reg.PC_C = Bus::LoadU8(reg.SP++); });
		instructions.push([]() { reg.PC_P = Bus::LoadU8(reg.SP++); });
		instructions.push([]() { interruptMasterEnable = true; });
		break;
	}
		}
		// 0xE0
		{
	case Opcode::LDH_$N_A:		// 12
	{
		instructions.push([]() { reg.temp.Full = 0xFF00 + Bus::LoadU8(reg.PC++); });
		instructions.push([]() { Bus::StoreU8(reg.temp.Full, reg.A); });
		break;
	}
	case Opcode::LD_$C_A:		// 8
	{
		instructions.push([]() { Bus::StoreU8(0xFF00 + reg.C, reg.A); });
		break;
	}
		}
	case Opcode::LD_$NN_A:
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.temp.H = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { Bus::StoreU8(reg.temp.Full, reg.A); });
		break;
	}
	// 0xF0
	{
	case Opcode::LDH_A_$N:
	{
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.A = Bus::LoadU8(0xFF00 + reg.temp.L); });
		break;
	}
	case Opcode::DI:			// 4
	{
		// This disables interrupts, but only after the next instruction is complete
		interruptDisableDelay = 1;
		break;
	}
	case Opcode::LD_A_$NN:		// 18
	{
		instructions.push([]() { reg.temp.H = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.temp.L = Bus::LoadU8(reg.PC++); });
		instructions.push([]() { reg.A = Bus::LoadU8(reg.temp.Full); });
		break;
	}
	case Opcode::EI:			// 4
	{
		// This enables interrupts, but only after the next instruction is complete
		interruptEnableDelay = 1;
		break;
	}
	case Opcode::CP_N:			// 8
	{
		instructions.push([]() { Math::Compare(Bus::LoadU8(reg.PC++)); });
		break;
	}
	}
	default:
		assert(false && "Unknown opcode");
		break;
	}
	CheckTiming(opcode, conditionalActionTaken);
}

enum class Opcode_CB : u8
{
	RLC_B = 0x00,
	RLC_C = 0x01,
	RLC_D = 0x02,
	RLC_E = 0x03,
	RLC_H = 0x04,
	RLC_L = 0x05,
	RLC_$HL = 0x06,
	RLC_A = 0x07,
	RRC_B = 0x08,
	RRC_C = 0x09,
	RRC_D = 0x0A,
	RRC_E = 0x0B,
	RRC_H = 0x0C,
	RRC_L = 0x0D,
	RRC_$HL = 0x0E,
	RRC_A = 0x0F,

	RL_B = 0x10,
	RL_C = 0x11,
	RL_D = 0x12,
	RL_E = 0x13,
	RL_H = 0x14,
	RL_L = 0x15,
	RL_$HL = 0x16,
	RL_A = 0x17,
	RR_B = 0x18,
	RR_C = 0x19,
	RR_D = 0x1A,
	RR_E = 0x1B,
	RR_H = 0x1C,
	RR_L = 0x1D,
	RR_$HL = 0x1E,
	RR_A = 0x1F,

	SLA_B = 0x20,
	SLA_C = 0x21,
	SLA_D = 0x22,
	SLA_E = 0x23,
	SLA_H = 0x24,
	SLA_L = 0x25,
	SLA_$HL = 0x26,
	SLA_A = 0x27,
	SRA_B = 0x28,
	SRA_C = 0x29,
	SRA_D = 0x2A,
	SRA_E = 0x2B,
	SRA_H = 0x2C,
	SRA_L = 0x2D,
	SRA_$HL = 0x2E,
	SRA_A = 0x2F,

	SWAP_B = 0x30,
	SWAP_C = 0x31,
	SWAP_D = 0x32,
	SWAP_E = 0x33,
	SWAP_H = 0x34,
	SWAP_L = 0x35,
	SWAP_$HL = 0x36,
	SWAP_A = 0x37,
	SRL_B = 0x38,
	SRL_C = 0x39,
	SRL_D = 0x3A,
	SRL_E = 0x3B,
	SRL_H = 0x3C,
	SRL_L = 0x3D,
	SRL_$HL = 0x3E,
	SRL_A = 0x3F,

	BIT_0_B = 0x40,
	BIT_0_C = 0x41,
	BIT_0_D = 0x42,
	BIT_0_E = 0x43,
	BIT_0_H = 0x44,
	BIT_0_L = 0x45,
	BIT_0_$HL = 0x46,
	BIT_0_A = 0x47,
	BIT_1_B = 0x48,
	BIT_1_C = 0x49,
	BIT_1_D = 0x4A,
	BIT_1_E = 0x4B,
	BIT_1_H = 0x4C,
	BIT_1_L = 0x4D,
	BIT_1_$HL = 0x4E,
	BIT_1_A = 0x4F,

	BIT_2_B = 0x50,
	BIT_2_C = 0x51,
	BIT_2_D = 0x52,
	BIT_2_E = 0x53,
	BIT_2_H = 0x54,
	BIT_2_L = 0x55,
	BIT_2_$HL = 0x56,
	BIT_2_A = 0x57,
	BIT_3_B = 0x58,
	BIT_3_C = 0x59,
	BIT_3_D = 0x5A,
	BIT_3_E = 0x5B,
	BIT_3_H = 0x5C,
	BIT_3_L = 0x5D,
	BIT_3_$HL = 0x5E,
	BIT_3_A = 0x5F,

	BIT_4_B = 0x60,
	BIT_4_C = 0x61,
	BIT_4_D = 0x62,
	BIT_4_E = 0x63,
	BIT_4_H = 0x64,
	BIT_4_L = 0x65,
	BIT_4_$HL = 0x66,
	BIT_4_A = 0x67,
	BIT_5_B = 0x68,
	BIT_5_C = 0x69,
	BIT_5_D = 0x6A,
	BIT_5_E = 0x6B,
	BIT_5_H = 0x6C,
	BIT_5_L = 0x6D,
	BIT_5_$HL = 0x6E,
	BIT_5_A = 0x6F,

	BIT_6_B = 0x70,
	BIT_6_C = 0x71,
	BIT_6_D = 0x72,
	BIT_6_E = 0x73,
	BIT_6_H = 0x74,
	BIT_6_L = 0x75,
	BIT_6_$HL = 0x76,
	BIT_6_A = 0x77,
	BIT_7_B = 0x78,
	BIT_7_C = 0x79,
	BIT_7_D = 0x7A,
	BIT_7_E = 0x7B,
	BIT_7_H = 0x7C,
	BIT_7_L = 0x7D,
	BIT_7_$HL = 0x7E,
	BIT_7_A = 0x7F,

	RES_0_B = 0x80,
	RES_0_C = 0x81,
	RES_0_D = 0x82,
	RES_0_E = 0x83,
	RES_0_H = 0x84,
	RES_0_L = 0x85,
	RES_0_$HL = 0x86,
	RES_0_A = 0x87,
	RES_1_B = 0x88,
	RES_1_C = 0x89,
	RES_1_D = 0x8A,
	RES_1_E = 0x8B,
	RES_1_H = 0x8C,
	RES_1_L = 0x8D,
	RES_1_$HL = 0x8E,
	RES_1_A = 0x8F,

	RES_2_B = 0x90,
	RES_2_C = 0x91,
	RES_2_D = 0x92,
	RES_2_E = 0x93,
	RES_2_H = 0x94,
	RES_2_L = 0x95,
	RES_2_$HL = 0x96,
	RES_2_A = 0x97,
	RES_3_B = 0x98,
	RES_3_C = 0x99,
	RES_3_D = 0x9A,
	RES_3_E = 0x9B,
	RES_3_H = 0x9C,
	RES_3_L = 0x9D,
	RES_3_$HL = 0x9E,
	RES_3_A = 0x9F,

	RES_4_B = 0xA0,
	RES_4_C = 0xA1,
	RES_4_D = 0xA2,
	RES_4_E = 0xA3,
	RES_4_H = 0xA4,
	RES_4_L = 0xA5,
	RES_4_$HL = 0xA6,
	RES_4_A = 0xA7,
	RES_5_B = 0xA8,
	RES_5_C = 0xA9,
	RES_5_D = 0xAA,
	RES_5_E = 0xAB,
	RES_5_H = 0xAC,
	RES_5_L = 0xAD,
	RES_5_$HL = 0xAE,
	RES_5_A = 0xAF,

	RES_6_B = 0xB0,
	RES_6_C = 0xB1,
	RES_6_D = 0xB2,
	RES_6_E = 0xB3,
	RES_6_H = 0xB4,
	RES_6_L = 0xB5,
	RES_6_$HL = 0xB6,
	RES_6_A = 0xB7,
	RES_7_B = 0xB8,
	RES_7_C = 0xB9,
	RES_7_D = 0xBA,
	RES_7_E = 0xBB,
	RES_7_H = 0xBC,
	RES_7_L = 0xBD,
	RES_7_$HL = 0xBE,
	RES_7_A = 0xBF,

	SET_0_B = 0xC0,
	SET_0_C = 0xC1,
	SET_0_D = 0xC2,
	SET_0_E = 0xC3,
	SET_0_H = 0xC4,
	SET_0_L = 0xC5,
	SET_0_$HL = 0xC6,
	SET_0_A = 0xC7,
	SET_1_B = 0xC8,
	SET_1_C = 0xC9,
	SET_1_D = 0xCA,
	SET_1_E = 0xCB,
	SET_1_H = 0xCC,
	SET_1_L = 0xCD,
	SET_1_$HL = 0xCE,
	SET_1_A = 0xCF,

	SET_2_B = 0xD0,
	SET_2_C = 0xD1,
	SET_2_D = 0xD2,
	SET_2_E = 0xD3,
	SET_2_H = 0xD4,
	SET_2_L = 0xD5,
	SET_2_$HL = 0xD6,
	SET_2_A = 0xD7,
	SET_3_B = 0xD8,
	SET_3_C = 0xD9,
	SET_3_D = 0xDA,
	SET_3_E = 0xDB,
	SET_3_H = 0xDC,
	SET_3_L = 0xDD,
	SET_3_$HL = 0xDE,
	SET_3_A = 0xDF,

	SET_4_B = 0xE0,
	SET_4_C = 0xE1,
	SET_4_D = 0xE2,
	SET_4_E = 0xE3,
	SET_4_H = 0xE4,
	SET_4_L = 0xE5,
	SET_4_$HL = 0xE6,
	SET_4_A = 0xE7,
	SET_5_B = 0xE8,
	SET_5_C = 0xE9,
	SET_5_D = 0xEA,
	SET_5_E = 0xEB,
	SET_5_H = 0xEC,
	SET_5_L = 0xED,
	SET_5_$HL = 0xEE,
	SET_5_A = 0xEF,

	SET_6_B = 0xF0,
	SET_6_C = 0xF1,
	SET_6_D = 0xF2,
	SET_6_E = 0xF3,
	SET_6_H = 0xF4,
	SET_6_L = 0xF5,
	SET_6_$HL = 0xF6,
	SET_6_A = 0xF7,
	SET_7_B = 0xF8,
	SET_7_C = 0xF9,
	SET_7_D = 0xFA,
	SET_7_E = 0xFB,
	SET_7_H = 0xFC,
	SET_7_L = 0xFD,
	SET_7_$HL = 0xFE,
	SET_7_A = 0xFF,
};

void CheckTimingCB(Opcode_CB opcode_cb)
{
	u8 opcodeVal = (u8)opcode_cb & 0x07; // Mask off all but the bottom 3 bits
	u8 expectedUops = opcodeVal == 0x06 ? 3 : 1;
	--expectedUops; // We effectively spent 4 cycles when we loaded the opcode initially.

	assert(instructions.size() == expectedUops && "Unexpected instruction count");
}

void ProcessOpcodeCB()
{
	Opcode_CB opcode_cb = (Opcode_CB)Bus::LoadU8(reg.PC);
	//printf("PC = 0x%x, 0x%x\n", reg.PC, (u8)opcode_cb);
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
		Math::RotateLeftThroughCarry_CB(reg.B);
		break;
	}
	case Opcode_CB::RL_C:
	{
		Math::RotateLeftThroughCarry_CB(reg.C);
		break;
	}
	case Opcode_CB::RL_D:
	{
		Math::RotateLeftThroughCarry_CB(reg.D);
		break;
	}
	case Opcode_CB::RL_E:
	{
		Math::RotateLeftThroughCarry_CB(reg.E);
		break;
	}
	case Opcode_CB::RL_H:
	{
		Math::RotateLeftThroughCarry_CB(reg.H);
		break;
	}
	case Opcode_CB::RL_L:
	{
		Math::RotateLeftThroughCarry_CB(reg.L);
		break;
	}
	case Opcode_CB::RL_$HL:
	{
		instructions.push([]()
			{
				u8 val = Bus::LoadU8(reg.HL);
				Math::RotateLeftThroughCarry_CB(val);
				reg.temp.L = val;
			});
		instructions.push([]() { Bus::StoreU8(reg.HL, reg.temp.L); });
		break;
	}
	case Opcode_CB::RL_A:
	{
		Math::RotateLeftThroughCarry_CB(reg.A);
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
	CheckTimingCB(opcode_cb);
}




class A
{
public:
	static const std::size_t size = 8;
	static u8 Get() { return reg.A; }
	static void Set(u8 value) { reg.A = value; }
};

class F
{
public:
	static const std::size_t size = 8;
	static u8 Get() { return reg.F; }
	static void Set(u8 value) { reg.F = value; }
};

class B
{
public:
	static const std::size_t size = 8;
	static u8 Get() { return reg.B; }
	static void Set(u8 value) { reg.B = value; }
};

class C
{
public:
	static const std::size_t size = 8;
	static u8 Get() { return reg.C; }
	static u16 GetAddr() { return 0xFF00 + reg.C; }
	static void Set(u8 value) { reg.C = value; }
	static bool IsMet() { return reg.F & (u8)Flags::C; }
};

class D
{
public:
	static const std::size_t size = 8;
	static u8 Get() { return reg.D; }
	static void Set(u8 value) { reg.D = value; }
};

class E
{
public:
	static const std::size_t size = 8;
	static u8 Get() { return reg.E; }
	static void Set(u8 value) { reg.E = value; }
};

class H
{
public:
	static const std::size_t size = 8;
	static u8 Get() { return reg.H; }
	static void Set(u8 value) { reg.H = value; }
	static bool IsMet() { return reg.F & (u8)Flags::H; }
};

class L
{
public:
	static const std::size_t size = 8;
	static u8 Get() { return reg.L; }
	static void Set(u8 value) { reg.L = value; }
};


class AF {
public:
	static const std::size_t size = 16;
	static u16 Get() { return reg.AF; }
	static void Set(u16 v) { reg.AF = v; }
};

class BC {
public:
	static const std::size_t size = 16;
	static u16 Get() { return reg.BC; }
	static u16 GetAddr() { return Get(); }
	static void Set(u16 v) { reg.BC = v; }
};

class DE {
public:
	static const std::size_t size = 16;
	static u16 Get() { return reg.DE; }
	static u16 GetAddr() { return Get(); }
	static void Set(u16 v) { reg.DE = v; }
};

class HL {
public:
	static const std::size_t size = 16;
	static u16 Get() { return reg.HL; }
	static u16 GetAddr() { return Get(); }
	static void Set(u16 v) { reg.HL = v; }
};

class SP {
public:
	static const std::size_t size = 16;
	static u16 Get() { return reg.SP; }
	static void Set(u16 v) { reg.SP = v; }
};

class PC {
public:
	static const std::size_t size = 16;
	static u16 Get() { return reg.PC; }
	static void Set(u16 v) { reg.PC = v; }
};


class d8
{
public:
	static const std::size_t size = 8;
	static u8 Get()
	{
		return Bus::LoadU8(reg.PC++);
	}
};

class d16
{
public:
	static const std::size_t size = 16;
	static u16 Get()
	{
		u16split v;
		v.L = Bus::LoadU8(reg.PC++);
		v.H = Bus::LoadU8(reg.PC++);
		return v.Full;
	}
};

class a8 : public d8
{
public:
	static u16 GetAddr() { return u16(0xFF00) + (u16)Get(); }
};

class a16 : public d16
{
public:
	static u16 GetAddr() { return Get(); }
};

class r8
{
public:
	static const std::size_t size = 8;
	static s8 Get()
	{
		u8 v = Bus::LoadU8(reg.PC++);
		return *reinterpret_cast<s8*>(&v);
	}
};


class Z
{
public:
	static bool IsMet()
	{
		return reg.F & (u8)Flags::Z;
	}
};

class NZ
{
public:
	static bool IsMet()
	{
		return (reg.F & (u8)Flags::Z) == false;
	}
};

class NC
{
public:
	static bool IsMet()
	{
		return (reg.F & (u8)Flags::C) == false;
	}
};


template <class SRC>
class UREF
{
public:
	static u8 Get()
	{
		u16 addr = SRC::GetAddr();
		return Bus::LoadU8(addr);
	}

	static void Set(u8 value)
	{
		u16 addr = SRC::GetAddr();
		Bus::StoreU8(addr, value);
	}

	static void Set(u16 value)
	{
		u16split split;
		split.Full = value;

		u16 addr = SRC::GetAddr();
		Bus::StoreU8(addr++, split.L); // todo(luke) : is this the correct order L then H?
		Bus::StoreU8(addr, split.H);
	}
};

template <class SRC>
class IREF
{
public:
	static u8 Get()
	{
		u16 addr = SRC::GetAddr();
		SRC::Set(addr + 1);
		return Bus::LoadU8(addr);
	}

	static void Set(u8 value)
	{
		u16 addr = SRC::GetAddr();
		SRC::Set(addr + 1);
		Bus::StoreU8(addr, value);
	}
};

template <class SRC>
class DREF
{
public:
	static u8 Get()
	{
		u16 addr = SRC::GetAddr();
		SRC::Set(addr - 1);
		return Bus::LoadU8(addr);
	}

	static void Set(u8 value)
	{
		u16 addr = SRC::GetAddr();
		SRC::Set(addr - 1);
		Bus::StoreU8(addr, value);
	}
};



void SetFlags(int Z, int N, int H, int C) // todo(luke) : more efficient flag setting?
{
	if (Z < 0) { Z = (reg.F & (u8)Flags::Z) ? 1 : 0; }
	if (N < 0) { N = (reg.F & (u8)Flags::N) ? 1 : 0; }
	if (H < 0) { H = (reg.F & (u8)Flags::H) ? 1 : 0; }
	if (C < 0) { C = (reg.F & (u8)Flags::C) ? 1 : 0; }

	reg.F =
		(Z ? (u8)Flags::Z : 0) |
		(N ? (u8)Flags::N : 0) |
		(H ? (u8)Flags::H : 0) |
		(C ? (u8)Flags::C : 0) |
		(reg.F & (u8)0x0000FFFFu);
}

const int _ = -1;


u8 Add(u8 a, u8 b, int carry)
{
	bool h;
	bool c;
	u8 r = 0;

	for (int i = 0; i < 4; ++i)
	{
		int index = 1 << i;
		int abit = (a & index) ? 1 : 0;
		int bbit = (b & index) ? 1 : 0;
		int rbit = abit + bbit + carry;

		carry = (rbit & 0b10) ? 1 : 0;
		int v = (rbit & 0b01) ? index : 0;
		r = r | v;
	}
	h = carry;

	for (int i = 4; i < 8; ++i)
	{
		int index = 1 << i;
		int abit = (a & index) ? 1 : 0;
		int bbit = (b & index) ? 1 : 0;
		int rbit = abit + bbit + carry;

		carry = (rbit & 0b10) ? 1 : 0;
		int v = (rbit & 0b01) ? index : 0;
		r = r | v;
	}
	c = carry;

	SetFlags(_, _, h, c);
	return r;
}

u8 Sub(u8 a, u8 b, int carry)
{
	bool h;
	bool c;
	u8 r = 0;

	for (int i = 0; i < 4; ++i)
	{
		int index = 1 << i;
		int abit = (a & index) ? 1 : 0;
		int bbit = (b & index) ? 1 : 0;
		int rbit = (abit - bbit) - carry;

		carry = (rbit & 0x10) ? 1 : 0;
		int v = (rbit & 0b01) ? index : 0;
		r = r | v;
	}
	h = carry == 0;

	for (int i = 4; i < 8; ++i)
	{
		int index = 1 << i;
		int abit = (a & index) ? 1 : 0;
		int bbit = (b & index) ? 1 : 0;
		int rbit = (abit - bbit) - carry;

		carry = (rbit & 0x10) ? 1 : 0;
		int v = (rbit & 0b01) ? index : 0;
		r = r | v;
	}
	c = carry == 0;

	SetFlags(_, _, h, c);
	return r;
}



template <std::size_t cost>
std::size_t NOP()
{
	return cost;
}

template <class DST, class SRC, std::size_t cost>
std::size_t LD()
{
	auto a = SRC::Get();
	DST::Set(a);
	return cost;
}

template <class DST, std::size_t cost>
std::size_t LD_SPr8()
{
	u16split as; as.Full = reg.SP;
	u16split bs; bs.Full = r8::Get();
	u16split r{ 0, 0 };

	r.L = Add(as.L, bs.L, 0);
	int carry = (reg.F & (u8)Flags::C) ? 1 : 0;
	r.H = Add(as.H, bs.H, 0);
	DST::Set(r.Full);

	SetFlags(0, 0, _, _);
	return cost;
}

template <class DST, class SRC, std::size_t cost>
std::size_t LDH()
{
	auto a = SRC::Get();
	DST::Set(a);
	return cost;
}

template <class DST, class SRC, std::size_t cost>
std::size_t ADD()
{
	if constexpr (DST::size == 8)
	{
		u8 a = SRC::Get();
		u8 b = DST::Get();
		u8 r = Add(a, b, 0);
		DST::Set(r);

		bool z = r == 0;
		SetFlags(z, 0, _, _);
		return cost;
	}
	else
	{
		u16split as; as.Full = SRC::Get();
		u16split bs; bs.Full = DST::Get();
		u16split r{ 0, 0 };

		r.L = Add(as.L, bs.L, 0);
		int carry = (reg.F & (u8)Flags::C) ? 1 : 0;
		r.H = Add(as.H, bs.H, carry);
		DST::Set(r.Full);

		SetFlags(_, 0, _, _);
		return cost;
	}
}

template <class DST, class SRC, std::size_t cost>
std::size_t ADC()
{
	u8 a = SRC::Get();
	u8 b = DST::Get();
	int carry = (reg.F & (u8)Flags::C) ? 1 : 0;
	u8 r = Add(a, b, 0);
	DST::Set(r);

	bool z = r == 0;
	SetFlags(z, 0, _, _);
	return cost;
}

template <class DST, class SRC, std::size_t cost>
std::size_t SUB()
{
	u8 a = SRC::Get();
	u8 b = DST::Get();
	u8 r = Sub(a, b, 0);
	DST::Set(r);

	bool z = r == 0;
	SetFlags(z, 1, _, _);
	return cost;
}

template <class DST, class SRC, std::size_t cost>
std::size_t SBC()
{
	u8 a = SRC::Get();
	u8 b = DST::Get();
	int carry = (reg.F & (u8)Flags::C) ? 1 : 0;
	u8 r = Sub(a, b, 0);
	DST::Set(r);

	bool z = r == 0;
	SetFlags(z, 1, _, _);
	return cost;
}

template <class DST, class SRC, std::size_t cost>
std::size_t AND()
{
	u8 a = DST::Get();
	u8 b = SRC::Get();
	u8 r = a & b;
	DST::Set(r);

	bool z = r == 0;
	SetFlags(z, 0, 1, 0);
	return cost;
}

template <class DST, class SRC, std::size_t cost>
std::size_t XOR()
{
	u8 a = DST::Get();
	u8 b = SRC::Get();
	u8 r = a ^ b;
	DST::Set(r);

	bool z = r == 0;
	SetFlags(z, 0, 0, 0);
	return cost;
}

template <class DST, class SRC, std::size_t cost>
std::size_t OR()
{
	u8 a = DST::Get();
	u8 b = SRC::Get();
	u8 r = a | b;
	DST::Set(r);

	bool z = r == 0;
	SetFlags(z, 0, 0, 0);
	return cost;
}

template <class REG, class SRC, std::size_t cost>
std::size_t CP()
{
	u8 a = SRC::Get();
	u8 b = REG::Get();
	u8 r = Sub(a, b, 0);

	bool z = r == 0;
	SetFlags(z, 1, _, _);
	return cost;
}

template <class DST, std::size_t cost>
std::size_t INC()
{
	if constexpr (DST::size == 8)
	{
		u8 a = DST::Get();
		u8 r = a++;
		DST::Set(r);

		bool z = r == 0;
		bool h = (a & 0x0F) == 0x0F;
		SetFlags(z, 0, h, _);
		return cost;
	}
	else
	{
		u16 a = DST::Get();
		u16 r = a++;
		DST::Set(r);

		return cost;
	}
}

template <class DST, std::size_t cost>
std::size_t DEC()
{
	if constexpr (DST::size == 8)
	{
		u8 a DST::Get();
		u8 r = a--;
		DST::Set(r);

		bool z = r == 0;
		bool h = (a & 0x0F) != 0;
		SetFlags(z, 1, h, _);
		return costl
	}
	else
	{
		u16 a = DST::Get();
		u16 r = a--;
		DST::Set(r);

		return cost;
	}
}

template <std::size_t cost>
std::size_t RLCA()
{
	u8 a = A:Get();
	bool c = a & 0x80;
	u8 r = (a << 1) | int(c);

	A::Set(r);
	SetFlags(0, 0, 0, c);
	return cost;
}

template <std::size_t cost>
std::size_t RLA()
{
	u8 a = A::Get();
	bool c = reg.F & u8(Flags::C);
	u8 r = (a << 1) | int(c);

	A::Set(r);
	c = a & 0x80;
	SetFlags(0, 0, 0, c);
	return cost;
}

template <std::size_t cost>
std::size_t RRCA()
{
	u8 a = A:Get();
	bool c = a & 0x01;
	u8 r = (a >> 1) | (c ? 0x80 : 0);

	A::Set(r);
	SetFlags(0, 0, 0, c);
	return cost;
}

template <std::size_t cost>
std::size_t RRA()
{
	u8 a = A::Get();
	bool c = reg.F & u8(Flags::C);
	u8 r = (a >> 1) | (c ? 0x80 : 0);

	A::Set(r);
	c = a & 0x01;
	SetFlags(0, 0, 0, c);
	return cost;
}

template <std::size_t cost>
std::size_t DAA()
{
	// allows two binary coded decimal values to be added or subtracted using the normal ADD/SUB operations and store the result in A
	// then DAA can be run restore the A register to the BCD representation of the result
	// i assume this is used to scores/timers etc so they are easier to convert to individual decimal digit tiles

	bool negative = (reg.F & (u8)Flags::N);
	bool carry = (reg.F & (u8)Flags::C);
	bool hcarry = (reg.F & (u8)Flags::H);

	u8 a = A::Get();
	bool c = false;

	if (negative)
	{
		if (carry)
		{
			a -= 0x60;
			c = true;
		}
		if (hcarry)
		{
			a -= 0x06;
		}
	}
	else
	{
		if (carry || a > 0x99)
		{
			a += 0x60;
			c = true;
		}
		if (hcarry || (a & 0xF0) > 0x09)
		{
			a += 0x06;
		}
	}

	A::Set(a);
	bool z = a == 0;
	SetFlags(z, _, 0, c);
	return cost;
}

// todo(luke) : continue on from here

template <std::size_t cost>
std::size_t CPL()
{
	// todo(luke) : implement whatever this is
	return cost;
}

template <std::size_t cost>
std::size_t SFC()
{
	// todo(luke) : implement whatever this is
	return cost;
}

template <std::size_t cost>
std::size_t CCF()
{
	// todo(luke) : implement whatever this is
	return cost;
}

template <std::size_t cost>
std::size_t STOP()
{
	// todo(luke) : stop the cpu somehow
	return cost;
}

template <std::size_t cost>
std::size_t HALT()
{
	// todo(luke) : halt the cpu somehow
	return cost;
}

template <class SRC, std::size_t cost>
std::size_t JR()
{
	s8 offset = SRC::Get();
	reg.PC += offset;			// todo(luke) : does this signed unsinged math work
	return cost;
}

template <class CONDITION, class SRC, std::size_t pass, std::size_t fail>
std::size_t JR()
{
	s8 offset = SRC::Get();
	if (CONDITION::IsMet())
	{
		reg.PC += offset;		// todo(luke) : does this signed unsinged math work
		return pass;
	}
	else
	{
		return fail;
	}
}

template <class CONDITION, class SRC, std::size_t pass, std::size_t fail>
std::size_t JP()
{
	// todo(luke) : implement jp
	return pass;
}

template <class SRC, std::size_t cost>
std::size_t JP()
{
	// todo(luke) : implement jp
	return cost;
}

template <class CONDITION, std::size_t pass, std::size_t fail >
std::size_t RET()
{
	// todo(luke) : implement ret
	return pass;
}

template <std::size_t cost>
std::size_t RET()
{
	// todo(luke) : implement ret
	return cost;
}

template <std::size_t cost>
std::size_t RETI()
{
	// todo(luke) : implement reti
	return cost;
}

template <class SRC, std::size_t cost>
std::size_t POP()
{
	// todo(luke) : implement pop
	return cost;
}

template <class DST, std::size_t cost>
std::size_t PUSH()
{
	// todo(luke) : implement push
	return cost;
}

template <class CONDITION, class SRC, std::size_t pass, std::size_t fail>
std::size_t CALL()
{
	// todo(luke) : implement call
	return pass;
}

template <class SRC, std::size_t cost>
std::size_t CALL()
{
	// todo(luke) : implement call
	return cost;
}

template <std::size_t addr, std::size_t cost>
std::size_t RST()
{
	// todo(luke) : implement rts
	return cost;
}

template <std::size_t cost>
std::size_t DI()
{
	// todo(luke) : implement di
	return cost;
}

template <std::size_t cost>
std::size_t EI()
{
	// todo(luke) : implement ei
	return cost;
}

std::size_t PREFIX_CB()
{
	// todo(luke) : call the secondary operation
	return 4;
}

std::size_t __()
{
	assert(false);
	return 4;
}


typedef std::size_t(*operation)(void);


#define $(_value_) UREF<_value_>
#define i(_value_) IREF<_value_>
#define d(_value_) DREF<_value_>

operation operations[] =
{
	// x0				// x1				// x2				// x3				// x4				// x5				// x6				// x7				// x8				// x9				// xA				// xB				// xC				// xD				// xE				// xF
	NOP<4>,				LD<BC,d16, 12>,		LD<$(BC),A, 8>,		INC<BC, 8>,			INC<B, 4>,			DEC<B, 4>,			LD<B,d8, 8>,		RLCA<4>,			LD<$(a16),SP, 20>,	ADD<HL,BC, 8>,		LD<A,$(BC), 8>,		DEC<BC, 8>,			INC<C, 4>,			DEC<C, 4>,			LD<C,d8, 8>,		RRCA<4>,
	STOP<4>,			LD<DE,d16, 12>,		LD<$(DE),A, 8>,		INC<DE, 8>,			INC<D, 4>,			DEC<D, 4>,			LD<D,d8, 8>,		RLA<4>,				JR<r8, 12>,			ADD<HL,DE, 8>,		LD<A,$(DE), 8>,		DEC<DE, 8>,			INC<E, 4>,			DEC<E, 4>,			LD<E,d8, 8>,		RRA<4>,
	JR<NZ,r8, 12,8>,	LD<HL,d16, 12>,		LD<i(HL),A, 8>,		INC<HL, 8>,			INC<H, 4>,			DEC<H, 4>,			LD<H,d8, 8>,		DAA<4>,				JR<Z,r8, 12,8>,		ADD<HL,HL, 8>,		LD<A,i(HL), 8>,		DEC<HL, 8>,			INC<L, 4>,			DEC<L, 4>,			LD<L,d8, 8>,		CPL<4>,
	JR<NC,r8, 12,8>,	LD<SP,d16, 12>,		LD<d(HL),A, 8>,		INC<SP, 8>,			INC<$(HL), 12>,		DEC<$(HL), 12>,		LD<$(HL),d8, 12>,	SFC<4>,				JR<C,r8, 12,8>,		ADD<HL,SP, 8>,		LD<A,d(HL), 8>,		DEC<SP, 8>,			INC<A, 4>,			DEC<A, 4>,			LD<A,d8, 8>,		CCF<4>,
	LD<B,B, 4>,			LD<B,C, 4>,			LD<B,D, 4>,			LD<B,E, 4>,			LD<B,H, 4>,			LD<B,L, 4>,			LD<B,$(HL), 8>,		LD<B,A, 4>,			LD<C,B, 4>,			LD<C,C, 4>,			LD<C,D, 4>,			LD<C,E, 4>,			LD<C,H, 4>,			LD<C,L, 4>,			LD<C,$(HL), 8>,		LD<C,A, 4>,
	LD<D,B, 4>,			LD<D,C, 4>,			LD<D,D, 4>,			LD<D,E, 4>,			LD<D,H, 4>,			LD<D,L, 4>,			LD<D,$(HL), 8>,		LD<D,A, 4>,			LD<E,B, 4>,			LD<E,C, 4>,			LD<E,D, 4>,			LD<E,E, 4>,			LD<E,H, 4>,			LD<E,L, 4>,			LD<E,$(HL), 8>,		LD<E,A, 4>,
	LD<H,B, 4>,			LD<H,C, 4>,			LD<H,D, 4>,			LD<H,E, 4>,			LD<H,H, 4>,			LD<H,L, 4>,			LD<H,$(HL), 8>,		LD<H,A, 4>,			LD<L,B, 4>,			LD<L,C, 4>,			LD<L,D, 4>,			LD<L,E, 4>,			LD<L,H, 4>,			LD<L,L, 4>,			LD<L,$(HL), 8>,		LD<L,A, 4>,
	LD<$(HL),B, 4>,		LD<$(HL),C, 4>,		LD<$(HL),D, 4>,		LD<$(HL),E, 4>,		LD<$(HL),H, 4>,		LD<$(HL),L, 4>,		HALT<4>,			LD<A,A, 4>,			LD<A,B, 4>,			LD<A,C, 4>,			LD<A,D, 4>,			LD<A,E, 4>,			LD<A,H, 4>,			LD<A,L, 4>,			LD<A,$(HL), 8>,		LD<A,A, 4>,
	ADD<A,B, 4>,		ADD<A,C, 4>,		ADD<A,D, 4>,		ADD<A,E, 4>,		ADD<A,H, 4>,		ADD<A,L, 4>,		ADD<A,$(HL), 8>,	ADD<A,A, 4>,		ADC<A,B, 4>,		ADC<A,C, 4>,		ADC<A,D, 4>,		ADC<A,E, 4>,		ADC<A,H, 4>,		ADC<A,L, 4>,		ADC<A,$(HL), 8>,	ADC<A,A, 4>,
	SUB<A,B, 4>,		SUB<A,C, 4>,		SUB<A,D, 4>,		SUB<A,E, 4>,		SUB<A,H, 4>,		SUB<A,L, 4>,		SUB<A,$(HL), 8>,	SUB<A,A, 4>,		SBC<A,B, 4>,		SBC<A,C, 4>,		SBC<A,D, 4>,		SBC<A,E, 4>,		SBC<A,H, 4>,		SBC<A,L, 4>,		SBC<A,$(HL), 8>,	SBC<A,A, 4>,
	AND<A,B, 4>,		AND<A,C, 4>,		AND<A,D, 4>,		AND<A,E, 4>,		AND<A,H, 4>,		AND<A,L, 4>,		AND<A,$(HL), 8>,	AND<A,A, 4>,		XOR<A,B, 4>,		XOR<A,C, 4>,		XOR<A,D, 4>,		XOR<A,E, 4>,		XOR<A,H, 4>,		XOR<A,L, 4>,		XOR<A,$(HL), 8>,	XOR<A,A, 4>,
	OR<A,B, 4>,			OR<A,C, 4>,			OR<A,D, 4>,			OR<A,E, 4>,			OR<A,H, 4>,			OR<A,L, 4>,			OR<A,$(HL), 8>,		OR<A,A, 4>,			CP<A,B, 4>,			CP<A,C, 4>,			CP<A,D, 4>,			CP<A,E, 4>,			CP<A,H, 4>,			CP<A,L, 4>,			CP<A,$(HL), 8>,		CP<A,A, 4>,
	RET<NZ, 20,8>,		POP<BC, 12>,		JP<NZ,a16, 16,12>,	JP<a16, 16>,		CALL<NZ,a16, 24,12>,PUSH<BC, 16>,		ADD<A,d8, 8>,		RST<0x00, 16>,		RET<Z, 20,8>,		RET<16>,			JP<Z,a16, 16,12>,	PREFIX_CB,			CALL<Z,a16, 24,12>,	CALL<a16, 24>,		ADC<A,d8, 8>,		RST<0x08, 16>,
	RET<NC, 20,8>,		POP<DE, 12>,		JP<NC,a16, 16,12>,	__,					CALL<NC,a16, 24,12>,PUSH<DE, 16>,		SUB<A,d8, 8>,		RST<0x10, 16>,		RET<C, 20,8>,		RETI<16>,			JP<C,a16, 16,12>,	__,					CALL<C,a16, 24,12>,	__,					SBC<A,d8, 8>,		RST<0x18, 16>,
	LDH<$(a8),A, 12>,	POP<HL, 12>,		LD<$(C),A, 8>,		__,					__,					PUSH<HL, 16>,		AND<A,d8, 8>,		RST<0x20, 16>,		ADD<SP,r8, 16>,		JP<$(HL), 4>,		LD<$(a16),A, 16>,	__,					__,					__,					XOR<A,d8, 8>,		RST<0x28, 16>,
	LDH<A,$(a8), 12>,	POP<AF, 12>,		LD<A,$(C), 8>,		DI<4>,				__,					PUSH<AF, 16>,		OR<A,d8, 8>,		RST<0x30, 16>,		LD_SPr8<HL, 12>,	LD<HL,SP, 8>,		LD<A,$(a16), 16>,	EI<4>,				__,					__,					CP<A,d8, 8>,		RST<0x38, 16>
};

#undef $
#undef i
#undef d