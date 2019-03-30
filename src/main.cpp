#include <string.h>

#include "cpu.h"
#include "ppu.h"
#include "bootrom.h"
#include "constants.h"
#include "memory.h"

void ParseArgs(int argc, char** argv)
{
	for (int i = 0; i < argc;)
	{
		std::string arg = argv[i++];
		if (arg == "-gamerom")
		{
			Memory::gameromPath = argv[i++];
		}
	}
}

int main(int argc, char** argv)
{
	ParseArgs(argc, argv);
	Memory::Init();
	BootRom::LoadFromDisk();
	Memory::LoadGameRom();
	int clock = 0;
	while (true)
	{
		if ((clock % 4) == 0)
		{
			CPU::Step();
		}
		PPU::Step();
		clock += 2;
	}
	return 0;
}

// TODO LIST
// 0xFF00 .. 0xFFFF Special Registers, set values on reset?
// Cartridge ROM bank switching, for titles that use it.
// Cartridge RAM bank switching, for titles that use it. Similar to above.
// Save RAM banks to disk, for titles with battery backed RAM banks.
// Interrupts!