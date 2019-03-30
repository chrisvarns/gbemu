#include "bootrom.h"
#include "bus.h"
#include "memory.h"
#include "specialRegisters.h"
#include "utils.h"
#include <assert.h>

u8 HandleIORead(u16 address)
{
	switch (SpecialRegister(address))
	{
	case SpecialRegister::SOUND_NR11:
	case SpecialRegister::SOUND_NR12:
	case SpecialRegister::SOUND_NR13:
	case SpecialRegister::SOUND_NR14:
	case SpecialRegister::SOUND_NR50:
	case SpecialRegister::SOUND_NR51:
	case SpecialRegister::SOUND_NR52:
	case SpecialRegister::VIDEO_SCY:
	case SpecialRegister::VIDEO_SCX:
	case SpecialRegister::VIDEO_LY:
	{
		return Memory::LoadU8(address);
	}
	}
	assert(false);
	return 0;
}

void HandleIOWrite(u16 address, u8 val)
{
	switch ((SpecialRegister)address)
	{
	case SpecialRegister::SOUND_NR11:
	case SpecialRegister::SOUND_NR12:
	case SpecialRegister::SOUND_NR13:
	case SpecialRegister::SOUND_NR14:
	case SpecialRegister::SOUND_NR50:
	case SpecialRegister::SOUND_NR51:
	case SpecialRegister::SOUND_NR52:
	{
		// todo
		break;
	}
	case SpecialRegister::VIDEO_LCDC:
	case SpecialRegister::VIDEO_SCY:
	case SpecialRegister::VIDEO_SCX:
	case SpecialRegister::VIDEO_BGP:
	case SpecialRegister::BOOTROM_SWITCH:
	{
		Memory::StoreU8(address, val);
		break;
	}
	case SpecialRegister::VIDEO_LY:
	default:
		assert(false);
	}
}

u8 Bus::LoadU8(u16 address)
{
	if (InRange(address, 0x0000, 0x0100))
	{
		// Either bootrom or cart rom
		if (Memory::LoadU8((u16)SpecialRegister::BOOTROM_SWITCH))
		{
			// 16KB Cartridge ROM bank #0
			return Memory::LoadU8(address);
		}
		else
		{
			// Boot ROM
			return BootRom::LoadU8(address);
		}
	}
	else if (InRange(address, 0x0100, 0x4000))
	{
		// 16KB Cartridge ROM bank #0
		return Memory::LoadU8(address);
	}
	else if (InRange(address, 0x4000, 0x8000))
	{
		// 16KB Cartridge ROM bank (switchable via MBC, some titles)
		assert(false); // We're outside the DMG boot rom, need to actually load a ROM then.
		return Memory::LoadU8(address);
	}
	else if (InRange(address, 0x8000, 0xA000))
	{
		// 8KB Video RAM
		return Memory::LoadU8(address);
	}
	else if (InRange(address, 0xA000, 0xC000))
	{
 		// 8KB switchable RAM bank
		// todo handle RAM bank switching
		return Memory::LoadU8(address);
	}
	else if (InRange(address, 0xC000, 0xE000))
	{
		// 8KB Internal RAM
		return Memory::LoadU8(address);
	}
	else if (InRange(address, 0xE000, 0xFE00))
	{
		// echo of 8KB Internal RAM
		return Memory::LoadU8(address - 0x2000);
	}
	else if (InRange(address, 0xFE00, 0xFEA0))
	{
		// Sprite Attrib Memory (OAM)
		// todo
		assert(false);
		return 0;
	}
	else if (InRange(address, 0xFEA0, 0xFF00))
	{
		// Unusable
		assert(false);
		return 0;
	}
	else if (InRange(address, 0xFF00, 0xFF4C))
	{
		// I/O ports
		return HandleIORead(address);
	}
	else if (InRange(address, 0xFF80, 0xFFFF))
	{
		// Internal RAM
		return Memory::LoadU8(address);
	}
	else
	{
		assert(address == 0xFFFF);
		// Interrupt Enable Register
		return Memory::LoadU8(address);
	}
}

void Bus::StoreU8(u16 address, u8 val)
{
	if (InRange(address, 0x0000, 0x8000))
	{
		// 32KB ROM space
		if (InRange(address, 0x0000, 0x2000))
		{
			// TODO RAM Bank Enable
		}
		else if (InRange(address, 0x2000, 0x4000))
		{
			// TODO ROM Bank Select
		}
		else if (InRange(address, 0x4000, 0x6000))
		{
			// TODO RAM Bank Select
		}
		else if (InRange(address, 0x6000, 0x8000))
		{
			// TODO MBC1 ROM/RAM Select
		}
	}
	else if (InRange(address, 0x8000, 0xA000))
	{
		// 8KB Video RAM
		Memory::StoreU8(address, val);
	}
	else if (InRange(address, 0xA000, 0xC000))
	{
		// 8KB switchable RAM bank
		// todo handle RAM bank switching
		Memory::StoreU8(address, val);
	}
	else if (InRange(address, 0xC000, 0xE000))
	{
		// 8KB Internal RAM
		Memory::StoreU8(address, val);
	}
	else if (InRange(address, 0xE000, 0xFE00))
	{
		// echo of 8KB Internal RAM
		Memory::StoreU8(address - 0x2000, val);
	}
	else if (InRange(address, 0xFE00, 0xFEA0))
	{
		// Sprite Attrib Memory (OAM)
		// todo
		assert(false);
	}
	else if (InRange(address, 0xFEA0, 0xFF00))
	{
		// Unusable
		assert(false);
	}
	else if (InRange(address, 0xFF00, 0xFF80))
	{
		// I/O ports
		HandleIOWrite(address, val);
	}
	else if (InRange(address, 0xFF80, 0xFFFF))
	{
		// Internal RAM
		Memory::StoreU8(address, val);
	}
	else
	{
		assert(address == 0xFFFF);
		// Interrupt Enable Register
		Memory::StoreU8(address, val);
	}
}

