#ifndef CONSTANTS_H
#define CONSTANTS_H

// Screen dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Sprite data configuration
#define SPRITE_DATA_START       0x0000U // Starting address in XRAM for sprite data
#define REDRACER_DATA_SIZE      0x0200U // Size of the RedRacer sprite data

#define SPRITE_DATA_END         (SPRITE_DATA_START + REDRACER_DATA_SIZE)

// 5. Keyboard, Gamepad and Sound
// -------------------------------------------------------------------------
#define OPL_ADDR        0xFE00  // OPL2 Address port
#define GAMEPAD_INPUT   0xFF78  // XRAM address for gamepad data
#define KEYBOARD_INPUT  0xFFA0  // XRAM address for keyboard data
#define PSG_XRAM_ADDR   0xFFC0  // PSG memory location (must match sound.c)

extern unsigned REDRACER_CONFIG;  // RedRacer Sprite Configuration

#endif // CONSTANTS_H