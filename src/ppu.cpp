#include "constants.h"
#include "main.h"
#include "bus.h"
#include "ppu.h"
#include "utils.h"

#include <cassert>
#include <queue>

namespace Bus
{
	void StoreU8_PPU(u16 address, u8 val);
}

enum class PPU_STAGE
{
	DISABLED,
	OAM_SEARCH,
	PIXEL_TRANSFER,
	HBLANK,
	VBLANK
};

enum class PALETTE_TYPE
{
	BG = 0,
	S0,
	S1,
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
	READ_TILE_NUMBER_IDLE = 0,
	READ_TILE_NUMBER,
	READ_DATA_LSB_IDLE,
	READ_DATA_LSB,
	READ_DATA_MSB_IDLE,
	READ_DATA_MSB,
	WRITE_TO_FIFO_IDLE,
	WRITE_TO_FIFO,

	NUM_STAGES
};

struct FifoPixel
{
	PALETTE_TYPE palette;
	u8 color;
};

const int DISPLAY_WIDTH = 160;
const int DISPLAY_HEIGHT = 144;
const int PIXEL_TRANSFER_START_CYCLE = 20 * 4;
const int VBLANK_START_LINE = 144;
const int NUM_LINES_TOTAL = 154;
const int NUM_LINE_CYCLES = 114 * 4;
const int BACKGROUND_MAP_NUM_TILE_X = 32;
const int BACKGROUND_MAP_NUM_TILE_Y = 32;
const int TILE_SIZE_BYTES = 16;

static PPU_STAGE ppu_stage = PPU_STAGE::DISABLED;
static int current_h_cycle = -1;

static FIFO_MODE fifo_mode = FIFO_MODE::DISABLED;
static std::queue<FifoPixel> fifo_queue;
static u8 fifo_pixels_written_out = 0;

static FETCH_MODE fetch_mode = FETCH_MODE::DISABLED;
static FETCH_STAGE fetch_stage = (FETCH_STAGE)0;
static u16 fetch_source_address;
static u8 fetch_tile_number;
static u8 fetch_tile_data_low_bits;
static u8 fetch_tile_data_high_bits;

static SDL_Renderer* sdl_renderer = nullptr;
static SDL_Texture* sdl_texture = nullptr;
static u8* sdl_pixels = nullptr;
static u8* sdl_pixels_write = nullptr;
static bool sdl_texture_locked = false;
static int current_frame_index = 0;

void PresentBackBuffer()
{
	if (sdl_texture_locked)
	{
		SDL_UnlockTexture(sdl_texture);
		sdl_texture_locked = false;
	}
	SDL_RenderCopy(sdl_renderer, sdl_texture, nullptr, nullptr);
	SDL_RenderPresent(sdl_renderer);
}

void ClearToWhite()
{	
	if (!sdl_texture_locked)
	{
		int unused_pitch;
		SDL_LockTexture(sdl_texture, nullptr, (void**)&sdl_pixels, &unused_pitch);
		sdl_texture_locked = true;
	}
	
	memset((void*)sdl_pixels, 255, total_gb_display_bytes);

	PresentBackBuffer();
}

void PPU::Init()
{
	sdl_renderer = SDL_CreateRenderer(g_window, -1, 0);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);

	ClearToWhite();
}

void Disable()
{
	current_h_cycle = -1;
	Bus::StoreU8_PPU((u16)SpecialRegister::VIDEO_CURRENT_SCANLINE, 0);

	fifo_mode = FIFO_MODE::DISABLED;
	fetch_mode = FETCH_MODE::DISABLED;
	fifo_queue = {};

	ClearToWhite();
}

void WritePixel(FifoPixel fifo_pixel)
{
	u16 palette_address = (u16)SpecialRegister::VIDEO_BG_PALETTE + (u16)fifo_pixel.palette;
	u8 palette = Bus::LoadU8(palette_address);
	u8 color = (palette >> fifo_pixel.color * 2) & 0x03;

	u8 greyscale = 255 - (85 * color);
	*sdl_pixels_write++ = greyscale;
	*sdl_pixels_write++ = greyscale;
	*sdl_pixels_write++ = greyscale;
	*sdl_pixels_write++ = 255;

#if 0
	// Crosshair rendering, useful for isolating problem pixels
	// todo remove when we have hover-over pixel location
	const int x_coord = 104;
	const int y_coord = 71;
	if (GetCurrentLineIdx() == y_coord || fifo_pixels_written_out == x_coord)
	{
		*(sdl_pixels_write - 4) = 255;
		*(sdl_pixels_write - 3) = 0;
		*(sdl_pixels_write - 2) = 0;
	}
#endif

#if 0
	// Immediately present every pixel (useful when stepping)
	auto write_offset = sdl_pixels_write - sdl_pixels;
	PresentBackBuffer();
	if (!sdl_texture_locked)
	{
		int unused_pitch;
		SDL_LockTexture(sdl_texture, nullptr, (void**)&sdl_pixels, &unused_pitch);
		sdl_texture_locked = true;
	}
	sdl_pixels_write = sdl_pixels + write_offset;
#endif // 0
 }

void TriggerHBlank()
{
	ppu_stage = PPU_STAGE::HBLANK;
	fifo_mode = FIFO_MODE::DISABLED;
	fetch_mode = FETCH_MODE::DISABLED;

	// todo trigger interrupt on CPU
}

void TriggerVBlank()
{
	ppu_stage = PPU_STAGE::VBLANK;
	PresentBackBuffer();

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
	u8 scanline_reg = Bus::LoadU8((u16)SpecialRegister::VIDEO_CURRENT_SCANLINE);
	return scanline_reg;
}

