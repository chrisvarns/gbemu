#pragma once
#include "types.h"

const int gb_width = 160;
const int gb_height = 144;
const int bytes_per_pixel = 4;
const int total_gb_display_bytes = gb_width * gb_height * bytes_per_pixel;

enum class SpecialRegister : u16
{
	INTERRUPT_FLAG = 0xFF0F,
	SOUND_NR11 = 0xFF11,
	SOUND_NR12 = 0xFF12,
	SOUND_NR13 = 0xFF13,
	SOUND_NR14 = 0xFF14,
	SOUND_NR50 = 0xFF24,
	SOUND_NR51 = 0xFF25,
	SOUND_NR52 = 0xFF26,
	VIDEO_LCD_CONTROL = 0xFF40,
	VIDEO_LCD_STATUS = 0xFF41,
	VIDEO_SCROLLY = 0xFF42,
	VIDEO_SCROLLX = 0xFF43,
	VIDEO_CURRENT_SCANLINE = 0xFF44,
	VIDEO_BG_PALETTE = 0xFF47,
	VIDEO_SPRITE0_PALETTE = 0xFF48,
	VIDEO_SPRITE1_PALETTE = 0xFF49,
	BOOTROM_SWITCH = 0xFF50,
	INTERRUPT_ENABLE = 0xFFFF,
};

enum class AddressRegion : u16
{
	BOOTROM_START = 0x0000,
	BOOTROM_END = 0x0100,
	ROMBANK_STATIC_START = 0x0000,
	INTERRUPT_VECTOR_START = 0x0040,
	ROMBANK_STATIC_END = 0x4000,
	ROMBANK_SWITCHABLE_START = 0x4000,
	ROMBANK_SWITCHABLE_END = 0x8000,
	VRAM_START = 0x8000,
	VRAM_END = 0xA000,
	RAMBANK_SWITCHABLE_START = 0xA000,
	RAMBANK_SWITCHABLE_END = 0xC000,
	RAMBANK_INTERNAL_START = 0xC000,
	RAMBANK_INTERNAL_END = 0xE000,
	RAMBANK_INTERNAL_ECHO_START = 0xE000,
	RAMBANK_INTERNAL_ECHO_END = 0xFE00,
	OAM_START = 0xFE00,
	OAM_END = 0xFEA0,
	IO_START = 0xFF00,
	IO_END = 0xFF80,
	ZEROPAGE_START = 0xFF80,
	ZEROPAGE_END = 0xFFFF,

	SELECT_START = 0x0000,
	RAMBANK_ENABLE_START = 0x0000,
	RAMBANK_ENABLE_END = 0x2000,
	ROMBANK_SELECT_START = 0x2000,
	ROMBANK_SELECT_END = 0x4000,
	RAMBANK_SELECT_START = 0x4000,
	RAMBANK_SELECT_END = 0x6000,
	MBC1_SELECT_START = 0x6000,
	MBC1_SELECT_END = 0x8000,
	SELECT_END = 0x8000,

	// LCD control register controlled start addresses. Mode refers to whether relevant LCDC bit is set
	BACKGROUND_TILE_MAP_MODE_0_START = 0x9800,
	BACKGROUND_TILE_MAP_MODE_0_END = 0x9C00,
	BACKGROUND_TILE_MAP_MODE_1_START = 0x9C00,
	BACKGROUND_TILE_MAP_MODE_1_END = 0xA000,
	TILE_PATTERN_TABLE_MODE_0_START = 0x8800,
	TILE_PATTERN_TABLE_MODE_0_END = 0x9800,
	TILE_PATTERN_TABLE_MODE_1_START = 0x8000,
	TILE_PATTERN_TABLE_MODE_1_END = 0x9000,
	TILE_PATTERN_TABLE_MODE_0_ELEMENT_0 = 0x9000,
	TILE_PATTERN_TABLE_MODE_1_ELEMENT_0 = 0x8000,

	VIDEO_REGISTER_BEGIN = (u16)SpecialRegister::VIDEO_LCD_CONTROL,
	VIDEO_REGISTER_END = (u16)SpecialRegister::VIDEO_SPRITE1_PALETTE,
};

// VIDEO_LCD_CONTROL 0xFF40
enum class LCD_CONTROL_FLAGS : u8
{
	POWER = 0x80,
	WINDOW_MAP_ADDR = 0x40,
	WINDOW_DISPLAY = 0x20,
	TILE_PATTERN_TABLE_ADDR = 0x10,
	BACKGROUND_MAP_ADDR = 0x08,
	SPRITE_SIZE = 0x04,
	COLOR_0_WINDOW_TRANSPARENCY = 0x02,
	BACKGROUND_DISPLAY = 0x01,
};

// VIDEO_LCD_STATUS 0xFF41
enum class LCD_STATUS_FLAGS : u8
{
	// Read/Write, interrupts raised
	INTERRUPT_LY_COINCIDENCE = 0x40,
	INTERRUPT_MODE_OAM = 0x20,
	INTERRUPT_MODE_VBLANK = 0x10,
	INTERRUPT_MODE_HBLANK = 0x08,

	// Read only, current state of LCD controller
	CURRENT_COINCIDENCE = 0x04,
	CURRENT_MODE_BITS = 0x03,

	CURRENT_MODE_TRANSFER = 0x03,
	CURRENT_MODE_OAM = 0x02,
	CURRENT_MODE_VBLANK = 0x01,
	CURRENT_MODE_HBLANK = 0x00,
};

// This is used both for INTERRUPT_ENABLE and INTERRUPT_FLAG registers
enum class INTERRUPT_FLAGS : u8
{
	JOYPAD = 0x10,
	SERIAL = 0x08,
	TIMER = 0x04,
	LCD_STAT = 0x02,
	VBLANK = 0x01,

	ANY = 0x1F
};