#include "cartridge.h"
#include "memory.h"

#include <fstream>
#include <assert.h>

extern u8 memory[0x10000];

std::string Cartridge::rom_path = "";

void Cartridge::LoadGameRom()
{
	assert(!rom_path.empty() && "Specify \"-gamerom something\" on cmdline");
	std::ifstream file;
	file.open(rom_path, std::iostream::binary);
	assert(file && "Can't find gamerom");
	file.read((char*)memory, 0x8000);
	file.close();
}