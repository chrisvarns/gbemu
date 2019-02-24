#include "cpu.h"
#include "memory.h"

int main()
{
	Memory::LoadBootRom();
	while (true)
	{
		CPU::step();
	}
	return 0;
}