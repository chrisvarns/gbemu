#include "constants.h"
#include "main.h"
#include "memory.h"
#include "ppu.h"
#include "utils.h"

#include <cassert>
#include <queue>

enum class PPU_STAGE
{
	DISABLED,
	OAM_SEARCH,
	PIXEL_TRANSFER,
	HBLANK,
	VBLANK
};

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

enum class FIFO_MODE
{
	DISABLED,
	ENABLED,
};

enum class FETCH_MODE
{
	DISABLED,
	BACKGROUND,
	SPRITE,
};

enum class FETCH_STAGE
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

const int DISPLAY_WIDTH = 160;
const int DISPLAY_HEIGHT = 144;
const int PIXEL_TRANSFER_START_CYCLE = 20;
const int VBLANK_START_LINE = 144;
const int NUM_LINES_TOTAL = 154;
const int NUM_LINE_CYCLES = 114 * 2;
const int BACKGROUND_MAP_NUM_TILE_X = 32;
const int BACKGROUND_MAP_NUM_TILE_Y = 32;
const int TILE_SIZE_BYTES = 16;

static PPU_STAGE ppu_stage = PPU_STAGE::DISABLED;
static int current_h_cycle = -1;

static FIFO_MODE fifo_mode = FIFO_MODE::DISABLED;
static std::queue<FifoPixel> fifo_queue;
static u8 fifo_pixels_written_out = 0;

static FETCH_MODE fetch_mode = FETCH_MODE::DISABLED;
static FETCH_STAGE fetch_stage = FETCH_STAGE::READ_TILE_NUMBER;
static u16 fetch_source_address;
static u16 fetch_tile_pattern_table_address;
static u8 fetch_tile_number;
static u8 fetch_tile_data_0;
static u8 fetch_tile_data_1;

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

	fifo_mode = FIFO_MODE::DISABLED;
	fetch_mode = FETCH_MODE::DISABLED;
	fifo_queue.empty();

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

void WritePixel(FifoPixel pixel)
{
	// todo apply the pallette
	*sdl_pixels_write++ = (current_h_cycle % 3) == 0 ? 255 : 0;
	*sdl_pixels_write++ = (current_h_cycle % 3) == 1 ? 255 : 0;
	*sdl_pixels_write++ = (current_h_cycle % 3) == 2 ? 255 : 0;
	*sdl_pixels_write++ = 255;
}

void TriggerHBlank()
{
	ppu_stage = PPU_STAGE::HBLANK;
	fetch_mode = FETCH_MODE::DISABLED;
	fifo_mode = FIFO_MODE::DISABLED;
	// todo trigger interrupt on CPU
}

void StepFifo()
{
	if (fifo_mode == FIFO_MODE::DISABLED)
	{
		return;
	}

	if (fifo_queue.size() > 8)
	{
		auto pixel = fifo_queue.front();
		fifo_queue.pop();

		WritePixel(pixel);
		fifo_pixels_written_out++;
		if (fifo_pixels_written_out == DISPLAY_WIDTH)
		{
			TriggerHBlank();
		}
	}
}

int CurrentPixelLine()
{
	u8 ly_reg = Memory::LoadU8((u16)SpecialRegister::VIDEO_LY);
	return ly_reg;
}

bool IsTilePatternTableMode1()
{
	u8 lcdc_reg = Memory::LoadU8((u16)SpecialRegister::VIDEO_LCDC);
	return lcdc_reg & (u8)LCD_CONTROL_FLAGS::TILE_PATTERN_TABLE_ADDR;
}

u16 GetTileAddress()
{
	auto tile_data_row_index = CurrentPixelLine() % 8; // todo scroll y? presumably yes
	u16 tile_address;
	if (IsTilePatternTableMode1())
	{
		auto tile_address_offset = TILE_SIZE_BYTES * fetch_tile_number;
		tile_address = u16(AddressRegion::TILE_PATTERN_TABLE_MODE_1_ELEMENT_0) + tile_address_offset;
	}
	else
	{
		s8 signed_fetch_tile_number = static_cast<s8>(fetch_tile_number);
		s16 tile_address_offset = fetch_tile_number * TILE_SIZE_BYTES;
		tile_address = u16(AddressRegion::TILE_PATTERN_TABLE_MODE_0_ELEMENT_0) + tile_address_offset;
	}
	return tile_address;
}

