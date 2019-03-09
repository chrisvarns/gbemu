#include <string.h>

#include "cpu.h"
#include "bootrom.h"
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
	while (true)
	{
		CPU::step();
	}
	return 0;
}