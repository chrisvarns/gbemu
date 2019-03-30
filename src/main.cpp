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
			Memory::gamerom_path = argv[i++];
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