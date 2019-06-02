#include <assert.h>
#include <string.h>

#include "bootrom.h"
#include "cartridge.h"
#include "constants.h"
#include "cpu.h"
#include "main.h"
#include "memory.h"
#include "ppu.h"

SDL_Window* g_window;

void ParseArgs(int argc, char** argv)
{
	for (int i = 0; i < argc;)
	{
		std::string arg = argv[i++];
		if (arg == "-gamerom")
		{
			Cartridge::rom_path = argv[i++];
		}
	}
}

int main(int argc, char** argv)
{
	ParseArgs(argc, argv);

	SDL_Init(SDL_INIT_VIDEO);
	const int res_multiplier = 4;
	g_window = SDL_CreateWindow(
		"GBEMU",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		gb_width * res_multiplier,
		gb_height * res_multiplier,
		SDL_WINDOW_OPENGL
	);
	assert(g_window);

	Memory::Init();
	BootRom::LoadFromDisk();
	Cartridge::LoadGameRom();
	PPU::Init();

	int clock = 0;
	while (true)
	{
		if ((clock % 4) == 0)
		{
			CPU::Step();
		}
		PPU::Step();
		clock++;
	}
	return 0;
}