bool IsTilePatternTableMode1()
{
	u8 lcd_control_reg = Bus::LoadU8((u16)SpecialRegister::VIDEO_LCD_CONTROL);
	return lcd_control_reg & (u8)LCD_CONTROL_FLAGS::TILE_PATTERN_TABLE_ADDR;
}

u16 GetTileAddress()
{
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
	auto row_in_tile = CurrentPixelLine() % 8;
	tile_address += row_in_tile * 2;
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
			const auto background_map_horizontal_index = (current_h_cycle - PIXEL_TRANSFER_START_CYCLE) / 8; // todo scroll x
			const auto background_map_vertical_index = CurrentPixelLine() / 8; // todo scroll y
			const auto tile_number_address_offset = background_map_vertical_index * BACKGROUND_MAP_NUM_TILE_X + background_map_horizontal_index;
			const u16 tile_number_address = fetch_source_address + tile_number_address_offset;
			fetch_tile_number = Bus::LoadU8(tile_number_address);
		}
		break;
		case FETCH_STAGE::READ_DATA_LSB:
		{
			const s16 tile_address = GetTileAddress();
			fetch_tile_data_low_bits = Bus::LoadU8(tile_address);
		}
		break;
		case FETCH_STAGE::READ_DATA_MSB:
		{
			const s16 tile_address = GetTileAddress();
			fetch_tile_data_high_bits = Bus::LoadU8(tile_address + 1);
			if (fifo_queue.size() > 8)
			{
				// Wait until WRITE_TO_FIFO as we are blocked from pushing out.
				break;
			}
			// The fifo has space so we immediately push out.
			fetch_stage = FETCH_STAGE::WRITE_TO_FIFO;

			// Pixel transfer takes minimum 43 1mhz cycles:
			//		40 to push out the 160 pixels (1 per 4mhz cycle)
			//		3 (12 4mhz cycles) for the fetch to populate the fifo before it can start writing out.
			// Note that the first 2 fetches are 12 4mhz cycles due to the fifo having space, but all subsequent fetches will be 16 4mhz cycles.
		}
		// Intentional fallthrough
		case FETCH_STAGE::WRITE_TO_FIFO:
		{
			assert(fifo_queue.size() == 0 || fifo_queue.size() == 8);
			for (int i = 7; i >= 0; --i)
			{
				FifoPixel fifo_pixel;
				fifo_pixel.palette = PALETTE_TYPE::BG;

				fifo_pixel.color = ((fetch_tile_data_high_bits >> i) & 0x01) << 1;
				fifo_pixel.color |= ((fetch_tile_data_low_bits >> i) & 0x01);
				fifo_queue.push(fifo_pixel);
			}
		}
		break;
		}
		fetch_stage = static_cast<FETCH_STAGE>((static_cast<int>(fetch_stage) + 1) % static_cast<int>(FETCH_STAGE::NUM_STAGES));
	}
	break;
	}
}

u16 GetBackgroundMapStartAddr()
{
	u8 lcdc_reg = Bus::LoadU8((u16)SpecialRegister::VIDEO_LCD_CONTROL);
	const bool mode_1 = lcdc_reg & (u8)LCD_CONTROL_FLAGS::BACKGROUND_MAP_ADDR;
	return mode_1 ? (u16)AddressRegion::BACKGROUND_TILE_MAP_MODE_1_START : (u16)AddressRegion::BACKGROUND_TILE_MAP_MODE_0_START;
}

bool IsPpuEnabled()
{
	u8 lcdc_reg = Bus::LoadU8((u16)SpecialRegister::VIDEO_LCD_CONTROL);
	return lcdc_reg & (u8)LCD_CONTROL_FLAGS::POWER;
}

void StartPixelTransfer()
{
	ppu_stage = PPU_STAGE::PIXEL_TRANSFER;

	fifo_pixels_written_out = 0;
	fifo_queue = {};
	fifo_mode = FIFO_MODE::ENABLED;

	fetch_source_address = GetBackgroundMapStartAddr();
	fetch_stage = (FETCH_STAGE)0;
	fetch_mode = FETCH_MODE::BACKGROUND;
}

void StartNewFrame()
{
	if (!sdl_texture_locked)
	{
		int unused_pitch;
		SDL_LockTexture(sdl_texture, nullptr, (void**)&sdl_pixels, &unused_pitch);
		sdl_texture_locked = true;
	}
	sdl_pixels_write = sdl_pixels;

	SDL_Log("Frame %d", ++current_frame_index);
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
	u8 ly_reg = Bus::LoadU8((u16)SpecialRegister::VIDEO_CURRENT_SCANLINE);
	current_h_cycle++;
	if (current_h_cycle == NUM_LINE_CYCLES) // end of line
	{
		current_h_cycle = 0;
		ly_reg++;
		if (ly_reg == NUM_LINES_TOTAL) // end of frame
		{
			ly_reg = 0;
		}
		Bus::StoreU8_PPU((u16)SpecialRegister::VIDEO_CURRENT_SCANLINE, ly_reg);
	}

	// Set state
	if (ly_reg == 0 && current_h_cycle == 0)
	{
		StartNewFrame();
	}
	else if (InRange(ly_reg, VBLANK_START_LINE, NUM_LINES_TOTAL))
	{
		if (ly_reg == VBLANK_START_LINE && current_h_cycle == 0)
		{
			TriggerVBlank();
		}
	}
	else if (current_h_cycle == 0)
	{
		ppu_stage = PPU_STAGE::OAM_SEARCH;
	}
	else if (current_h_cycle == PIXEL_TRANSFER_START_CYCLE)
	{
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
		StepFetch();
	}
	break;

	}
}