#include <assert.h>
#include <queue>

#include "cpu.h"

#include "Bus.h"
#include "constants.h"
#include "math.h"
#include "types.h"

Registers reg;

static bool bHalted = false;
static bool bRepeatPCPostHalt = false;

static int interruptDisableDelay = -1;
static int interruptEnableDelay = -1;

static bool interruptMasterEnable = false;


std::size_t cycles = 0;
typedef std::size_t(*operation)(void); // todo(luke) : clean up where this new stuff lives at some point
operation operations[];
operation exops[]; // todo(luke) : move the forward declerations somewhere a bit better


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
		if (cycles == 0)
		{
			//keeping this around as its handy if we ever need to keep a log of register changes per ops for debuggin
			//---
			//freopen("output.txt", "w", stdout);
			//std::cout << "PC:" << std::setfill('0') << std::setw(4) << std::hex << reg.PC << " ";
			//std::cout << "SP:" << std::setfill('0') << std::setw(4) << std::hex << reg.SP << " ";
			//std::cout << "AF:" << std::setfill('0') << std::setw(4) << std::hex << reg.AF << " ";
			//std::cout << "BC:" << std::setfill('0') << std::setw(4) << std::hex << reg.BC << " ";
			//std::cout << "DE:" << std::setfill('0') << std::setw(4) << std::hex << reg.DE << " ";
			//std::cout << "HL:" << std::setfill('0') << std::setw(4) << std::hex << reg.HL;
			//std::cout << std::endl << std::flush;

			// note :	some docs suggest the change happens after the next machine cycle, some suggest after the next opcode is executed.
			//			After the next machine cycle means reading the next opcode, but not the operands (if there are any) before handling the interrupt.
			//			I can't reason how this would be safe, so i am taking the "after the next opcode is executed" reading
			//			There's no _logical_ difference between the 2 options, but there is a slight _timing_ difference.
			HandleIMEFlagChange();
			HandlePendingInterrupt();
			u8 opcode = Bus::LoadU8(reg.PC++);
			HandleHaltInstructionSideEffects();
			cycles = operations[opcode]();
		}

		cycles -= 4;
	}
};


// todo(luke) : more all the reigster declerations etc to their own locations etc

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
	static const std::size_t size = 8;
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
	static const std::size_t size = 8;
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
	static const std::size_t size = 8;
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
		(reg.F & u8(0x0F));
}

const int _ = -1;


void Push(u16 value)
{
	u16split s; s.Full = value;
	Bus::StoreU8(--reg.SP, s.L);	// todo(luke) : verify that these writes are in the correct order
	Bus::StoreU8(--reg.SP, s.H);
}

u16 Pop()
{
	u16split s;
	s.H = Bus::LoadU8(reg.SP++);
	s.L = Bus::LoadU8(reg.SP++);
	return s.Full;
}


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
	h = carry;

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
	c = carry;

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
		u8 a = DST::Get();
		u8 b = SRC::Get();
		u8 r = Add(a, b, 0);
		DST::Set(r);

		bool z = r == 0;
		SetFlags(z, 0, _, _);
		return cost;
	}
	else
	{
		u16split as; as.Full = DST::Get();
		u16split bs; bs.Full = SRC::Get();
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
	u8 a = DST::Get();
	u8 b = SRC::Get();
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
	u8 a = DST::Get();
	u8 b = SRC::Get();
	u8 r = Sub(a, b, 0);
	DST::Set(r);

	bool z = r == 0;
	SetFlags(z, 1, _, _);
	return cost;
}

template <class DST, class SRC, std::size_t cost>
std::size_t SBC()
{
	u8 a = DST::Get();
	u8 b = SRC::Get();
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
	u8 a = REG::Get();
	u8 b = SRC::Get();
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
		u8 r = a + 1;
		DST::Set(r);

		bool z = r == 0;
		bool h = (a & 0x0F) == 0x0F;
		SetFlags(z, 0, h, _);
		return cost;
	}
	else
	{
		u16 a = DST::Get();
		u16 r = a + 1;
		DST::Set(r);

		return cost;
	}
}

