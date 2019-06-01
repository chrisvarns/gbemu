#include "constants.h"
#include "main.h"
#include "memory.h"
#include "ppu.h"
#include "utils.h"

#include <cassert>
#include <queue>

enum class PPU_STATE
{
	DISABLED,
	OAM_SEARCH,
	PIXEL_TRANSFER,
	HBLANK,
	VBLANK
};

const int PIXEL_TRANSFER_START_CYCLE = 20;
const int VBLANK_START_LINE = 144;
const int LINE_LENGTH_NUM_CYCLES = 114 * 2;
const int NUM_LINES = 154;

static PPU_STATE ppu_state = PPU_STATE::DISABLED;
static int current_h_cycle = -1;

static SDL_Renderer* sdl_renderer = nullptr;
static SDL_Texture* sdl_texture = nullptr;

static void* sdl_pixels = nullptr;
static u8* sdl_pixels_write = nullptr;
static bool sdl_texture_locked = false;

void DrawDebugRed()
{	
	int unused_pitch;
	SDL_LockTexture(sdl_texture, nullptr, &sdl_pixels, &unused_pitch);
	sdl_texture_locked = true;
	sdl_pixels_write = (u8*)sdl_pixels;
	u8* pixel_end = sdl_pixels_write + total_gb_display_bytes;
	while (sdl_pixels_write != pixel_end)
	{
		*sdl_pixels_write++ = 255;
		*sdl_pixels_write++ = 0;
		*sdl_pixels_write++ = 0;
		*sdl_pixels_write++ = 255;
	}

	SDL_UnlockTexture(sdl_texture);
	sdl_texture_locked = false;
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

	// Clear the screen
	if (!sdl_texture_locked)
	{
		int unused_pitch;
		SDL_LockTexture(sdl_texture, nullptr, &sdl_pixels, &unused_pitch);
		sdl_texture_locked = true;
	}
	memset(sdl_pixels, 0, total_gb_display_bytes);
	SDL_UnlockTexture(sdl_texture);
	sdl_texture_locked = false;
	SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
	SDL_RenderPresent(sdl_renderer);
}

enum class PALLETTE_SOURCE
{
	BG,
	S0,
	S1,
};

struct FifoPixel
{
	u8 pallette_color;
	PALLETTE_SOURCE pallette_source;
};
void WritePixel(FifoPixel pixel)
{
	// todo apply the pallette
	*sdl_pixels_write++ = (current_h_cycle % 3) == 0 ? 255 : 0;
	*sdl_pixels_write++ = (current_h_cycle % 3) == 1 ? 255 : 0;
	*sdl_pixels_write++ = (current_h_cycle % 3) == 2 ? 255 : 0;
	*sdl_pixels_write++ = 255;
}

static std::queue<FifoPixel> pixel_fifo;
void StepFifo()
{
	if (pixel_fifo.size() > 8)
	{
		auto pixel = pixel_fifo.front();
		pixel_fifo.pop();

		WritePixel(pixel);
	}
	// todo start hblank when we've written out a line.
}

enum class FETCH_MODE
{
	BACKGROUND,
	SPRITE,
};
static FETCH_MODE fetch_mode = FETCH_MODE::BACKGROUND;
enum class FETCH_STATE
{
	READ_TILE_NUMBER = 0,
	READ_TILE_NUMBER_IDLE,
	READ_DATA_ZERO,
	READ_DATA_ZERO_IDLE,
	READ_DATA_ONE,
	READ_DATA_ONE_IDLE,
	WRITE_TO_FIFO_IDLE,
	WRITE_TO_FIFO
};
static FETCH_STATE fetch_state = FETCH_STATE::READ_TILE_NUMBER;
static u16 fetch_source_address;
void StepFetch()
{
	switch (fetch_mode)
	{
	case FETCH_MODE::BACKGROUND:
	{
		switch (fetch_state)
		{
		case FETCH_STATE::READ_TILE_NUMBER:
		{

		}
		break;
		case FETCH_STATE::READ_DATA_ZERO:
		{
		}
		break;
		case FETCH_STATE::READ_DATA_ONE:
		{
		}
		break;
		case FETCH_STATE::READ_DATA_ONE_IDLE:
		{
			if (pixel_fifo.size() == 8)
			{
				goto label_write_to_fifo;
			}
		}
		break;
		case FETCH_STATE::WRITE_TO_FIFO:
		{
		label_write_to_fifo:
			void(0);
			//assert(pixel_fifo.size() == 8);
			// todo move pixels into fifo
		}
		break;
		}

		fetch_state = static_cast<FETCH_STATE>(static_cast<int>(fetch_state)+1);
	}
	break;
	}
}

u16 GetBackgroundMapStartAddr()
{
	u8 lcdc_reg = Memory::LoadU8((u16)SpecialRegister::VIDEO_LCDC);
	const bool mode_1 = lcdc_reg & (u8)LCD_CONTROL_FLAGS::BACKGROUND_MAP_ADDR;
	return mode_1 ? (u16)AddressRegion::BACKGROUND_TILE_MAP_MODE_1_START : (u16)AddressRegion::BACKGROUND_TILE_MAP_MODE_0_START;
}

bool IsPpuEnabled()
{
	u8 lcdc_reg = Memory::LoadU8((u16)SpecialRegister::VIDEO_LCDC);
	return lcdc_reg & (u8)LCD_CONTROL_FLAGS::POWER;
}

void PPU::Step()
{
	if (!IsPpuEnabled())
	{
		if (ppu_state != PPU_STATE::DISABLED)
		{
			Disable();
			ppu_state = PPU_STATE::DISABLED;
		}
		return;
	}

	// Update current h/v values
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
	if (InRange(ly_reg, VBLANK_START_LINE, NUM_LINES))
	{
		ppu_state = PPU_STATE::VBLANK;
	}
	else if (current_h_cycle == 0)
	{
		ppu_state = PPU_STATE::OAM_SEARCH;
	}
	else if (current_h_cycle == PIXEL_TRANSFER_START_CYCLE)
	{
		ppu_state = PPU_STATE::PIXEL_TRANSFER;
		fetch_state = FETCH_STATE::READ_TILE_NUMBER;
		fetch_source_address = GetBackgroundMapStartAddr();
	}

	switch (ppu_state)
	{
	case PPU_STATE::OAM_SEARCH:
	{
		//todo
	}
	break;
	case PPU_STATE::PIXEL_TRANSFER:
	{
		StepFifo();
		StepFetch();
	}
	break;

	}
}