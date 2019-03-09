#include "bus.h"
#include "memory.h"
#include "bootrom.h"

#include <assert.h>

enum class SpecialRegisters
{
	BOOTROM_SWITCH = 0xFF50
};

bool InRange(u16 address, u16 lowerInclusive, u16 upperExclusive)
{
	return address >= lowerInclusive && address < upperExclusive;
}

u8 Bus::LoadU8(u16 address)
{
	if (InRange(address, 0x0000, 0x0100))
	{
		// BootRom/Cart depends on switch
		if (Memory::LoadU8((u8)SpecialRegisters::BOOTROM_SWITCH))
		{
			assert(false); // We're outside the DMG boot rom, need to actually load a ROM then.
			return Memory::LoadU8(address);
		}
		else
		{
			return BootRom::LoadU8(address);
		}
	}
	else if (InRange(address, 0x0100, 0x4000))
	{
		// 16KB ROM bank #0
		if (!InRange(address, 0x0000, 0x0100))
		{
			assert(false); // We're outside the DMG boot rom, need to actually load a ROM then.
		}
		return Memory::LoadU8(address);
	}
	else if (InRange(address, 0x4000, 0x8000))
	{
		// 16KB switchable ROM bank
		// todo handle ROM bank switching.
		assert(false); // We're outside the DMG boot rom, need to actually load a ROM then.
		return Memory::LoadU8(address);
	}
	else if (InRange(address, 0x8000, 0xA000))
	{
		// 8KB Video RAM
		// todo handle VRAM
		assert(false);
		return 0;
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
		// todo handle RAM bank switching
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
		// Empty but unusable for I/O
		assert(false);
		return 0;
	}
	else if (InRange(address, 0xFF00, 0xFF4C))
	{
		// I/O ports
		return Memory::LoadU8(address);
	}
	else if (InRange(address, 0xFF4C, 0xFF80))
	{
		// Empty but unusable for I/O
		assert(false);
		return 0;
	}
	else if (InRange(address, 0xFF80, 0xFFFF))
	{
		// Internal RAM
		// todo ?
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
		assert(false && "Illegal write to ROM");
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
		// todo handle RAM bank switching
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
		// Empty but unusable for I/O
		assert(false);
	}
	else if (InRange(address, 0xFF00, 0xFF4C))
	{
		// I/O ports
		Memory::StoreU8(address, val);
	}
	else if (InRange(address, 0xFF4C, 0xFF80))
	{
		// Empty but unusable for I/O
		assert(false);
	}
	else if (InRange(address, 0xFF80, 0xFFFF))
	{
		// Internal RAM
		// todo ?
		Memory::StoreU8(address, val);
	}
	else
	{
		assert(address == 0xFFFF);
		// Interrupt Enable Register
		Memory::StoreU8(address, val);
	}
}