template <class DST, std::size_t cost>
std::size_t DEC()
{
	if constexpr (DST::size == 8)
	{
		u8 a = DST::Get();
		u8 r = a - 1;
		DST::Set(r);

		bool z = r == 0;
		bool h = (a & 0x0F) == 0; // todo(luke) : should this be set on carry or when not carrying?
		SetFlags(z, 1, h, _);
		return cost;
	}
	else
	{
		u16 a = DST::Get();
		u16 r = a - 1;
		DST::Set(r);

		return cost;
	}
}

template <class SRC, std::size_t cost>
std::size_t RLC()
{
	u8 a = SRC::Get();
	bool c = a & 0x80;
	u8 r = (a << 1) | int(c);

	SRC::Set(r);
	bool z = r == 0;
	SetFlags(z, 0, 0, c);
	return cost;
}

template <std::size_t cost>
std::size_t RLCA()
{
	return RLC<A, cost>();
}

template <class SRC, std::size_t cost>
std::size_t RL()
{
	u8 a = SRC::Get();
	bool c = reg.F & u8(Flags::C);
	u8 r = (a << 1) | int(c);

	SRC::Set(r);
	c = a & 0x80;
	bool z = r == 0;
	SetFlags(z, 0, 0, c);
	return cost;
}

template <std::size_t cost>
std::size_t RLA()	// todo(luke) : check z flag output for other rotate on A operations
{
	std::size_t scost = RL<A, cost>();
	SetFlags(0, _, _, _);
	return scost;
}

template <class SRC, std::size_t cost>
std::size_t RRC()
{
	u8 a = SRC::Get();
	bool c = a & 0x01;
	u8 r = (a >> 1) | (c ? 0x80 : 0);

	SRC::Set(r);
	bool z = r == 0;
	SetFlags(z, 0, 0, c);
	return cost;
}

template <std::size_t cost>
std::size_t RRCA()
{
	return RRC<A, cost>();
}

template <class SRC, std::size_t cost>
std::size_t RR()
{
	u8 a = SRC::Get();
	bool c = reg.F & u8(Flags::C);
	u8 r = (a >> 1) | (c ? 0x80 : 0);

	SRC::Set(r);
	c = a & 0x01;
	bool z = r == 0;
	SetFlags(z, 0, 0, c);
	return cost;
}

