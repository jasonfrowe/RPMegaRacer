#include <fcntl.h>
#include <rp6502.h>
#include <stdio.h>
#include <unistd.h>
#include "gfx.h"
#include "race.h"
#include "track.h"
#include "trig.h"
#include "xram.h"

static const uint16_t tile_palette[16] = {
    0x0000, 0x0020, 0x41A7, 0x1AE0, 0x72AA, 0x0038, 0x003E, 0x0372,
    0x2C60, 0x35AE, 0x053C, 0x073E, 0x93AE, 0xC4B4, 0xD534, 0xF7BE,
};

/* What gfx_commit() writes: each car's transform and position, then the
 * scroll. The sprite part matches the start of mode4_asprite_t. */
typedef struct
{
    int16_t transform[6];
    int16_t x_pos_px;
    int16_t y_pos_px;
} car_sprite_t;

static car_sprite_t shadow[NUM_CARS];
static int16_t scroll_x, scroll_y;

void gfx_init(void)
{
    xreg_vga_canvas(CANVAS_320X240);
    xram0_write(XRAM_PALETTE, tile_palette, sizeof(tile_palette));

    for (uint8_t i = 0; i < NUM_CARS; i++) {
        mode4_asprite_t s = {
            .transform = {MODE4_AFFINE_ONE, 0, 0, 0, MODE4_AFFINE_ONE, 0},
            .x_pos_px = -32,
            .y_pos_px = -32,
            .xram_sprite_ptr = XRAM_CAR_IMG_N(i),
            .log_size = 4,
            .has_opacity_metadata = false,
        };
        xram0_write(XRAM_CAR_SPR_N(i), &s, sizeof(s));
    }

    mode2_config_t track = {
        .x_wrap = false,
        .y_wrap = false,
        .width_tiles = MAP_W_TILES,
        .height_tiles = MAP_H_TILES,
        .xram_data_ptr = XRAM_TRACK_MAP,
        .xram_palette_ptr = XRAM_PALETTE,
        .xram_tile_ptr = XRAM_TRACK_TILES,
    };
    xram0_write(XRAM_TRACK_CONFIG, &track, sizeof(track));

    mode1_config_t hud = {
        .x_wrap = false,
        .y_wrap = false,
        .x_pos_px = 0,
        .y_pos_px = 0,
        .width_chars = HUD_COLS,
        .height_chars = HUD_ROWS,
        .xram_data_ptr = XRAM_HUD,
        .xram_palette_ptr = 0xFFFF,
        .xram_font_ptr = 0xFFFF,
    };
    xram0_write(XRAM_HUD_CONFIG, &hud, sizeof(hud));
    xram0_set(XRAM_HUD, 0, sizeof(mode1_8bpp_data_t) * HUD_ROWS * HUD_COLS);

    mode2_config_t title = track;
    title.x_wrap = true;
    title.y_wrap = true;
    title.xram_data_ptr = XRAM_TITLE_MAP;
    title.xram_tile_ptr = XRAM_TITLE_TILES;
    xram0_write(XRAM_TITLE_CONFIG, &title, sizeof(title));
    xram0_set(XRAM_TITLE_MAP, 0, MAP_BYTES);

    xreg_vga_mode2(MODE2_4BPP | MODE2_8X8, XRAM_TRACK_CONFIG, 0);
    xreg_vga_mode4(MODE4_AFFINE_BIT, XRAM_CAR_SPR, NUM_CARS, 1);
    xreg_vga_mode1(MODE1_8BPP, XRAM_HUD_CONFIG, 1);
    xreg_vga_mode2(MODE2_4BPP | MODE2_8X8, XRAM_TITLE_CONFIG, 2);
}

void gfx_title_show(void)
{
    int fd = open("ROM:title_tiles.bin", O_RDONLY);
    if (fd >= 0) {
        read_xram(XRAM_TITLE_TILES, TILESET_BYTES, fd);
        close(fd);
    }
    fd = open("ROM:title_map.bin", O_RDONLY);
    if (fd >= 0) {
        read_xram(XRAM_TITLE_MAP, MAP_BYTES, fd);
        close(fd);
    }
}

void gfx_title_hide(void)
{
    xram0_set(XRAM_TITLE_MAP, 0, MAP_BYTES);
}

void gfx_commit(void)
{
    RIA.step0 = 1;
    for (uint8_t i = 0; i < NUM_CARS; i++) {
        RIA.addr0 = XRAM_CAR_SPR_N(i);
        const uint8_t *p = (const uint8_t *)&shadow[i];
        for (uint8_t k = 0; k < sizeof(car_sprite_t); k++)
            RIA.rw0 = p[k];
    }
    xram0_poke16(XRAM_TRACK_CONFIG + offsetof(mode2_config_t, x_pos_px), (uint16_t)scroll_x);
    xram0_poke16(XRAM_TRACK_CONFIG + offsetof(mode2_config_t, y_pos_px), (uint16_t)scroll_y);
}

void gfx_prepare(void)
{
    /* Centre the player, clamped to the 512x384 world. */
    int16_t sx = 160 - PX(racers[PLAYER].car.x);
    int16_t sy = 120 - PX(racers[PLAYER].car.y);
    if (sx > 0) sx = 0;
    if (sx < 320 - WORLD_W) sx = 320 - WORLD_W;
    if (sy > 0) sy = 0;
    if (sy < 240 - WORLD_H) sy = 240 - WORLD_H;
    scroll_x = sx;
    scroll_y = sy;

    for (uint8_t i = 0; i < NUM_CARS; i++) {
        const Car *c = &racers[i].car;
        uint8_t a = (uint8_t)(c->heading >> 8);
        int16_t s = (int16_t)SIN(a) << 1;
        int16_t co = (int16_t)COS(a) << 1;
        car_sprite_t *d = &shadow[i];
        d->transform[0] = co;
        d->transform[1] = -s;
        d->transform[2] = TX_LUT[a];
        d->transform[3] = s;
        d->transform[4] = co;
        d->transform[5] = TY_LUT[a];
        d->x_pos_px = PX(c->x) - 8 + sx;
        d->y_pos_px = PX(c->y) - 8 + sy;
    }
}
