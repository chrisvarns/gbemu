#include "bootrom.h"
#include <fstream>
#include <assert.h>

u8 BootRom::bootrom[0x0100];

u8 BootRom::LoadU8(u16 address)
{
	assert(address < 0x0100);
	return bootrom[address];
}

void BootRom::LoadFromDisk()
{
	std::ifstream file;
	file.open("assets\\DMG_ROM.bin", std::ifstream::binary);
	assert(file);
	file.read((char*)bootrom, 256);
	file.close();
}