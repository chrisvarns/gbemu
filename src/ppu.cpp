#include "constants.h"
#include "main.h"
#include "memory.h"
#include "ppu.h"
#include "utils.h"

enum class LCDC_FLAGS : u8
{
	LCD_POWER = 0x80,
};

enum class PPU_STATE
{
	DISABLED,
	OAM_SEARCH,
	PIXEL_TRANSFER,
	HBLANK,
	VBLANK
};

const int PIXEL_TRANSFER_START = 20;
const int HBLANK_START = 63;
const int VBLANK_START = 144;
const int LINE_LENGTH_NUM_CYCLES = 114 * 2;
const int NUM_LINES = 154;

static PPU_STATE ppu_state = PPU_STATE::DISABLED;
static int current_h_cycle = -1;

static SDL_Renderer* sdl_renderer = nullptr;
static SDL_Texture* sdl_texture = nullptr;
static u8* sdl_pixels = nullptr;

void DrawDebugRed()
{
	void* pixels;
	int pitch;
	SDL_LockTexture(sdl_texture, nullptr, &pixels, &pitch);
	u8* pixel_write = (u8*)pixels;
	u8* pixel_end = pixel_write + (gb_width * gb_height * 4);
	while (pixel_write != pixel_end)
	{
		*pixel_write++ = 255;
		*pixel_write++ = 0;
		*pixel_write++ = 0;
		*pixel_write++ = 255;
	}

	SDL_UnlockTexture(sdl_texture);
	SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
	SDL_RenderPresent(sdl_renderer);
}

void PPU::Init()
{
	sdl_renderer = SDL_CreateRenderer(g_window, -1, 0);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);

	DrawDebugRed();
}

void Disable()
{
	current_h_cycle = -1;
	Memory::StoreU8((u16)SpecialRegister::VIDEO_LY, 0);
}

void PPU::Step()
{
	u8 lcdc_reg = Memory::LoadU8((u16)SpecialRegister::VIDEO_LCDC);
	if ((lcdc_reg & (u8)LCDC_FLAGS::LCD_POWER) == 0)
	{
		if (ppu_state != PPU_STATE::DISABLED)
		{
			Disable();
			ppu_state = PPU_STATE::DISABLED;
		}
		return;
	}

	// Set current h/v values
	u8 ly_reg = Memory::LoadU8((u16)SpecialRegister::VIDEO_LY);
	current_h_cycle++;
	if (current_h_cycle == LINE_LENGTH_NUM_CYCLES)
	{
		current_h_cycle = 0;
		ly_reg++;
		if (ly_reg == NUM_LINES)
		{
			ly_reg = 0;
		}
		Memory::StoreU8((u16)SpecialRegister::VIDEO_LY, ly_reg);
	}

	// Set state
	if (InRange(ly_reg, VBLANK_START, NUM_LINES))
	{
		ppu_state = PPU_STATE::VBLANK;
	}
	else if (current_h_cycle == 0)
	{
		ppu_state = PPU_STATE::OAM_SEARCH;
	}
	else if (current_h_cycle == PIXEL_TRANSFER_START)
	{
		ppu_state = PPU_STATE::PIXEL_TRANSFER;
	}
	else if (current_h_cycle == HBLANK_START)
	{
		ppu_state = PPU_STATE::HBLANK;
		// todo generate HBLANK interrupt
	}
}