template <std::size_t cost>
std::size_t RRA()
{
	return RR<A, cost>();
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

template <std::size_t cost>
std::size_t CPL()
{
	u8 a = A::Get();
	a = ~a;

	A::Set(a);
	SetFlags(_, 1, 1, _);
	return cost;
}

template <std::size_t cost>
std::size_t SCF()
{
	SetFlags(_, 0, 0, 1);
	return cost;
}

template <std::size_t cost>
std::size_t CCF()
{
	bool c = reg.F & (u8)Flags::C;
	SetFlags(_, 0, 0, !c);
	return cost;
}

template <std::size_t cost>
std::size_t HALT()
{
	if (interruptMasterEnable)
	{
		bHalted = true;
	}
	else
	{
		// todo if (GB/SGB/GBP)		todo(vanrz) : implement what this is?
		bRepeatPCPostHalt = true;
	}
	return cost;
}

template <std::size_t cost>
std::size_t STOP()
{
	// todo(luke) : stop the cpu somehow
	return cost;
}

template <class SRC, std::size_t cost>
std::size_t JR()
{
	u16 offset = SRC::Get();
	reg.PC += offset;
	return cost;
}

template <class CONDITION, class SRC, std::size_t pass, std::size_t fail>
std::size_t JR()
{
	u16 offset = SRC::Get();
	if (CONDITION::IsMet())
	{
		reg.PC += offset;
		return pass;
	}
	else
	{
		return fail;
	}
}

template <class SRC, std::size_t cost>
std::size_t JP()
{
	u16 a = SRC::Get();
	reg.PC = a;
	return cost;
}

template <class CONDITION, class SRC, std::size_t pass, std::size_t fail>
std::size_t JP()
{
	u16 a = SRC::Get();
	if (CONDITION::IsMet())
	{
		reg.PC = a;
		return pass;
	}
	else
	{
		return fail;
	}
}

template <std::size_t cost>
std::size_t RET()
{
	u16 a = Pop();
	reg.PC = a;
	return cost;
}

template <class CONDITION, std::size_t pass, std::size_t fail >
std::size_t RET()
{
	if (CONDITION::IsMet())
	{
		u16 a = Pop();
		reg.PC = a;
		return pass;
	}
	else
	{
		return fail;
	}
}

template <std::size_t cost>
std::size_t RETI()
{
	u16 a = Pop();
	reg.HL = a;
	interruptEnableDelay = 1;
	return cost;
}

template <class SRC, std::size_t cost>
std::size_t PUSH()
{
	u16 v = SRC::Get();
	Push(v);
	return cost;
}

template <class DST, std::size_t cost>
std::size_t POP()
{
	u16 v = Pop();
	DST::Set(v);
	return cost;
}

template <class SRC, std::size_t cost>
std::size_t CALL()
{
	u16 a = SRC::Get();
	Push(reg.PC);
	reg.PC = a;
	return cost;
}

template <class CONDITION, class SRC, std::size_t pass, std::size_t fail>
std::size_t CALL()
{
	u16 a = SRC::Get();
	if (CONDITION::IsMet())
	{
		Push(reg.PC);
		reg.PC = a;
		return pass;
	}
	else
	{
		return fail;
	}
}

template <u16 addr, std::size_t cost>
std::size_t RST()
{
	Push(reg.PC);
	reg.PC = addr;
	return cost;
}

template <std::size_t cost>
std::size_t EI()
{
	interruptEnableDelay = 1;
	return cost;
}

template <std::size_t cost>
std::size_t DI()
{
	interruptDisableDelay = 1;
	return cost;
}

std::size_t PREFIX_CB()
{
	u8 opcode = Bus::LoadU8(reg.PC++);
	return exops[opcode]();
}

std::size_t __()
{
	assert(false);
	return 4;
}

template <class SRC, std::size_t cost>
std::size_t SLA()
{
	u8 a = SRC::Get();
	u8 r = a << 1;

	SRC::Set(r);
	bool c = a & 0x80;
	bool z = r == 0;
	SetFlags(z, 0, 0, c);
	return cost;
}

template <class SRC, std::size_t cost>
std::size_t SRA()
{
	u8 a = SRC::Get();
	u8 r = (a & 0x80) | ((a >> 1) & 0x7F);

	SRC::Set(r);
	bool c = a & 0x01;
	bool z = r == 0;
	SetFlags(z, 0, 0, c);
	return cost;
}

template <class SRC, std::size_t cost>
std::size_t SWAP()
{
	u8 a = SRC::Get();
	u8 r = ((a << 4) & 0xF0) | ((a >> 4) & 0x0F);

	SRC::Set(r);
	bool z = r == 0;
	SetFlags(z, 0, 0, 0);
	return cost;
}

template <class SRC, std::size_t cost>
std::size_t SRL()
{
	u8 a = SRC::Get();
	u8 r = (a >> 1) & 0x7F;

	SRC::Set(r);
	bool c = a & 0x01;
	bool z = r == 0;
	SetFlags(z, 0, 0, c);
	return cost;
}

template <int I, class SRC, std::size_t cost>
std::size_t BIT()
{
	u8 a = SRC::Get();
	bool z = (a & (1 << I)) == 0;
	SetFlags(z, 0, 1, _);
	return cost;
}

template <int I, class SRC, std::size_t cost>
std::size_t RES()
{
	u8 a = SRC::Get();
	u8 r = a | (1 << I);

	SRC::Set(r);
	return cost;
}

template <int I, class SRC, std::size_t cost>
std::size_t SET()
{
	u8 a = SRC::Get();
	u8 r = a & ~(1 << I);

	SRC::Set(r);
	return cost;
}




#define $(_value_) UREF<_value_>
#define i(_value_) IREF<_value_>
#define d(_value_) DREF<_value_>

operation operations[] =
{
	// x0				// x1				// x2				// x3				// x4				// x5				// x6				// x7				// x8				// x9				// xA				// xB				// xC				// xD				// xE				// xF
	NOP<4>,				LD<BC,d16, 12>,		LD<$(BC),A, 8>,		INC<BC, 8>,			INC<B, 4>,			DEC<B, 4>,			LD<B,d8, 8>,		RLCA<4>,			LD<$(a16),SP, 20>,	ADD<HL,BC, 8>,		LD<A,$(BC), 8>,		DEC<BC, 8>,			INC<C, 4>,			DEC<C, 4>,			LD<C,d8, 8>,		RRCA<4>,
	STOP<4>,			LD<DE,d16, 12>,		LD<$(DE),A, 8>,		INC<DE, 8>,			INC<D, 4>,			DEC<D, 4>,			LD<D,d8, 8>,		RLA<4>,				JR<r8, 12>,			ADD<HL,DE, 8>,		LD<A,$(DE), 8>,		DEC<DE, 8>,			INC<E, 4>,			DEC<E, 4>,			LD<E,d8, 8>,		RRA<4>,
	JR<NZ,r8, 12,8>,	LD<HL,d16, 12>,		LD<i(HL),A, 8>,		INC<HL, 8>,			INC<H, 4>,			DEC<H, 4>,			LD<H,d8, 8>,		DAA<4>,				JR<Z,r8, 12,8>,		ADD<HL,HL, 8>,		LD<A,i(HL), 8>,		DEC<HL, 8>,			INC<L, 4>,			DEC<L, 4>,			LD<L,d8, 8>,		CPL<4>,
	JR<NC,r8, 12,8>,	LD<SP,d16, 12>,		LD<d(HL),A, 8>,		INC<SP, 8>,			INC<$(HL), 12>,		DEC<$(HL), 12>,		LD<$(HL),d8, 12>,	SCF<4>,				JR<C,r8, 12,8>,		ADD<HL,SP, 8>,		LD<A,d(HL), 8>,		DEC<SP, 8>,			INC<A, 4>,			DEC<A, 4>,			LD<A,d8, 8>,		CCF<4>,
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
	LDH<$(a8),A, 12>,	POP<HL, 12>,		LD<$(C),A, 8>,		__,					__,					PUSH<HL, 16>,		AND<A,d8, 8>,		RST<0x20, 16>,		ADD<SP,r8, 16>,		JP<HL, 4>,			LD<$(a16),A, 16>,	__,					__,					__,					XOR<A,d8, 8>,		RST<0x28, 16>,
	LDH<A,$(a8), 12>,	POP<AF, 12>,		LD<A,$(C), 8>,		DI<4>,				__,					PUSH<AF, 16>,		OR<A,d8, 8>,		RST<0x30, 16>,		LD_SPr8<HL, 12>,	LD<HL,SP, 8>,		LD<A,$(a16), 16>,	EI<4>,				__,					__,					CP<A,d8, 8>,		RST<0x38, 16>
};

operation exops[] =
{
	// x0				// x1				// x2				// x3				// x4				// x5				// x6				// x7				// x8				// x9				// xA				// xB				// xC				// xD				// xE				// xF
	RLC<B, 8>,			RLC<C, 8>,			RLC<D, 8>,			RLC<E, 8>,			RLC<H, 8>,			RLC<L, 8>,			RLC<$(HL), 16>,		RLC<A, 8>,			RRC<B, 8>,			RRC<C, 8>,			RRC<D, 8>,			RRC<E, 8>,			RRC<H, 8>,			RRC<L, 8>,			RRC<$(HL), 16>,		RRC<A, 8>,
	RL<B, 8>,			RL<C, 8>,			RL<D, 8>,			RL<E, 8>,			RL<H, 8>,			RL<L, 8>,			RL<$(HL), 16>,		RL<A, 8>,			RR<B, 8>,			RR<C, 8>,			RR<D, 8>,			RR<E, 8>,			RR<H, 8>,			RR<L, 8>,			RR<$(HL), 16>,		RR<A, 8>,
	SLA<B, 8>,			SLA<C, 8>,			SLA<D, 8>,			SLA<E, 8>,			SLA<H, 8>,			SLA<L, 8>,			SLA<$(HL), 16>,		SLA<A, 8>,			SRA<B, 8>,			SRA<C, 8>,			SRA<D, 8>,			SRA<E, 8>,			SRA<H, 8>,			SRA<L, 8>,			SRA<$(HL), 16>,		SRA<A, 8>,
	SWAP<B, 8>,			SWAP<C, 8>,			SWAP<D, 8>,			SWAP<E, 8>,			SWAP<H, 8>,			SWAP<L, 8>,			SWAP<$(HL), 16>,	SWAP<A, 8>,			SRL<B, 8>,			SRL<C, 8>,			SRL<D, 8>,			SRL<E, 8>,			SRL<H, 8>,			SRL<L, 8>,			SRL<$(HL), 16>,		SRL<A, 8>,
	BIT<0,B, 8>,		BIT<0,C, 8>,		BIT<0,D, 8>,		BIT<0,E, 8>,		BIT<0,H, 8>,		BIT<0,L, 8>,		BIT<0,$(HL), 12>,	BIT<0,A, 8>,		BIT<1,B, 8>,		BIT<1,C, 8>,		BIT<1,D, 8>,		BIT<1,E, 8>,		BIT<1,H, 8>,		BIT<1,L, 8>,		BIT<1,$(HL), 12>,	BIT<1,A, 8>,
	BIT<2,B, 8>,		BIT<2,C, 8>,		BIT<2,D, 8>,		BIT<2,E, 8>,		BIT<2,H, 8>,		BIT<2,L, 8>,		BIT<2,$(HL), 32>,	BIT<2,A, 8>,		BIT<3,B, 8>,		BIT<3,C, 8>,		BIT<3,D, 8>,		BIT<3,E, 8>,		BIT<3,H, 8>,		BIT<3,L, 8>,		BIT<3,$(HL), 32>,	BIT<3,A, 8>,
	BIT<4,B, 8>,		BIT<4,C, 8>,		BIT<4,D, 8>,		BIT<4,E, 8>,		BIT<4,H, 8>,		BIT<4,L, 8>,		BIT<4,$(HL), 52>,	BIT<4,A, 8>,		BIT<5,B, 8>,		BIT<5,C, 8>,		BIT<5,D, 8>,		BIT<5,E, 8>,		BIT<5,H, 8>,		BIT<5,L, 8>,		BIT<5,$(HL), 52>,	BIT<5,A, 8>,
	BIT<6,B, 8>,		BIT<6,C, 8>,		BIT<6,D, 8>,		BIT<6,E, 8>,		BIT<6,H, 8>,		BIT<6,L, 8>,		BIT<6,$(HL), 72>,	BIT<6,A, 8>,		BIT<7,B, 8>,		BIT<7,C, 8>,		BIT<7,D, 8>,		BIT<7,E, 8>,		BIT<7,H, 8>,		BIT<7,L, 8>,		BIT<7,$(HL), 72>,	BIT<7,A, 8>,
	RES<0,B, 8>,		RES<0,C, 8>,		RES<0,D, 8>,		RES<0,E, 8>,		RES<0,H, 8>,		RES<0,L, 8>,		RES<0,$(HL), 12>,	RES<0,A, 8>,		RES<1,B, 8>,		RES<1,C, 8>,		RES<1,D, 8>,		RES<1,E, 8>,		RES<1,H, 8>,		RES<1,L, 8>,		RES<1,$(HL), 12>,	RES<1,A, 8>,
	RES<2,B, 8>,		RES<2,C, 8>,		RES<2,D, 8>,		RES<2,E, 8>,		RES<2,H, 8>,		RES<2,L, 8>,		RES<2,$(HL), 32>,	RES<2,A, 8>,		RES<3,B, 8>,		RES<3,C, 8>,		RES<3,D, 8>,		RES<3,E, 8>,		RES<3,H, 8>,		RES<3,L, 8>,		RES<3,$(HL), 32>,	RES<3,A, 8>,
	RES<4,B, 8>,		RES<4,C, 8>,		RES<4,D, 8>,		RES<4,E, 8>,		RES<4,H, 8>,		RES<4,L, 8>,		RES<4,$(HL), 52>,	RES<4,A, 8>,		RES<5,B, 8>,		RES<5,C, 8>,		RES<5,D, 8>,		RES<5,E, 8>,		RES<5,H, 8>,		RES<5,L, 8>,		RES<5,$(HL), 52>,	RES<5,A, 8>,
	RES<6,B, 8>,		RES<6,C, 8>,		RES<6,D, 8>,		RES<6,E, 8>,		RES<6,H, 8>,		RES<6,L, 8>,		RES<6,$(HL), 72>,	RES<6,A, 8>,		RES<7,B, 8>,		RES<7,C, 8>,		RES<7,D, 8>,		RES<7,E, 8>,		RES<7,H, 8>,		RES<7,L, 8>,		RES<7,$(HL), 72>,	RES<7,A, 8>,
	SET<0,B, 8>,		SET<0,C, 8>,		SET<0,D, 8>,		SET<0,E, 8>,		SET<0,H, 8>,		SET<0,L, 8>,		SET<0,$(HL), 12>,	SET<0,A, 8>,		SET<1,B, 8>,		SET<1,C, 8>,		SET<1,D, 8>,		SET<1,E, 8>,		SET<1,H, 8>,		SET<1,L, 8>,		SET<1,$(HL), 12>,	SET<1,A, 8>,
	SET<2,B, 8>,		SET<2,C, 8>,		SET<2,D, 8>,		SET<2,E, 8>,		SET<2,H, 8>,		SET<2,L, 8>,		SET<2,$(HL), 32>,	SET<2,A, 8>,		SET<3,B, 8>,		SET<3,C, 8>,		SET<3,D, 8>,		SET<3,E, 8>,		SET<3,H, 8>,		SET<3,L, 8>,		SET<3,$(HL), 32>,	SET<3,A, 8>,
	SET<4,B, 8>,		SET<4,C, 8>,		SET<4,D, 8>,		SET<4,E, 8>,		SET<4,H, 8>,		SET<4,L, 8>,		SET<4,$(HL), 52>,	SET<4,A, 8>,		SET<5,B, 8>,		SET<5,C, 8>,		SET<5,D, 8>,		SET<5,E, 8>,		SET<5,H, 8>,		SET<5,L, 8>,		SET<5,$(HL), 52>,	SET<5,A, 8>,
	SET<6,B, 8>,		SET<6,C, 8>,		SET<6,D, 8>,		SET<6,E, 8>,		SET<6,H, 8>,		SET<6,L, 8>,		SET<6,$(HL), 72>,	SET<6,A, 8>,		SET<7,B, 8>,		SET<7,C, 8>,		SET<7,D, 8>,		SET<7,E, 8>,		SET<7,H, 8>,		SET<7,L, 8>,		SET<7,$(HL), 72>,	SET<7,A, 8>


};

#undef $
#undef i
#undef d