#include "cartridge.h"

#include "constants.h"
#include "memory.h"
#include "utils.h"

#include <fstream>
#include <assert.h>

extern u8 memory[0x10000];

std::string Cartridge::rom_path = "";
u8* rom_data = nullptr;
int rom_size;

void Cartridge::LoadGameRom()
{
	assert(!rom_path.empty() && "Specify \"-gamerom something\" on cmdline");

	// Load in the raw file
	{
		std::ifstream file;
		file.open(rom_path, std::iostream::in | std::iostream::binary | std::iostream::ate);
		assert(file && "Can't find gamerom");

		rom_size = static_cast<int>(file.tellg());
		assert(rom_size > 0);
		rom_data = new u8[rom_size];
		file.seekg(0, std::iostream::beg);
		file.read((char*)rom_data, rom_size);
		file.close();
	}

	// Pull out the various metadata values we need to know
	// todo
}

u8 Cartridge::LoadU8(u16 address)
{
	assert(address < rom_size);
	return rom_data[address];
}

void Cartridge::StoreU8(u16 address, u8 val)
{
	assert(false);
	if (InRange(address, AddressRegion::ROMBANK_STATIC_START, AddressRegion::ROMBANK_STATIC_END))
	{
		// TODO RAM Bank Enable
	}
	else if (InRange(address, AddressRegion::ROMBANK_SELECT_START, AddressRegion::ROMBANK_SELECT_END))
	{
		// TODO ROM Bank Select
	}
	else if (InRange(address, AddressRegion::RAMBANK_SELECT_START, AddressRegion::RAMBANK_SELECT_END))
	{
		// TODO RAM Bank Select
	}
	else if (InRange(address, AddressRegion::MBC1_SELECT_START, AddressRegion::MBC1_SELECT_END))
	{
		// TODO MBC1 ROM/RAM Select
	}
}