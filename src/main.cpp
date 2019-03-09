#include "cpu.h"
#include "bootrom.h"
#include "memory.h"

int main()
{
	Memory::Init();
	BootRom::LoadFromDisk();
	while (true)
	{
		CPU::step();
	}
	return 0;
}