/* RP6502 XRAM Mapping */

#ifndef XRAM_H
#define XRAM_H

#include <rp6502.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Snippets from the docs, copied unchanged. */
/* https://picocomputer.github.io/sdk.html#xram-memory-map */

/* RIA Keyboard */

#define KEYBOARD_NO_KEY 0
#define KEYBOARD_NUM_LOCK 1
#define KEYBOARD_CAPS_LOCK 2
#define KEYBOARD_SCROLL_LOCK 3

#define KEYBOARD_PRESSED(keys, code) ((keys)[(code) >> 3] & (1 << ((code) & 7)))

#define xreg_ria_keyboard(...) xreg(0, 0, 0, __VA_ARGS__)

typedef struct
{
    uint8_t keys[32];
} keyboard_t; /* layout */

/* RIA Gamepads */

#define GAMEPAD_PLAYERS 4

#define xreg_ria_gamepad(...) xreg(0, 0, 2, __VA_ARGS__)

typedef struct
{
    uint8_t dpad;
    uint8_t sticks;
    uint8_t btn0;
    uint8_t btn1;
    int8_t lx;
    int8_t ly;
    int8_t rx;
    int8_t ry;
    uint8_t l2;
    uint8_t r2;
} gamepad_player_t;

typedef struct
{
    gamepad_player_t player[GAMEPAD_PLAYERS];
} gamepad_t; /* layout */

/* RIA OPL2 */

#define xreg_ria_opl(...) xreg(0, 1, 1, __VA_ARGS__)

typedef struct
{
    uint8_t reg[256];
} opl_t; /* layout */

/* VGA Canvas */

#define xreg_vga_canvas(...) xreg(1, 0, 0, __VA_ARGS__)

#define CANVAS_CONSOLE 0
#define CANVAS_320X240 1
#define CANVAS_320X180 2
#define CANVAS_640X480 3
#define CANVAS_640X360 4

/* VGA Mode 1: Character */

#define xreg_vga_mode1(...) xreg(1, 0, 1, 1, __VA_ARGS__)

#define MODE1_1BPP 0x00
#define MODE1_4BPPR 0x01
#define MODE1_4BPP 0x02
#define MODE1_8BPP 0x03
#define MODE1_16BPP 0x04

#define MODE1_8X8 0x00
#define MODE1_8X16 0x08

typedef struct
{
    bool x_wrap;
    bool y_wrap;
    int16_t x_pos_px;
    int16_t y_pos_px;
    int16_t width_chars;
    int16_t height_chars;
    uint16_t xram_data_ptr;
    uint16_t xram_palette_ptr;
    uint16_t xram_font_ptr;
} mode1_config_t; /* layout */

typedef struct
{
    uint8_t glyph_code;
    uint8_t fg_index;
    uint8_t bg_index;
} mode1_8bpp_data_t; /* layout */

/* VGA Mode 2: Tile */

#define xreg_vga_mode2(...) xreg(1, 0, 1, 2, __VA_ARGS__)

#define MODE2_1BPP 0x00
#define MODE2_2BPP 0x01
#define MODE2_4BPP 0x02
#define MODE2_8BPP 0x03

#define MODE2_8X8 0x00
#define MODE2_16X16 0x08

typedef struct
{
    bool x_wrap;
    bool y_wrap;
    int16_t x_pos_px;
    int16_t y_pos_px;
    int16_t width_tiles;
    int16_t height_tiles;
    uint16_t xram_data_ptr;
    uint16_t xram_palette_ptr;
    uint16_t xram_tile_ptr;
} mode2_config_t; /* layout */

/* VGA Mode 4: Sprite */

#define xreg_vga_mode4(...) xreg(1, 0, 1, 4, __VA_ARGS__)

#define MODE4_AFFINE_BIT 0x01

#define MODE4_AFFINE_A00 0
#define MODE4_AFFINE_A01 1
#define MODE4_AFFINE_B0 2
#define MODE4_AFFINE_A10 3
#define MODE4_AFFINE_A11 4
#define MODE4_AFFINE_B1 5
#define MODE4_AFFINE_ONE 0x0100

typedef struct
{
    int16_t transform[6];
    int16_t x_pos_px;
    int16_t y_pos_px;
    uint16_t xram_sprite_ptr;
    uint8_t log_size;
    bool has_opacity_metadata;
} mode4_asprite_t; /* layout */

/* ------------------------------------------------------------------ */
/* RPMegaRacer's XRAM.                                                */
/* ------------------------------------------------------------------ */

#define NUM_CARS 4

/* HUD text grid, 8x8 font on the 320x240 canvas. */
#define HUD_COLS 40
#define HUD_ROWS 30

/* Track and title tile maps: 64x48 tiles of 8x8, 4 bits per pixel. */
#define MAP_W_TILES 64
#define MAP_H_TILES 48
#define MAP_BYTES (MAP_W_TILES * MAP_H_TILES)
#define TILE_BYTES 32
#define TILESET_BYTES (256 * TILE_BYTES)

typedef struct
{
    uint16_t c[16];
} palette_t;

typedef struct
{
    /* First, so the OPL2 registers start on a page boundary. */
    opl_t opl;

    /* 16x16 RGB555 car images, one per car. Loaded with the ROM. */
    uint16_t car_img[NUM_CARS][16][16];

    keyboard_t keyboard;
    gamepad_t gamepad;
    palette_t palette;
    mode4_asprite_t car_spr[NUM_CARS];
    mode2_config_t track_config; /* plane 0: the track */
    mode1_config_t hud_config;   /* plane 1: text, under the cars */
    mode2_config_t title_config; /* plane 2: the title picture */
    mode1_8bpp_data_t hud[HUD_ROWS][HUD_COLS];
    uint8_t track_map[MAP_BYTES];
    uint8_t track_tiles[TILESET_BYTES];
    uint8_t title_map[MAP_BYTES];
    uint8_t title_tiles[TILESET_BYTES];
} xram_layout_t;

#define XRAM_OPL offsetof(xram_layout_t, opl)
_Static_assert((XRAM_OPL & 0xFF) == 0, "The OPL2 registers must start on a page boundary.");

#define XRAM_CAR_IMG offsetof(xram_layout_t, car_img)
#define XRAM_KEYBOARD offsetof(xram_layout_t, keyboard)
#define XRAM_GAMEPAD offsetof(xram_layout_t, gamepad)
#define XRAM_PALETTE offsetof(xram_layout_t, palette)
#define XRAM_CAR_SPR offsetof(xram_layout_t, car_spr)
#define XRAM_TRACK_CONFIG offsetof(xram_layout_t, track_config)
#define XRAM_HUD_CONFIG offsetof(xram_layout_t, hud_config)
#define XRAM_TITLE_CONFIG offsetof(xram_layout_t, title_config)
#define XRAM_HUD offsetof(xram_layout_t, hud)
#define XRAM_TRACK_MAP offsetof(xram_layout_t, track_map)
#define XRAM_TRACK_TILES offsetof(xram_layout_t, track_tiles)
#define XRAM_TITLE_MAP offsetof(xram_layout_t, title_map)
#define XRAM_TITLE_TILES offsetof(xram_layout_t, title_tiles)

#define XRAM_CAR_IMG_N(n) (XRAM_CAR_IMG + (n) * sizeof(uint16_t[16][16]))
#define XRAM_CAR_SPR_N(n) (XRAM_CAR_SPR + (n) * sizeof(mode4_asprite_t))

#endif
