#include "bootrom.h"
#include "bus.h"
#include "cartridge.h"
#include "timer.h"
#include "constants.h"
#include "memory.h"
#include "utils.h"
#include <assert.h>

namespace Bus
{
	// Special store used by PPU to allow writing to PPU registers
	void StoreU8_PPU(u16 address, u8 val);
}

inline u8 HandleIORead(u16 address)
{
	SpecialRegister special_register = SpecialRegister(address);
	switch (special_register)
	{
	case SpecialRegister::DIV:	return Timer::R_DIV();
	case SpecialRegister::TIMA: return Timer::R_TIMA();
	case SpecialRegister::TMA:	return Timer::R_TMA();
	case SpecialRegister::TAC:	return Timer::R_TAC();

	case SpecialRegister::INTERRUPT_FLAG:
	case SpecialRegister::SOUND_NR11:
	case SpecialRegister::SOUND_NR12:
	case SpecialRegister::SOUND_NR13:
	case SpecialRegister::SOUND_NR14:
	case SpecialRegister::SOUND_NR50:
	case SpecialRegister::SOUND_NR51:
	case SpecialRegister::SOUND_NR52:
	case SpecialRegister::VIDEO_LCD_CONTROL:
	case SpecialRegister::VIDEO_LCD_STATUS:
	case SpecialRegister::VIDEO_SCROLLY:
	case SpecialRegister::VIDEO_SCROLLX:
	case SpecialRegister::VIDEO_CURRENT_SCANLINE:
	case SpecialRegister::VIDEO_BG_PALETTE:
	case SpecialRegister::VIDEO_SPRITE0_PALETTE:
	case SpecialRegister::VIDEO_SPRITE1_PALETTE:
	{
		return Memory::LoadU8(address);
	}
	}
	assert(false);
	return 0;
}

inline void HandleIOWrite(u16 address, u8 val)
{
	SpecialRegister special_register = SpecialRegister(address);
	switch (special_register)
	{
	case SpecialRegister::DIV:	Timer::W_DIV(val);	break;
	case SpecialRegister::TIMA: Timer::W_TIMA(val); break;
	case SpecialRegister::TMA:	Timer::W_TMA(val);	break;
	case SpecialRegister::TAC:	Timer::W_TAC(val);	break;

	case SpecialRegister::INTERRUPT_FLAG:
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
	case SpecialRegister::VIDEO_LCD_CONTROL:
	case SpecialRegister::VIDEO_LCD_STATUS:
	case SpecialRegister::VIDEO_SCROLLY:
	case SpecialRegister::VIDEO_SCROLLX:
	case SpecialRegister::VIDEO_BG_PALETTE:
	case SpecialRegister::BOOTROM_SWITCH:
	{
		Memory::StoreU8(address, val);
		break;
	}
	case SpecialRegister::VIDEO_CURRENT_SCANLINE: // Should be calling StoreU8_PPU
	default:
		assert(false);
	}
}