void StepFetch()
{
	switch (fetch_mode)
	{
	case FETCH_MODE::BACKGROUND:
	{
		switch (fetch_stage)
		{
		case FETCH_STAGE::READ_TILE_NUMBER:
		{
			auto background_map_horizontal_index = (current_h_cycle - PIXEL_TRANSFER_START_CYCLE) / 8; // todo scroll x
			auto background_map_vertical_index = CurrentPixelLine() / 8; // todo scroll y
			auto tile_number_address_offset = background_map_vertical_index * BACKGROUND_MAP_NUM_TILE_X + background_map_horizontal_index;
			u16 tile_number_address = fetch_source_address + tile_number_address_offset;
			fetch_tile_number = Memory::LoadU8(tile_number_address);
		}
		break;
		case FETCH_STAGE::READ_DATA_ZERO:
		{
			s16 tile_address = GetTileAddress();
			fetch_tile_data_0 = Memory::LoadU8(tile_address);
		}
		break;
		case FETCH_STAGE::READ_DATA_ONE:
		{
			s16 tile_address = GetTileAddress();
			fetch_tile_data_1 = Memory::LoadU8(tile_address + 1);
		}
		break;
		case FETCH_STAGE::READ_DATA_ONE_IDLE:
		{
			if (fifo_queue.size() == 8)
			{
				goto label_write_to_fifo;
			}
		}
		break;
		case FETCH_STAGE::WRITE_TO_FIFO:
		{
		label_write_to_fifo:
			assert(fifo_queue.size() == 0 || fifo_queue.size() == 8);
			// todo move pixels into fifo
		}
		break;
		}

		fetch_stage = static_cast<FETCH_STAGE>(static_cast<int>(fetch_stage)+1);
	}
	break;
	}
}

u16 GetTilePatternTableStartAddr()
{
	return IsTilePatternTableMode1() ? (u16)AddressRegion::BACKGROUND_TILE_MAP_MODE_1_START : (u16)AddressRegion::BACKGROUND_TILE_MAP_MODE_0_START;
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

void StartPixelTransfer()
{
	fifo_pixels_written_out = 0;
	fifo_queue.empty();
	fifo_mode = FIFO_MODE::ENABLED;

	fetch_source_address = GetBackgroundMapStartAddr();
	fetch_tile_pattern_table_address = GetTilePatternTableStartAddr();
	fetch_stage = FETCH_STAGE::READ_TILE_NUMBER;
	fetch_mode = FETCH_MODE::BACKGROUND;
}

void PPU::Step()
{
	if (!IsPpuEnabled())
	{
		if (ppu_stage != PPU_STAGE::DISABLED)
		{
			Disable();
			ppu_stage = PPU_STAGE::DISABLED;
		}
		return;
	}

	// Update current h/v values
	u8 ly_reg = Memory::LoadU8((u16)SpecialRegister::VIDEO_LY);
	current_h_cycle++;
	if (current_h_cycle == NUM_LINE_CYCLES) // end of line
	{
		current_h_cycle = 0;
		ly_reg++;
		if (ly_reg == NUM_LINES_TOTAL) // end of frame
		{
			ly_reg = 0;
		}
		Memory::StoreU8((u16)SpecialRegister::VIDEO_LY, ly_reg);
	}

	// Set state
	if (InRange(ly_reg, VBLANK_START_LINE, NUM_LINES_TOTAL))
	{
		ppu_stage = PPU_STAGE::VBLANK;
	}
	else if (current_h_cycle == 0)
	{
		ppu_stage = PPU_STAGE::OAM_SEARCH;
	}
	else if (current_h_cycle == PIXEL_TRANSFER_START_CYCLE)
	{
		ppu_stage = PPU_STAGE::PIXEL_TRANSFER;
		StartPixelTransfer();
	}
	// HBlank triggered by the fifo once all pixels are scanned out

	switch (ppu_stage)
	{
	case PPU_STAGE::OAM_SEARCH:
	{
		//todo
	}
	break;
	case PPU_STAGE::PIXEL_TRANSFER:
	{
		StepFifo();
		if (ppu_stage == PPU_STAGE::PIXEL_TRANSFER) // we might have gone to hblank if the fifo wrote out
		{
			StepFetch();
		}
	}
	break;

	}
}