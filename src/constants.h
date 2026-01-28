#ifndef CONSTANTS_H
#define CONSTANTS_H

// Screen dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Sprite data configuration
#define SPRITE_DATA_START       0x0000U // Starting address in XRAM for sprite data
#define REDRACER_DATA           (SPRITE_DATA_START) // Address for RedRacer sprite
#define REDRACER_DATA_SIZE      0x0800U // Size of the RedRacer sprite data - 4 x 16x16 tiles

#define SPRITE_DATA_END         (SPRITE_DATA_START + REDRACER_DATA_SIZE)

// XRAM memory layout:
// 0x0000-0x0800: Car sprite data (4 cars × 512 bytes each)
// 0x0800-0x0850: Configuration structs (player + 3 AI cars)
// 0x0850-0x1450: Tile map (3072 bytes)
// 0x1450-0x3370: Tile graphics

// Tile data configuration
#define TRACK_MAP_ADDR          0x0850U // Address for track map data in XRAM
#define TRACK_DATA              0x1450U // Address for track tile data in XRAM (Re-used for tiles.bin)
#define TRACK_DATA_SIZE         0x2000U // Size of track tile data (8192 bytes = 256 tiles * 32 bytes)
#define TRACK_DATA_END          (TRACK_DATA + TRACK_DATA_SIZE)

#define TITLE_MAP_ADDR          0x4284U // Address for title map data in XRAM (Aligned)
#define TITLE_DATA              0x4E84U // Address for title tile data in XRAM (Aligned)
#define TITLE_DATA_SIZE         0x2000U // Size of title tile data (8192 bytes = 256 tiles * 32 bytes)
#define TITLE_DATA_END          (TITLE_DATA + TITLE_DATA_SIZE)

// 5. Keyboard, Gamepad and Sound
// -------------------------------------------------------------------------
#define OPL_ADDR        0xFE00  // OPL2 Address port
#define PALETTE_ADDR    0xFF58  // XRAM address for palette data
#define GAMEPAD_INPUT   0xFF78  // XRAM address for gamepad data
#define KEYBOARD_INPUT  0xFFA0  // XRAM address for keyboard data
#define PSG_XRAM_ADDR   0xFFC0  // PSG memory location (must match sound.c)

extern unsigned REDRACER_CONFIG;  // RedRacer Sprite Configuration
extern unsigned TRACK_CONFIG;     // Track Tilemap Configuration
extern unsigned TEXT_CONFIG;      // Text Overlay Configuration
extern unsigned text_message_addr; // Start address for text messages in XRAM
extern unsigned TITLE_MAP_START;  // Start address for title map data in XRAM

// Track tile map
#define TRACK_MAP_WIDTH_TILES   64
#define TRACK_MAP_HEIGHT_TILES  48
#define MAP_BYTES_PER_TILE       1
#define TRACK_MAP_SIZE          (TRACK_MAP_WIDTH_TILES * TRACK_MAP_HEIGHT_TILES * MAP_BYTES_PER_TILE)

// Title tile map
#define TITLE_MAP_WIDTH_TILES   64
#define TITLE_MAP_HEIGHT_TILES  48
#define TITLE_NUMBER_OF_TILES   (TITLE_MAP_WIDTH_TILES * TITLE_MAP_HEIGHT_TILES)
#define TITLE_MAP_SIZE          (TITLE_MAP_WIDTH_TILES * TITLE_MAP_HEIGHT_TILES * MAP_BYTES_PER_TILE) 

#endif // CONSTANTS_H