u8 Bus::LoadU8(u16 address)
{
	if (InRange(address, AddressRegion::BOOTROM_START, AddressRegion::BOOTROM_END))
	{
		// Either bootrom or cart rom
		if (Memory::LoadU8((u16)SpecialRegister::BOOTROM_SWITCH))
		{
			// 16KB Cartridge ROM bank #0
			return Cartridge::LoadU8(address);
		}
		else
		{
			// Boot ROM
			return BootRom::LoadU8(address);
		}
	}
	else if (InRange(address, AddressRegion::ROMBANK_STATIC_START, AddressRegion::ROMBANK_STATIC_END))
	{
		// 16KB Cartridge ROM bank #0
		return Cartridge::LoadU8(address);
	}
	else if (InRange(address, AddressRegion::ROMBANK_SWITCHABLE_START, AddressRegion::ROMBANK_SWITCHABLE_END))
	{
		// 16KB Cartridge ROM bank (switchable via MBC, some titles)
		return Cartridge::LoadU8(address);
	}
	else if (InRange(address, AddressRegion::VRAM_START, AddressRegion::VRAM_END))
	{
		// 8KB Video RAM
		return Memory::LoadU8(address); // todo go through PPU
	}
	else if (InRange(address, AddressRegion::RAMBANK_SWITCHABLE_START, AddressRegion::RAMBANK_SWITCHABLE_END))
	{
		// 8KB switchable RAM bank
		// todo handle RAM bank switching
		return Memory::LoadU8(address);
	}
	else if (InRange(address, AddressRegion::RAMBANK_INTERNAL_START, AddressRegion::RAMBANK_INTERNAL_END))
	{
		// 8KB Internal RAM
		return Memory::LoadU8(address);
	}
	else if (InRange(address, AddressRegion::RAMBANK_INTERNAL_ECHO_START, AddressRegion::RAMBANK_INTERNAL_ECHO_END))
	{
		// echo of 8KB Internal RAM
		return Memory::LoadU8(address - ((u16)AddressRegion::RAMBANK_INTERNAL_ECHO_START - (u16)AddressRegion::RAMBANK_INTERNAL_START));
	}
	else if (InRange(address, AddressRegion::OAM_START, AddressRegion::OAM_END))
	{
		// Sprite Attrib Memory (OAM)
		// todo
		assert(false);
		return 0;
	}
	else if (InRange(address, AddressRegion::OAM_END, AddressRegion::IO_START))
	{
		// Unusable
		assert(false);
		return 0;
	}
	else if (InRange(address, AddressRegion::IO_START, AddressRegion::IO_END))
	{
		// I/O ports
		return HandleIORead(address);
	}
	else if (InRange(address, AddressRegion::ZEROPAGE_START, AddressRegion::ZEROPAGE_END))
	{
		// Internal RAM
		return Memory::LoadU8(address);
	}
	else
	{
		assert(address == (u16)SpecialRegister::INTERRUPT_ENABLE);
		// Interrupt Enable Register
		return Memory::LoadU8(address);
	}
}

void Bus::StoreU8(u16 address, u8 val)
{
	if (InRange(address, AddressRegion::SELECT_START, AddressRegion::SELECT_END))
	{
		// 32KB ROM space
		Cartridge::StoreU8(address, val);
	}
	else if (InRange(address, AddressRegion::VRAM_START, AddressRegion::VRAM_END))
	{
		// 8KB Video RAM
		Memory::StoreU8(address, val);
	}
	else if (InRange(address, AddressRegion::RAMBANK_SWITCHABLE_START, AddressRegion::RAMBANK_SWITCHABLE_END))
	{
		// 8KB switchable RAM bank
		// todo handle RAM bank switching
		Memory::StoreU8(address, val);
	}
	else if (InRange(address, AddressRegion::RAMBANK_INTERNAL_START, AddressRegion::RAMBANK_INTERNAL_END))
	{
		// 8KB Internal RAM
		Memory::StoreU8(address, val);
	}
	else if (InRange(address, AddressRegion::RAMBANK_INTERNAL_ECHO_START, AddressRegion::RAMBANK_INTERNAL_ECHO_END))
	{
		// echo of 8KB Internal RAM
		Memory::StoreU8(address - ((u16)AddressRegion::RAMBANK_INTERNAL_ECHO_START - (u16)AddressRegion::RAMBANK_INTERNAL_START), val);
	}
	else if (InRange(address, AddressRegion::OAM_START, AddressRegion::OAM_END))
	{
		// Sprite Attrib Memory (OAM)
		// todo
		assert(false);
	}
	else if (InRange(address, AddressRegion::OAM_END, AddressRegion::IO_START))
	{
		// Unusable
		assert(false);
	}
	else if (InRange(address, AddressRegion::IO_START, AddressRegion::IO_END))
	{
		// I/O ports
		HandleIOWrite(address, val);
	}
	else if (InRange(address, AddressRegion::ZEROPAGE_START, AddressRegion::ZEROPAGE_END))
	{
		// Internal RAM
		Memory::StoreU8(address, val);
	}
	else
	{
		assert(address == (u16)SpecialRegister::INTERRUPT_ENABLE);
		// Interrupt Enable Register
		Memory::StoreU8(address, val);
	}
}

void Bus::StoreU8_PPU(u16 address, u8 val)
{
	assert(InRange(address, AddressRegion::VIDEO_REGISTER_BEGIN, AddressRegion::VIDEO_REGISTER_END));
	Memory::StoreU8(address, val);
}

