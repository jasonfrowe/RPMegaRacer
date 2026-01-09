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
#define TRACK_DATA              0x1450U // Address for track tile data in XRAM

// 5. Keyboard, Gamepad and Sound
// -------------------------------------------------------------------------
#define OPL_ADDR        0xFE00  // OPL2 Address port
#define PALETTE_ADDR    0xFF58  // XRAM address for palette data
#define GAMEPAD_INPUT   0xFF78  // XRAM address for gamepad data
#define KEYBOARD_INPUT  0xFFA0  // XRAM address for keyboard data
#define PSG_XRAM_ADDR   0xFFC0  // PSG memory location (must match sound.c)

extern unsigned REDRACER_CONFIG;  // RedRacer Sprite Configuration
extern unsigned TRACK_CONFIG;     // Track Tilemap Configuration

// Track tile map
#define TRACK_MAP_WIDTH_TILES   64
#define TRACK_MAP_HEIGHT_TILES  48
#define MAP_BYTES_PER_TILE       1
#define TRACK_MAP_SIZE          (TRACK_MAP_WIDTH_TILES * TRACK_MAP_HEIGHT_TILES * MAP_BYTES_PER_TILE)

#endif // CONSTANTS_H