#include "cpu.h"
#include "bootrom.h"

int main()
{
	BootRom::LoadFromDisk();
	while (true)
	{
		CPU::step();
	}
	return 0;
}