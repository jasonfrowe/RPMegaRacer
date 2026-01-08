#ifndef CONSTANTS_H
#define CONSTANTS_H

// Screen dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Sprite data configuration
#define SPRITE_DATA_START       0x0000U // Starting address in XRAM for sprite data
#define REDRACER_DATA           (SPRITE_DATA_START) // Address for RedRacer sprite
#define REDRACER_DATA_SIZE      0x0200U // Size of the RedRacer sprite data

#define SPRITE_DATA_END         (SPRITE_DATA_START + REDRACER_DATA_SIZE)

// Tile data configuration
#define TRACK_DATA              0x1A14U // Address for track tile data

// 5. Keyboard, Gamepad and Sound
// -------------------------------------------------------------------------
#define OPL_ADDR        0xFE00  // OPL2 Address port
#define PALETTE_ADDR    0xFF58  // XRAM address for palette data
#define GAMEPAD_INPUT   0xFF78  // XRAM address for gamepad data
#define KEYBOARD_INPUT  0xFFA0  // XRAM address for keyboard data
#define PSG_XRAM_ADDR   0xFFC0  // PSG memory location (must match sound.c)

extern unsigned REDRACER_CONFIG;  // RedRacer Sprite Configuration

// Track tile map
#define TRACK_MAP_WIDTH_TILES   64
#define TRACK_MAP_HEIGHT_TILES  48
#define MAP_BYTES_PER_TILE       2
#define TRACK_MAP_SIZE          (TRACK_MAP_WIDTH_TILES * TRACK_MAP_HEIGHT_TILES * MAP_BYTES_PER_TILE)

#endif // CONSTANTS_H