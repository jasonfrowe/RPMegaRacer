#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "track.h"

uint8_t world_map[3072];

uint8_t tile_properties[256];

// Collision masks: 8 rows per tile, 8 bits per row
// Bit 7 (0x80) = leftmost pixel, Bit 0 (0x01) = rightmost pixel
// 1 = solid/wall, 0 = passable
uint8_t tile_collision_masks[256][8];

// Forward declaration - generated collision masks and tile properties
void init_tile_collision_masks(void);
void init_tile_properties(void);

void init_track_physics(void) {
    // Initialize all collision masks to 0 (passable)
    memset(tile_collision_masks, 0, sizeof(tile_collision_masks));

    // Initialize all tile properties to WALL by default
    for (int i = 0; i < 256; i++) {
        tile_properties[i] = TERRAIN_WALL;
    }

    // Load auto-generated collision masks and tile properties from tile graphics
    init_tile_collision_masks();
    init_tile_properties();
}

uint8_t get_terrain_at(int16_t x, int16_t y) {
    // 1. Clamp to world bounds (handle negative and out-of-bounds)
    if (x < 0 || y < 0 || x >= 512 || y >= 384) return TERRAIN_WALL;

    // 2. Convert pixels to tile coordinates (8x8 tiles)
    uint8_t tx = x >> 3; // x / 8
    uint8_t ty = y >> 3; // y / 8
    uint8_t px = x & 7;  // Pixel within tile X (0-7)
    uint8_t py = y & 7;  // Pixel within tile Y (0-7)

    // 3. Get Tile ID from the map (Width is 64, 1 byte per tile)
    uint16_t map_index = ty * 64 + tx;
    uint8_t tile_id = world_map[map_index];

    // 4. Check pixel-level collision mask
    uint8_t row_mask = tile_collision_masks[tile_id][py];
    if (row_mask != 0) {
        // This tile has collision data - check the specific pixel
        uint8_t pixel_mask = 0x80 >> px;  // Bit mask for this pixel (bit 7 to bit 0)
        if (row_mask & pixel_mask) {
            return TERRAIN_WALL;  // This specific pixel is solid
        }
    }

    // 5. Fall back to tile properties for tiles without collision masks
    return tile_properties[tile_id];
}

// Generated from Track_A_tiles.bin
// Found 249 tiles (7968 bytes)

void init_tile_collision_masks(void) {
    // Tile 0
    tile_collision_masks[0][0] = 0xFF;  // ████████
    tile_collision_masks[0][1] = 0xFF;  // ████████
    tile_collision_masks[0][2] = 0xFF;  // ████████
    tile_collision_masks[0][3] = 0xFF;  // ████████
    tile_collision_masks[0][4] = 0xFF;  // ████████
    tile_collision_masks[0][5] = 0xFF;  // ████████
    tile_collision_masks[0][6] = 0xFF;  // ████████
    tile_collision_masks[0][7] = 0xFF;  // ████████
    // Tile 3
    tile_collision_masks[3][1] = 0xFF;  // ████████
    tile_collision_masks[3][2] = 0xFF;  // ████████
    tile_collision_masks[3][3] = 0x87;  // █□□□□███
    tile_collision_masks[3][4] = 0xC3;  // ██□□□□██
    tile_collision_masks[3][5] = 0xE1;  // ███□□□□█
    tile_collision_masks[3][7] = 0xFF;  // ████████
    // Rows [0, 6] are fully passable
    // Tile 4
    tile_collision_masks[4][4] = 0x07;  // □□□□□███
    tile_collision_masks[4][5] = 0x1C;  // □□□███□□
    tile_collision_masks[4][6] = 0x38;  // □□███□□□
    tile_collision_masks[4][7] = 0x58;  // □█□██□□□
    // Rows [0, 1, 2, 3] are fully passable
    // Tile 5
    tile_collision_masks[5][2] = 0x0F;  // □□□□████
    tile_collision_masks[5][3] = 0xFF;  // ████████
    tile_collision_masks[5][4] = 0x7F;  // □███████
    tile_collision_masks[5][5] = 0x70;  // □███□□□□
    tile_collision_masks[5][6] = 0xFF;  // ████████
    // Rows [0, 1, 7] are fully passable
    // Tile 6
    tile_collision_masks[6][1] = 0xFF;  // ████████
    tile_collision_masks[6][2] = 0xFF;  // ████████
    tile_collision_masks[6][3] = 0xFC;  // ██████□□
    tile_collision_masks[6][4] = 0xFC;  // ██████□□
    // Rows [0, 5, 6, 7] are fully passable
    // Tile 7
    tile_collision_masks[7][0] = 0xFF;  // ████████
    tile_collision_masks[7][1] = 0xFC;  // ██████□□
    tile_collision_masks[7][2] = 0xFC;  // ██████□□
    tile_collision_masks[7][3] = 0xC0;  // ██□□□□□□
    tile_collision_masks[7][4] = 0x3C;  // □□████□□
    tile_collision_masks[7][5] = 0xC0;  // ██□□□□□□
    // Rows [6, 7] are fully passable
    // Tile 8
    tile_collision_masks[8][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[8][3] = 0x03;  // □□□□□□██
    tile_collision_masks[8][4] = 0x02;  // □□□□□□█□
    tile_collision_masks[8][5] = 0x06;  // □□□□□██□
    tile_collision_masks[8][6] = 0x0C;  // □□□□██□□
    tile_collision_masks[8][7] = 0x0C;  // □□□□██□□
    // Rows [0, 1] are fully passable
    // Tile 9
    tile_collision_masks[9][0] = 0x30;  // □□██□□□□
    tile_collision_masks[9][1] = 0x60;  // □██□□□□□
    tile_collision_masks[9][2] = 0x60;  // □██□□□□□
    tile_collision_masks[9][3] = 0x60;  // □██□□□□□
    tile_collision_masks[9][4] = 0x60;  // □██□□□□□
    tile_collision_masks[9][5] = 0x60;  // □██□□□□□
    tile_collision_masks[9][6] = 0x60;  // □██□□□□□
    tile_collision_masks[9][7] = 0x60;  // □██□□□□□
    // Tile 10
    tile_collision_masks[10][0] = 0x18;  // □□□██□□□
    tile_collision_masks[10][1] = 0x18;  // □□□██□□□
    tile_collision_masks[10][2] = 0x18;  // □□□██□□□
    tile_collision_masks[10][3] = 0x18;  // □□□██□□□
    tile_collision_masks[10][4] = 0x18;  // □□□██□□□
    tile_collision_masks[10][5] = 0x18;  // □□□██□□□
    tile_collision_masks[10][6] = 0x18;  // □□□██□□□
    tile_collision_masks[10][7] = 0x18;  // □□□██□□□
    // Tile 11
    tile_collision_masks[11][0] = 0x06;  // □□□□□██□
    tile_collision_masks[11][1] = 0x06;  // □□□□□██□
    tile_collision_masks[11][2] = 0x06;  // □□□□□██□
    tile_collision_masks[11][3] = 0x06;  // □□□□□██□
    tile_collision_masks[11][4] = 0x03;  // □□□□□□██
    tile_collision_masks[11][5] = 0x03;  // □□□□□□██
    tile_collision_masks[11][6] = 0x03;  // □□□□□□██
    tile_collision_masks[11][7] = 0x03;  // □□□□□□██
    // Tile 12
    tile_collision_masks[12][0] = 0x70;  // □███□□□□
    tile_collision_masks[12][1] = 0x70;  // □███□□□□
    tile_collision_masks[12][2] = 0x70;  // □███□□□□
    tile_collision_masks[12][3] = 0x70;  // □███□□□□
    tile_collision_masks[12][4] = 0x18;  // □□□██□□□
    tile_collision_masks[12][5] = 0x18;  // □□□██□□□
    tile_collision_masks[12][6] = 0x38;  // □□███□□□
    tile_collision_masks[12][7] = 0x38;  // □□███□□□
    // Tile 13
    tile_collision_masks[13][0] = 0x3C;  // □□████□□
    tile_collision_masks[13][1] = 0x3C;  // □□████□□
    tile_collision_masks[13][2] = 0x3C;  // □□████□□
    tile_collision_masks[13][3] = 0x2C;  // □□█□██□□
    tile_collision_masks[13][4] = 0x2C;  // □□█□██□□
    tile_collision_masks[13][5] = 0x2C;  // □□█□██□□
    tile_collision_masks[13][6] = 0x3C;  // □□████□□
    tile_collision_masks[13][7] = 0x1E;  // □□□████□
    // Tile 14
    tile_collision_masks[14][0] = 0x0F;  // □□□□████
    tile_collision_masks[14][1] = 0x0F;  // □□□□████
    tile_collision_masks[14][2] = 0x0F;  // □□□□████
    tile_collision_masks[14][3] = 0x0B;  // □□□□█□██
    tile_collision_masks[14][4] = 0x0B;  // □□□□█□██
    tile_collision_masks[14][5] = 0x05;  // □□□□□█□█
    tile_collision_masks[14][6] = 0x05;  // □□□□□█□█
    tile_collision_masks[14][7] = 0x07;  // □□□□□███
    // Tile 15
    tile_collision_masks[15][0] = 0x03;  // □□□□□□██
    tile_collision_masks[15][1] = 0x02;  // □□□□□□█□
    tile_collision_masks[15][2] = 0x01;  // □□□□□□□█
    // Rows [3, 4, 5, 6, 7] are fully passable
    // Tile 16
    tile_collision_masks[16][0] = 0xE0;  // ███□□□□□
    tile_collision_masks[16][1] = 0xF0;  // ████□□□□
    tile_collision_masks[16][2] = 0x78;  // □████□□□
    tile_collision_masks[16][3] = 0xCE;  // ██□□███□
    tile_collision_masks[16][4] = 0x87;  // █□□□□███
    tile_collision_masks[16][5] = 0x43;  // □█□□□□██
    tile_collision_masks[16][6] = 0x2F;  // □□█□████
    tile_collision_masks[16][7] = 0x1F;  // □□□█████
    // Tile 17
    tile_collision_masks[17][0] = 0x0D;  // □□□□██□█
    tile_collision_masks[17][1] = 0x06;  // □□□□□██□
    tile_collision_masks[17][2] = 0x01;  // □□□□□□□█
    // Rows [3, 4, 5, 6, 7] are fully passable
    // Tile 18
    tile_collision_masks[18][0] = 0x9F;  // █□□█████
    tile_collision_masks[18][1] = 0xBC;  // █□████□□
    tile_collision_masks[18][2] = 0x8E;  // █□□□███□
    tile_collision_masks[18][3] = 0x7B;  // □████□██
    tile_collision_masks[18][4] = 0x04;  // □□□□□█□□
    tile_collision_masks[18][5] = 0x03;  // □□□□□□██
    // Rows [6, 7] are fully passable
    // Tile 19
    tile_collision_masks[19][0] = 0xFF;  // ████████
    tile_collision_masks[19][1] = 0x7F;  // □███████
    tile_collision_masks[19][2] = 0x73;  // □███□□██
    tile_collision_masks[19][3] = 0x39;  // □□███□□█
    tile_collision_masks[19][4] = 0x1C;  // □□□███□□
    tile_collision_masks[19][5] = 0x80;  // █□□□□□□□
    tile_collision_masks[19][6] = 0x7F;  // □███████
    // Rows [7] are fully passable
    // Tile 20
    tile_collision_masks[20][1] = 0xE0;  // ███□□□□□
    tile_collision_masks[20][2] = 0xFF;  // ████████
    tile_collision_masks[20][3] = 0x9F;  // █□□█████
    tile_collision_masks[20][4] = 0xC7;  // ██□□□███
    tile_collision_masks[20][5] = 0xE3;  // ███□□□██
    tile_collision_masks[20][6] = 0x81;  // █□□□□□□█
    tile_collision_masks[20][7] = 0xE0;  // ███□□□□□
    // Rows [0] are fully passable
    // Tile 21
    tile_collision_masks[21][0] = 0xFF;  // ████████
    tile_collision_masks[21][1] = 0x7F;  // □███████
    tile_collision_masks[21][2] = 0x7F;  // □███████
    tile_collision_masks[21][3] = 0x07;  // □□□□□███
    tile_collision_masks[21][4] = 0xF8;  // █████□□□
    tile_collision_masks[21][5] = 0x07;  // □□□□□███
    // Rows [6, 7] are fully passable
    // Tile 22
    tile_collision_masks[22][1] = 0xFE;  // ███████□
    tile_collision_masks[22][2] = 0xFF;  // ████████
    tile_collision_masks[22][3] = 0x7F;  // □███████
    tile_collision_masks[22][4] = 0x7F;  // □███████
    tile_collision_masks[22][6] = 0xFF;  // ████████
    // Rows [0, 5, 7] are fully passable
    // Tile 23
    tile_collision_masks[23][2] = 0xE0;  // ███□□□□□
    tile_collision_masks[23][3] = 0xFE;  // ███████□
    tile_collision_masks[23][4] = 0xFF;  // ████████
    tile_collision_masks[23][5] = 0xFC;  // ██████□□
    // Rows [0, 1, 6, 7] are fully passable
    // Tile 24
    tile_collision_masks[24][4] = 0xC0;  // ██□□□□□□
    tile_collision_masks[24][5] = 0xF0;  // ████□□□□
    tile_collision_masks[24][6] = 0x78;  // □████□□□
    tile_collision_masks[24][7] = 0x3C;  // □□████□□
    // Rows [0, 1, 2, 3] are fully passable
    // Tile 25
    tile_collision_masks[25][0] = 0x02;  // □□□□□□█□
    tile_collision_masks[25][1] = 0x01;  // □□□□□□□█
    tile_collision_masks[25][2] = 0x01;  // □□□□□□□█
    // Rows [3, 4, 5, 6, 7] are fully passable
    // Tile 26
    tile_collision_masks[26][2] = 0x80;  // █□□□□□□□
    tile_collision_masks[26][3] = 0xC0;  // ██□□□□□□
    tile_collision_masks[26][4] = 0x40;  // □█□□□□□□
    tile_collision_masks[26][5] = 0x60;  // □██□□□□□
    tile_collision_masks[26][6] = 0x30;  // □□██□□□□
    tile_collision_masks[26][7] = 0x30;  // □□██□□□□
    // Rows [0, 1] are fully passable
    // Tile 27
    tile_collision_masks[27][0] = 0x18;  // □□□██□□□
    tile_collision_masks[27][1] = 0x18;  // □□□██□□□
    tile_collision_masks[27][2] = 0x18;  // □□□██□□□
    tile_collision_masks[27][3] = 0x18;  // □□□██□□□
    tile_collision_masks[27][4] = 0x18;  // □□□██□□□
    tile_collision_masks[27][5] = 0x18;  // □□□██□□□
    tile_collision_masks[27][6] = 0x18;  // □□□██□□□
    tile_collision_masks[27][7] = 0x18;  // □□□██□□□
    // Tile 28
    tile_collision_masks[28][0] = 0x0C;  // □□□□██□□
    tile_collision_masks[28][1] = 0x06;  // □□□□□██□
    tile_collision_masks[28][2] = 0x06;  // □□□□□██□
    tile_collision_masks[28][3] = 0x06;  // □□□□□██□
    tile_collision_masks[28][4] = 0x06;  // □□□□□██□
    tile_collision_masks[28][5] = 0x06;  // □□□□□██□
    tile_collision_masks[28][6] = 0x06;  // □□□□□██□
    tile_collision_masks[28][7] = 0x06;  // □□□□□██□
    // Tile 29
    tile_collision_masks[29][0] = 0x07;  // □□□□□███
    tile_collision_masks[29][1] = 0x0F;  // □□□□████
    tile_collision_masks[29][2] = 0x1E;  // □□□████□
    tile_collision_masks[29][3] = 0x73;  // □███□□██
    tile_collision_masks[29][4] = 0xE1;  // ███□□□□█
    tile_collision_masks[29][5] = 0xC2;  // ██□□□□█□
    tile_collision_masks[29][6] = 0xE4;  // ███□□█□□
    tile_collision_masks[29][7] = 0xE8;  // ███□█□□□
    // Tile 30
    tile_collision_masks[30][0] = 0x01;  // □□□□□□□█
    tile_collision_masks[30][1] = 0xFF;  // ████████
    tile_collision_masks[30][2] = 0xFF;  // ████████
    tile_collision_masks[30][3] = 0x87;  // █□□□□███
    tile_collision_masks[30][4] = 0xC3;  // ██□□□□██
    tile_collision_masks[30][5] = 0xE0;  // ███□□□□□
    tile_collision_masks[30][6] = 0x81;  // █□□□□□□█
    tile_collision_masks[30][7] = 0xFE;  // ███████□
    // Tile 31
    tile_collision_masks[31][0] = 0xFF;  // ████████
    tile_collision_masks[31][1] = 0xFF;  // ████████
    tile_collision_masks[31][2] = 0x87;  // █□□□□███
    tile_collision_masks[31][3] = 0xC3;  // ██□□□□██
    tile_collision_masks[31][4] = 0xE0;  // ███□□□□□
    tile_collision_masks[31][5] = 0x81;  // █□□□□□□█
    tile_collision_masks[31][6] = 0xFE;  // ███████□
    // Rows [7] are fully passable
    // Tile 32
    tile_collision_masks[32][0] = 0xC3;  // ██□□□□██
    tile_collision_masks[32][1] = 0xE0;  // ███□□□□□
    tile_collision_masks[32][2] = 0xE3;  // ███□□□██
    tile_collision_masks[32][3] = 0xBC;  // █□████□□
    tile_collision_masks[32][4] = 0xE0;  // ███□□□□□
    // Rows [5, 6, 7] are fully passable
    // Tile 33
    tile_collision_masks[33][0] = 0x1E;  // □□□████□
    tile_collision_masks[33][1] = 0x1E;  // □□□████□
    tile_collision_masks[33][2] = 0x1E;  // □□□████□
    tile_collision_masks[33][3] = 0x16;  // □□□█□██□
    tile_collision_masks[33][4] = 0x16;  // □□□█□██□
    tile_collision_masks[33][5] = 0x16;  // □□□█□██□
    tile_collision_masks[33][6] = 0x16;  // □□□█□██□
    tile_collision_masks[33][7] = 0x1E;  // □□□████□
    // Tile 34
    tile_collision_masks[34][0] = 0xFF;  // ████████
    tile_collision_masks[34][1] = 0xFF;  // ████████
    tile_collision_masks[34][2] = 0xF0;  // ████□□□□
    tile_collision_masks[34][4] = 0x0F;  // □□□□████
    tile_collision_masks[34][5] = 0xF0;  // ████□□□□
    // Rows [3, 6, 7] are fully passable
    // Tile 35
    tile_collision_masks[35][0] = 0x1F;  // □□□█████
    tile_collision_masks[35][1] = 0xFF;  // ████████
    tile_collision_masks[35][2] = 0xFF;  // ████████
    tile_collision_masks[35][3] = 0xFF;  // ████████
    tile_collision_masks[35][4] = 0xF0;  // ████□□□□
    tile_collision_masks[35][5] = 0xE0;  // ███□□□□□
    tile_collision_masks[35][6] = 0xCF;  // ██□□████
    tile_collision_masks[35][7] = 0x30;  // □□██□□□□
    // Tile 36
    tile_collision_masks[36][0] = 0xFE;  // ███████□
    tile_collision_masks[36][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[36][3] = 0xFE;  // ███████□
    // Rows [1, 4, 5, 6, 7] are fully passable
    // Tile 37
    tile_collision_masks[37][1] = 0x1F;  // □□□█████
    tile_collision_masks[37][2] = 0xE0;  // ███□□□□□
    // Rows [0, 3, 4, 5, 6, 7] are fully passable
    // Tile 38
    tile_collision_masks[38][0] = 0xFE;  // ███████□
    tile_collision_masks[38][1] = 0xF9;  // █████□□█
    tile_collision_masks[38][2] = 0xF6;  // ████□██□
    tile_collision_masks[38][3] = 0xE8;  // ███□█□□□
    tile_collision_masks[38][4] = 0xD0;  // ██□█□□□□
    tile_collision_masks[38][5] = 0xA0;  // █□█□□□□□
    tile_collision_masks[38][6] = 0x40;  // □█□□□□□□
    tile_collision_masks[38][7] = 0x40;  // □█□□□□□□
    // Tile 39
    tile_collision_masks[39][0] = 0x3E;  // □□█████□
    tile_collision_masks[39][1] = 0x7E;  // □██████□
    tile_collision_masks[39][2] = 0x7D;  // □█████□█
    tile_collision_masks[39][3] = 0xFD;  // ██████□█
    tile_collision_masks[39][4] = 0xFA;  // █████□█□
    tile_collision_masks[39][5] = 0xFA;  // █████□█□
    tile_collision_masks[39][6] = 0xFA;  // █████□█□
    tile_collision_masks[39][7] = 0xF4;  // ████□█□□
    // Tile 40
    tile_collision_masks[40][0] = 0xF4;  // ████□█□□
    tile_collision_masks[40][1] = 0xE8;  // ███□█□□□
    tile_collision_masks[40][2] = 0xE8;  // ███□█□□□
    tile_collision_masks[40][3] = 0xD0;  // ██□█□□□□
    tile_collision_masks[40][4] = 0xD0;  // ██□█□□□□
    tile_collision_masks[40][5] = 0xD0;  // ██□█□□□□
    tile_collision_masks[40][6] = 0xD0;  // ██□█□□□□
    tile_collision_masks[40][7] = 0xD0;  // ██□█□□□□
    // Tile 41
    tile_collision_masks[41][0] = 0xD0;  // ██□█□□□□
    tile_collision_masks[41][1] = 0xA0;  // █□█□□□□□
    tile_collision_masks[41][2] = 0xA0;  // █□█□□□□□
    tile_collision_masks[41][3] = 0xA0;  // █□█□□□□□
    tile_collision_masks[41][4] = 0xA0;  // █□█□□□□□
    tile_collision_masks[41][5] = 0xA0;  // █□█□□□□□
    tile_collision_masks[41][6] = 0xA0;  // █□█□□□□□
    tile_collision_masks[41][7] = 0xA0;  // █□█□□□□□
    // Tile 42
    tile_collision_masks[42][0] = 0xA0;  // █□█□□□□□
    tile_collision_masks[42][1] = 0xA0;  // █□█□□□□□
    tile_collision_masks[42][2] = 0xA0;  // █□█□□□□□
    tile_collision_masks[42][3] = 0x40;  // □█□□□□□□
    tile_collision_masks[42][4] = 0x40;  // □█□□□□□□
    tile_collision_masks[42][5] = 0x40;  // □█□□□□□□
    tile_collision_masks[42][6] = 0x40;  // □█□□□□□□
    tile_collision_masks[42][7] = 0x40;  // □█□□□□□□
    // Tile 43
    tile_collision_masks[43][1] = 0xF0;  // ████□□□□
    tile_collision_masks[43][2] = 0x0F;  // □□□□████
    // Rows [0, 3, 4, 5, 6, 7] are fully passable
    // Tile 44
    tile_collision_masks[44][0] = 0xFF;  // ████████
    tile_collision_masks[44][1] = 0x01;  // □□□□□□□█
    tile_collision_masks[44][3] = 0xFE;  // ███████□
    tile_collision_masks[44][4] = 0x01;  // □□□□□□□█
    // Rows [2, 5, 6, 7] are fully passable
    // Tile 45
    tile_collision_masks[45][0] = 0xFF;  // ████████
    tile_collision_masks[45][1] = 0xFF;  // ████████
    tile_collision_masks[45][2] = 0x1F;  // □□□█████
    tile_collision_masks[45][3] = 0x01;  // □□□□□□□█
    tile_collision_masks[45][4] = 0xE0;  // ███□□□□□
    tile_collision_masks[45][5] = 0x1E;  // □□□████□
    tile_collision_masks[45][6] = 0x01;  // □□□□□□□█
    // Rows [7] are fully passable
    // Tile 46
    tile_collision_masks[46][0] = 0xF0;  // ████□□□□
    tile_collision_masks[46][1] = 0xFE;  // ███████□
    tile_collision_masks[46][2] = 0xFF;  // ████████
    tile_collision_masks[46][3] = 0xFF;  // ████████
    tile_collision_masks[46][4] = 0x1F;  // □□□█████
    tile_collision_masks[46][5] = 0x0F;  // □□□□████
    tile_collision_masks[46][6] = 0xE7;  // ███□□███
    tile_collision_masks[46][7] = 0x19;  // □□□██□□█
    // Tile 47
    tile_collision_masks[47][0] = 0xFE;  // ███████□
    tile_collision_masks[47][1] = 0x3F;  // □□██████
    tile_collision_masks[47][2] = 0xDF;  // ██□█████
    tile_collision_masks[47][3] = 0x2F;  // □□█□████
    tile_collision_masks[47][4] = 0x17;  // □□□█□███
    tile_collision_masks[47][5] = 0x0B;  // □□□□█□██
    tile_collision_masks[47][6] = 0x05;  // □□□□□█□█
    tile_collision_masks[47][7] = 0x05;  // □□□□□█□█
    // Tile 48
    tile_collision_masks[48][0] = 0xF8;  // █████□□□
    tile_collision_masks[48][1] = 0xFC;  // ██████□□
    tile_collision_masks[48][2] = 0x7C;  // □█████□□
    tile_collision_masks[48][3] = 0x7E;  // □██████□
    tile_collision_masks[48][4] = 0xBE;  // █□█████□
    tile_collision_masks[48][5] = 0xBE;  // █□█████□
    tile_collision_masks[48][6] = 0xBF;  // █□██████
    tile_collision_masks[48][7] = 0x5F;  // □█□█████
    // Tile 49
    tile_collision_masks[49][0] = 0x5F;  // □█□█████
    tile_collision_masks[49][1] = 0x2F;  // □□█□████
    tile_collision_masks[49][2] = 0x2F;  // □□█□████
    tile_collision_masks[49][3] = 0x17;  // □□□█□███
    tile_collision_masks[49][4] = 0x17;  // □□□█□███
    tile_collision_masks[49][5] = 0x17;  // □□□█□███
    tile_collision_masks[49][6] = 0x17;  // □□□█□███
    tile_collision_masks[49][7] = 0x17;  // □□□█□███
    // Tile 50
    tile_collision_masks[50][0] = 0x17;  // □□□█□███
    tile_collision_masks[50][1] = 0x0B;  // □□□□█□██
    tile_collision_masks[50][2] = 0x0B;  // □□□□█□██
    tile_collision_masks[50][3] = 0x0B;  // □□□□█□██
    tile_collision_masks[50][4] = 0x0B;  // □□□□█□██
    tile_collision_masks[50][5] = 0x0B;  // □□□□█□██
    tile_collision_masks[50][6] = 0x0B;  // □□□□█□██
    tile_collision_masks[50][7] = 0x0B;  // □□□□█□██
    // Tile 51
    tile_collision_masks[51][0] = 0x0B;  // □□□□█□██
    tile_collision_masks[51][1] = 0x0B;  // □□□□█□██
    tile_collision_masks[51][2] = 0x0B;  // □□□□█□██
    tile_collision_masks[51][3] = 0x05;  // □□□□□█□█
    tile_collision_masks[51][4] = 0x05;  // □□□□□█□█
    tile_collision_masks[51][5] = 0x05;  // □□□□□█□█
    tile_collision_masks[51][6] = 0x05;  // □□□□□█□█
    tile_collision_masks[51][7] = 0x05;  // □□□□□█□█
    // Tile 52
    tile_collision_masks[52][3] = 0x80;  // █□□□□□□□
    tile_collision_masks[52][4] = 0x80;  // █□□□□□□□
    tile_collision_masks[52][5] = 0x80;  // █□□□□□□□
    tile_collision_masks[52][6] = 0x80;  // █□□□□□□□
    tile_collision_masks[52][7] = 0x80;  // █□□□□□□□
    // Rows [0, 1, 2] are fully passable
    // Tile 53
    tile_collision_masks[53][0] = 0xC0;  // ██□□□□□□
    tile_collision_masks[53][1] = 0xC0;  // ██□□□□□□
    tile_collision_masks[53][2] = 0xC0;  // ██□□□□□□
    tile_collision_masks[53][3] = 0xC0;  // ██□□□□□□
    tile_collision_masks[53][4] = 0xC0;  // ██□□□□□□
    tile_collision_masks[53][5] = 0xC0;  // ██□□□□□□
    tile_collision_masks[53][6] = 0xC0;  // ██□□□□□□
    tile_collision_masks[53][7] = 0xE0;  // ███□□□□□
    // Tile 54
    tile_collision_masks[54][0] = 0xE0;  // ███□□□□□
    tile_collision_masks[54][1] = 0xE0;  // ███□□□□□
    tile_collision_masks[54][2] = 0xE0;  // ███□□□□□
    tile_collision_masks[54][3] = 0xE0;  // ███□□□□□
    tile_collision_masks[54][4] = 0xE0;  // ███□□□□□
    tile_collision_masks[54][5] = 0xF0;  // ████□□□□
    tile_collision_masks[54][6] = 0xF0;  // ████□□□□
    tile_collision_masks[54][7] = 0xF8;  // █████□□□
    // Tile 55
    tile_collision_masks[55][0] = 0xF8;  // █████□□□
    tile_collision_masks[55][1] = 0xFC;  // ██████□□
    tile_collision_masks[55][2] = 0xFC;  // ██████□□
    tile_collision_masks[55][3] = 0xFC;  // ██████□□
    tile_collision_masks[55][4] = 0xFE;  // ███████□
    tile_collision_masks[55][5] = 0x7E;  // □██████□
    tile_collision_masks[55][6] = 0x7F;  // □███████
    tile_collision_masks[55][7] = 0x3F;  // □□██████
    // Tile 56
    tile_collision_masks[56][0] = 0xC0;  // ██□□□□□□
    tile_collision_masks[56][1] = 0xE0;  // ███□□□□□
    tile_collision_masks[56][2] = 0xF0;  // ████□□□□
    tile_collision_masks[56][3] = 0xF8;  // █████□□□
    tile_collision_masks[56][4] = 0xFC;  // ██████□□
    tile_collision_masks[56][5] = 0xFF;  // ████████
    tile_collision_masks[56][6] = 0xFF;  // ████████
    tile_collision_masks[56][7] = 0xFF;  // ████████
    // Tile 57
    tile_collision_masks[57][0] = 0xF8;  // █████□□□
    tile_collision_masks[57][1] = 0xFF;  // ████████
    tile_collision_masks[57][2] = 0xFF;  // ████████
    tile_collision_masks[57][3] = 0xFF;  // ████████
    tile_collision_masks[57][4] = 0xFF;  // ████████
    tile_collision_masks[57][5] = 0xFF;  // ████████
    tile_collision_masks[57][6] = 0xFF;  // ████████
    tile_collision_masks[57][7] = 0xFF;  // ████████
    // Tile 58
    tile_collision_masks[58][1] = 0x80;  // █□□□□□□□
    tile_collision_masks[58][2] = 0xF8;  // █████□□□
    tile_collision_masks[58][3] = 0xFF;  // ████████
    tile_collision_masks[58][4] = 0xFF;  // ████████
    tile_collision_masks[58][5] = 0xFF;  // ████████
    tile_collision_masks[58][6] = 0xFF;  // ████████
    tile_collision_masks[58][7] = 0xFF;  // ████████
    // Rows [0] are fully passable
    // Tile 59
    tile_collision_masks[59][3] = 0x80;  // █□□□□□□□
    tile_collision_masks[59][4] = 0xFF;  // ████████
    tile_collision_masks[59][5] = 0xFF;  // ████████
    tile_collision_masks[59][6] = 0xFF;  // ████████
    tile_collision_masks[59][7] = 0xFF;  // ████████
    // Rows [0, 1, 2] are fully passable
    // Tile 60
    tile_collision_masks[60][5] = 0xFF;  // ████████
    tile_collision_masks[60][6] = 0xFF;  // ████████
    tile_collision_masks[60][7] = 0xFF;  // ████████
    // Rows [0, 1, 2, 3, 4] are fully passable
    // Tile 61
    tile_collision_masks[61][5] = 0xF0;  // ████□□□□
    tile_collision_masks[61][6] = 0xFF;  // ████████
    tile_collision_masks[61][7] = 0xFF;  // ████████
    // Rows [0, 1, 2, 3, 4] are fully passable
    // Tile 62
    tile_collision_masks[62][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[62][7] = 0x03;  // □□□□□□██
    // Rows [0, 1, 2, 3, 4, 5] are fully passable
    // Tile 63
    tile_collision_masks[63][5] = 0x01;  // □□□□□□□█
    tile_collision_masks[63][6] = 0x03;  // □□□□□□██
    tile_collision_masks[63][7] = 0x0F;  // □□□□████
    // Rows [0, 1, 2, 3, 4] are fully passable
    // Tile 64
    tile_collision_masks[64][4] = 0x01;  // □□□□□□□█
    tile_collision_masks[64][5] = 0xFF;  // ████████
    tile_collision_masks[64][6] = 0xFF;  // ████████
    tile_collision_masks[64][7] = 0xFF;  // ████████
    // Rows [0, 1, 2, 3] are fully passable
    // Tile 65
    tile_collision_masks[65][3] = 0x03;  // □□□□□□██
    tile_collision_masks[65][4] = 0xFF;  // ████████
    tile_collision_masks[65][5] = 0xFF;  // ████████
    tile_collision_masks[65][6] = 0xFF;  // ████████
    tile_collision_masks[65][7] = 0xFF;  // ████████
    // Rows [0, 1, 2] are fully passable
    // Tile 66
    tile_collision_masks[66][1] = 0x03;  // □□□□□□██
    tile_collision_masks[66][2] = 0x3F;  // □□██████
    tile_collision_masks[66][3] = 0xFF;  // ████████
    tile_collision_masks[66][4] = 0xFF;  // ████████
    tile_collision_masks[66][5] = 0xFF;  // ████████
    tile_collision_masks[66][6] = 0xFF;  // ████████
    tile_collision_masks[66][7] = 0xFF;  // ████████
    // Rows [0] are fully passable
    // Tile 67
    tile_collision_masks[67][0] = 0xC0;  // ██□□□□□□
    // Rows [1, 2, 3, 4, 5, 6, 7] are fully passable
    // Tile 68
    tile_collision_masks[68][0] = 0x80;  // █□□□□□□□
    tile_collision_masks[68][1] = 0x80;  // █□□□□□□□
    // Rows [2, 3, 4, 5, 6, 7] are fully passable
    // Tile 69
    tile_collision_masks[69][1] = 0xFF;  // ████████
    // Rows [0, 2, 3, 4, 5, 6, 7] are fully passable
    // Tile 70
    tile_collision_masks[70][0] = 0x06;  // □□□□□██□
    tile_collision_masks[70][1] = 0x01;  // □□□□□□□█
    // Rows [2, 3, 4, 5, 6, 7] are fully passable
    // Tile 71
    tile_collision_masks[71][0] = 0x02;  // □□□□□□█□
    tile_collision_masks[71][1] = 0x02;  // □□□□□□█□
    tile_collision_masks[71][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[71][3] = 0x01;  // □□□□□□□█
    // Rows [4, 5, 6, 7] are fully passable
    // Tile 72
    tile_collision_masks[72][0] = 0x40;  // □█□□□□□□
    tile_collision_masks[72][1] = 0x40;  // □█□□□□□□
    tile_collision_masks[72][2] = 0x40;  // □█□□□□□□
    tile_collision_masks[72][3] = 0x40;  // □█□□□□□□
    tile_collision_masks[72][4] = 0x40;  // □█□□□□□□
    tile_collision_masks[72][5] = 0x40;  // □█□□□□□□
    tile_collision_masks[72][6] = 0x40;  // □█□□□□□□
    tile_collision_masks[72][7] = 0x40;  // □█□□□□□□
    // Tile 73
    tile_collision_masks[73][0] = 0x02;  // □□□□□□█□
    tile_collision_masks[73][1] = 0x02;  // □□□□□□█□
    tile_collision_masks[73][2] = 0x02;  // □□□□□□█□
    tile_collision_masks[73][3] = 0x02;  // □□□□□□█□
    tile_collision_masks[73][4] = 0x02;  // □□□□□□█□
    tile_collision_masks[73][5] = 0x02;  // □□□□□□█□
    tile_collision_masks[73][6] = 0x02;  // □□□□□□█□
    tile_collision_masks[73][7] = 0x02;  // □□□□□□█□
    // Tile 74
    tile_collision_masks[74][1] = 0xFF;  // ████████
    tile_collision_masks[74][2] = 0xFF;  // ████████
    tile_collision_masks[74][3] = 0xFF;  // ████████
    tile_collision_masks[74][4] = 0xFF;  // ████████
    tile_collision_masks[74][5] = 0xFF;  // ████████
    tile_collision_masks[74][6] = 0xFF;  // ████████
    // Rows [0, 7] are fully passable
    // Tile 75
    tile_collision_masks[75][1] = 0xF0;  // ████□□□□
    tile_collision_masks[75][2] = 0xFF;  // ████████
    tile_collision_masks[75][3] = 0xFF;  // ████████
    tile_collision_masks[75][4] = 0xFF;  // ████████
    tile_collision_masks[75][5] = 0xFF;  // ████████
    tile_collision_masks[75][6] = 0xFF;  // ████████
    tile_collision_masks[75][7] = 0x0F;  // □□□□████
    // Rows [0] are fully passable
    // Tile 76
    tile_collision_masks[76][2] = 0xFF;  // ████████
    tile_collision_masks[76][3] = 0xFF;  // ████████
    tile_collision_masks[76][4] = 0xFF;  // ████████
    tile_collision_masks[76][5] = 0xFF;  // ████████
    tile_collision_masks[76][6] = 0xFF;  // ████████
    tile_collision_masks[76][7] = 0xFF;  // ████████
    // Rows [0, 1] are fully passable
    // Tile 77
    tile_collision_masks[77][2] = 0xFF;  // ████████
    // Rows [0, 1, 3, 4, 5, 6, 7] are fully passable
    // Tile 78
    tile_collision_masks[78][3] = 0xFC;  // ██████□□
    tile_collision_masks[78][4] = 0xFE;  // ███████□
    tile_collision_masks[78][5] = 0xFF;  // ████████
    tile_collision_masks[78][6] = 0xFF;  // ████████
    tile_collision_masks[78][7] = 0xFF;  // ████████
    // Rows [0, 1, 2] are fully passable
    // Tile 79
    tile_collision_masks[79][5] = 0xE0;  // ███□□□□□
    tile_collision_masks[79][6] = 0xFF;  // ████████
    tile_collision_masks[79][7] = 0xFF;  // ████████
    // Rows [0, 1, 2, 3, 4] are fully passable
    // Tile 80
    tile_collision_masks[80][3] = 0xC0;  // ██□□□□□□
    tile_collision_masks[80][4] = 0xE0;  // ███□□□□□
    tile_collision_masks[80][5] = 0xF0;  // ████□□□□
    tile_collision_masks[80][6] = 0xF8;  // █████□□□
    tile_collision_masks[80][7] = 0xFC;  // ██████□□
    // Rows [0, 1, 2] are fully passable
    // Tile 81
    tile_collision_masks[81][5] = 0x0F;  // □□□□████
    tile_collision_masks[81][6] = 0xFF;  // ████████
    tile_collision_masks[81][7] = 0xFF;  // ████████
    // Rows [0, 1, 2, 3, 4] are fully passable
    // Tile 82
    tile_collision_masks[82][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[82][3] = 0x7F;  // □███████
    tile_collision_masks[82][4] = 0xFF;  // ████████
    tile_collision_masks[82][5] = 0xFF;  // ████████
    tile_collision_masks[82][6] = 0xFF;  // ████████
    tile_collision_masks[82][7] = 0xFF;  // ████████
    // Rows [0, 1] are fully passable
    // Tile 83
    tile_collision_masks[83][1] = 0x1F;  // □□□█████
    tile_collision_masks[83][2] = 0xFF;  // ████████
    tile_collision_masks[83][3] = 0xFF;  // ████████
    tile_collision_masks[83][4] = 0xFF;  // ████████
    tile_collision_masks[83][5] = 0xFF;  // ████████
    tile_collision_masks[83][6] = 0xFF;  // ████████
    tile_collision_masks[83][7] = 0xE0;  // ███□□□□□
    // Rows [0] are fully passable
    // Tile 84
    tile_collision_masks[84][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[84][3] = 0x07;  // □□□□□███
    tile_collision_masks[84][4] = 0x0F;  // □□□□████
    tile_collision_masks[84][5] = 0x1F;  // □□□█████
    tile_collision_masks[84][6] = 0x3F;  // □□██████
    tile_collision_masks[84][7] = 0x7F;  // □███████
    // Rows [0, 1] are fully passable
    // Tile 85
    tile_collision_masks[85][0] = 0x80;  // █□□□□□□□
    tile_collision_masks[85][1] = 0x80;  // █□□□□□□□
    tile_collision_masks[85][2] = 0x80;  // █□□□□□□□
    tile_collision_masks[85][3] = 0x80;  // █□□□□□□□
    tile_collision_masks[85][4] = 0x80;  // █□□□□□□□
    tile_collision_masks[85][5] = 0x80;  // █□□□□□□□
    tile_collision_masks[85][6] = 0x80;  // █□□□□□□□
    tile_collision_masks[85][7] = 0x80;  // █□□□□□□□
    // Tile 86
    tile_collision_masks[86][0] = 0x01;  // □□□□□□□█
    tile_collision_masks[86][1] = 0x01;  // □□□□□□□█
    tile_collision_masks[86][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[86][3] = 0x01;  // □□□□□□□█
    tile_collision_masks[86][4] = 0x01;  // □□□□□□□█
    tile_collision_masks[86][5] = 0x01;  // □□□□□□□█
    tile_collision_masks[86][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[86][7] = 0x01;  // □□□□□□□█
    // Tile 87
    tile_collision_masks[87][0] = 0x78;  // □████□□□
    tile_collision_masks[87][1] = 0x78;  // □████□□□
    tile_collision_masks[87][2] = 0x78;  // □████□□□
    tile_collision_masks[87][3] = 0x68;  // □██□█□□□
    tile_collision_masks[87][4] = 0x68;  // □██□█□□□
    tile_collision_masks[87][5] = 0x68;  // □██□█□□□
    tile_collision_masks[87][6] = 0x68;  // □██□█□□□
    tile_collision_masks[87][7] = 0x78;  // □████□□□
    // Tile 88
    tile_collision_masks[88][0] = 0x0E;  // □□□□███□
    tile_collision_masks[88][1] = 0x0E;  // □□□□███□
    tile_collision_masks[88][2] = 0x0E;  // □□□□███□
    tile_collision_masks[88][3] = 0x0E;  // □□□□███□
    tile_collision_masks[88][4] = 0x18;  // □□□██□□□
    tile_collision_masks[88][5] = 0x18;  // □□□██□□□
    tile_collision_masks[88][6] = 0x1C;  // □□□███□□
    tile_collision_masks[88][7] = 0x1C;  // □□□███□□
    // Tile 89
    tile_collision_masks[89][0] = 0x3C;  // □□████□□
    tile_collision_masks[89][1] = 0x3C;  // □□████□□
    tile_collision_masks[89][2] = 0x3C;  // □□████□□
    tile_collision_masks[89][3] = 0x34;  // □□██□█□□
    tile_collision_masks[89][4] = 0x34;  // □□██□█□□
    tile_collision_masks[89][5] = 0x34;  // □□██□█□□
    tile_collision_masks[89][6] = 0x3C;  // □□████□□
    tile_collision_masks[89][7] = 0x78;  // □████□□□
    // Tile 90
    tile_collision_masks[90][0] = 0xF0;  // ████□□□□
    tile_collision_masks[90][1] = 0xF0;  // ████□□□□
    tile_collision_masks[90][2] = 0xF0;  // ████□□□□
    tile_collision_masks[90][3] = 0xD0;  // ██□█□□□□
    tile_collision_masks[90][4] = 0xD0;  // ██□█□□□□
    tile_collision_masks[90][5] = 0xA0;  // █□█□□□□□
    tile_collision_masks[90][6] = 0xA0;  // █□█□□□□□
    tile_collision_masks[90][7] = 0xE0;  // ███□□□□□
    // Tile 91
    tile_collision_masks[91][0] = 0xC0;  // ██□□□□□□
    tile_collision_masks[91][1] = 0x40;  // □█□□□□□□
    tile_collision_masks[91][2] = 0x80;  // █□□□□□□□
    // Rows [3, 4, 5, 6, 7] are fully passable
    // Tile 92
    tile_collision_masks[92][1] = 0x0F;  // □□□□████
    tile_collision_masks[92][2] = 0xFF;  // ████████
    tile_collision_masks[92][3] = 0xF7;  // ████□███
    tile_collision_masks[92][4] = 0x83;  // █□□□□□██
    tile_collision_masks[92][5] = 0xC1;  // ██□□□□□█
    tile_collision_masks[92][6] = 0xE0;  // ███□□□□□
    tile_collision_masks[92][7] = 0x8F;  // █□□□████
    // Rows [0] are fully passable
    // Tile 93
    tile_collision_masks[93][0] = 0x10;  // □□□█□□□□
    tile_collision_masks[93][1] = 0x60;  // □██□□□□□
    tile_collision_masks[93][2] = 0x80;  // █□□□□□□□
    // Rows [3, 4, 5, 6, 7] are fully passable
    // Tile 94
    tile_collision_masks[94][0] = 0x01;  // □□□□□□□█
    tile_collision_masks[94][1] = 0x01;  // □□□□□□□█
    tile_collision_masks[94][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[94][3] = 0x03;  // □□□□□□██
    tile_collision_masks[94][4] = 0x03;  // □□□□□□██
    tile_collision_masks[94][5] = 0x03;  // □□□□□□██
    tile_collision_masks[94][6] = 0x03;  // □□□□□□██
    tile_collision_masks[94][7] = 0x03;  // □□□□□□██
    // Tile 95
    tile_collision_masks[95][0] = 0x07;  // □□□□□███
    tile_collision_masks[95][1] = 0x07;  // □□□□□███
    tile_collision_masks[95][2] = 0x07;  // □□□□□███
    tile_collision_masks[95][3] = 0x07;  // □□□□□███
    tile_collision_masks[95][4] = 0x07;  // □□□□□███
    tile_collision_masks[95][5] = 0x07;  // □□□□□███
    tile_collision_masks[95][6] = 0x07;  // □□□□□███
    tile_collision_masks[95][7] = 0x0F;  // □□□□████
    // Tile 96
    tile_collision_masks[96][0] = 0x0F;  // □□□□████
    tile_collision_masks[96][1] = 0x0F;  // □□□□████
    tile_collision_masks[96][2] = 0x0F;  // □□□□████
    tile_collision_masks[96][3] = 0x0F;  // □□□□████
    tile_collision_masks[96][4] = 0x0F;  // □□□□████
    tile_collision_masks[96][5] = 0x1F;  // □□□█████
    tile_collision_masks[96][6] = 0x1F;  // □□□█████
    tile_collision_masks[96][7] = 0x3F;  // □□██████
    // Tile 97
    tile_collision_masks[97][0] = 0x3F;  // □□██████
    tile_collision_masks[97][1] = 0x7F;  // □███████
    tile_collision_masks[97][2] = 0x7E;  // □██████□
    tile_collision_masks[97][3] = 0x7E;  // □██████□
    tile_collision_masks[97][4] = 0xFE;  // ███████□
    tile_collision_masks[97][5] = 0xFC;  // ██████□□
    tile_collision_masks[97][6] = 0xFC;  // ██████□□
    tile_collision_masks[97][7] = 0xF8;  // █████□□□
    // Tile 98
    tile_collision_masks[98][0] = 0x07;  // □□□□□███
    tile_collision_masks[98][1] = 0x0F;  // □□□□████
    tile_collision_masks[98][2] = 0x1F;  // □□□█████
    tile_collision_masks[98][3] = 0x3F;  // □□██████
    tile_collision_masks[98][4] = 0x7F;  // □███████
    tile_collision_masks[98][5] = 0xFF;  // ████████
    tile_collision_masks[98][6] = 0xFF;  // ████████
    tile_collision_masks[98][7] = 0xFF;  // ████████
    // Tile 99
    tile_collision_masks[99][5] = 0x1F;  // □□□█████
    tile_collision_masks[99][6] = 0xFF;  // ████████
    tile_collision_masks[99][7] = 0xFF;  // ████████
    // Rows [0, 1, 2, 3, 4] are fully passable
    // Tile 100
    tile_collision_masks[100][0] = 0x3F;  // □□██████
    tile_collision_masks[100][1] = 0xFF;  // ████████
    tile_collision_masks[100][2] = 0xFF;  // ████████
    tile_collision_masks[100][3] = 0xFF;  // ████████
    tile_collision_masks[100][4] = 0xFF;  // ████████
    tile_collision_masks[100][5] = 0xFF;  // ████████
    tile_collision_masks[100][6] = 0xFF;  // ████████
    tile_collision_masks[100][7] = 0xFE;  // ███████□
    // Tile 101
    tile_collision_masks[101][2] = 0x80;  // █□□□□□□□
    tile_collision_masks[101][3] = 0xC0;  // ██□□□□□□
    tile_collision_masks[101][4] = 0xE0;  // ███□□□□□
    tile_collision_masks[101][5] = 0xE0;  // ███□□□□□
    tile_collision_masks[101][6] = 0xF0;  // ████□□□□
    tile_collision_masks[101][7] = 0xF8;  // █████□□□
    // Rows [0, 1] are fully passable
    // Tile 102
    tile_collision_masks[102][7] = 0xE0;  // ███□□□□□
    // Rows [0, 1, 2, 3, 4, 5, 6] are fully passable
    // Tile 103
    tile_collision_masks[103][1] = 0x80;  // █□□□□□□□
    tile_collision_masks[103][2] = 0x80;  // █□□□□□□□
    tile_collision_masks[103][3] = 0x80;  // █□□□□□□□
    tile_collision_masks[103][4] = 0x80;  // █□□□□□□□
    tile_collision_masks[103][5] = 0x80;  // █□□□□□□□
    tile_collision_masks[103][6] = 0x80;  // █□□□□□□□
    tile_collision_masks[103][7] = 0x80;  // █□□□□□□□
    // Rows [0] are fully passable
    // Tile 104
    tile_collision_masks[104][0] = 0x80;  // █□□□□□□□
    tile_collision_masks[104][1] = 0xC0;  // ██□□□□□□
    tile_collision_masks[104][2] = 0xC0;  // ██□□□□□□
    tile_collision_masks[104][3] = 0xC0;  // ██□□□□□□
    tile_collision_masks[104][4] = 0xC0;  // ██□□□□□□
    tile_collision_masks[104][5] = 0xC0;  // ██□□□□□□
    tile_collision_masks[104][6] = 0xC0;  // ██□□□□□□
    tile_collision_masks[104][7] = 0xC0;  // ██□□□□□□
    // Tile 106
    tile_collision_masks[106][0] = 0x05;  // □□□□□█□█
    tile_collision_masks[106][1] = 0x05;  // □□□□□█□█
    tile_collision_masks[106][2] = 0x05;  // □□□□□█□█
    tile_collision_masks[106][3] = 0x05;  // □□□□□█□█
    tile_collision_masks[106][4] = 0x05;  // □□□□□█□█
    tile_collision_masks[106][5] = 0x05;  // □□□□□█□█
    tile_collision_masks[106][6] = 0x05;  // □□□□□█□█
    tile_collision_masks[106][7] = 0x05;  // □□□□□█□█
    // Tile 107
    tile_collision_masks[107][0] = 0xC0;  // ██□□□□□□
    tile_collision_masks[107][1] = 0xC0;  // ██□□□□□□
    tile_collision_masks[107][2] = 0xC0;  // ██□□□□□□
    tile_collision_masks[107][3] = 0xF0;  // ████□□□□
    tile_collision_masks[107][4] = 0xF0;  // ████□□□□
    tile_collision_masks[107][5] = 0xF0;  // ████□□□□
    tile_collision_masks[107][6] = 0xF0;  // ████□□□□
    tile_collision_masks[107][7] = 0xF0;  // ████□□□□
    // Tile 108
    tile_collision_masks[108][0] = 0x0B;  // □□□□█□██
    tile_collision_masks[108][1] = 0x0B;  // □□□□█□██
    tile_collision_masks[108][2] = 0x0B;  // □□□□█□██
    tile_collision_masks[108][3] = 0x0B;  // □□□□█□██
    tile_collision_masks[108][4] = 0x0B;  // □□□□█□██
    tile_collision_masks[108][5] = 0x0B;  // □□□□█□██
    tile_collision_masks[108][6] = 0x0B;  // □□□□█□██
    tile_collision_masks[108][7] = 0x0B;  // □□□□█□██
    // Tile 109
    tile_collision_masks[109][0] = 0x17;  // □□□█□███
    tile_collision_masks[109][1] = 0x17;  // □□□█□███
    tile_collision_masks[109][2] = 0x17;  // □□□█□███
    tile_collision_masks[109][3] = 0x17;  // □□□█□███
    tile_collision_masks[109][4] = 0x17;  // □□□█□███
    tile_collision_masks[109][5] = 0x17;  // □□□█□███
    tile_collision_masks[109][6] = 0x17;  // □□□█□███
    tile_collision_masks[109][7] = 0x17;  // □□□█□███
    // Tile 110
    tile_collision_masks[110][0] = 0x05;  // □□□□□█□█
    tile_collision_masks[110][1] = 0x01;  // □□□□□□□█
    tile_collision_masks[110][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[110][3] = 0x01;  // □□□□□□□█
    tile_collision_masks[110][4] = 0x01;  // □□□□□□□█
    tile_collision_masks[110][5] = 0x01;  // □□□□□□□█
    tile_collision_masks[110][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[110][7] = 0x01;  // □□□□□□□█
    // Tile 111
    tile_collision_masks[111][0] = 0xE0;  // ███□□□□□
    tile_collision_masks[111][1] = 0xE0;  // ███□□□□□
    tile_collision_masks[111][2] = 0xE0;  // ███□□□□□
    tile_collision_masks[111][3] = 0xE0;  // ███□□□□□
    tile_collision_masks[111][4] = 0xE0;  // ███□□□□□
    tile_collision_masks[111][5] = 0xC0;  // ██□□□□□□
    tile_collision_masks[111][6] = 0xC0;  // ██□□□□□□
    tile_collision_masks[111][7] = 0xC0;  // ██□□□□□□
    // Tile 112
    tile_collision_masks[112][0] = 0x80;  // █□□□□□□□
    tile_collision_masks[112][1] = 0x80;  // █□□□□□□□
    tile_collision_masks[112][2] = 0x80;  // █□□□□□□□
    tile_collision_masks[112][3] = 0x80;  // █□□□□□□□
    tile_collision_masks[112][4] = 0x80;  // █□□□□□□□
    tile_collision_masks[112][5] = 0x80;  // █□□□□□□□
    tile_collision_masks[112][6] = 0x80;  // █□□□□□□□
    tile_collision_masks[112][7] = 0x80;  // █□□□□□□□
    // Tile 113
    tile_collision_masks[113][0] = 0xC0;  // ██□□□□□□
    tile_collision_masks[113][1] = 0xC0;  // ██□□□□□□
    tile_collision_masks[113][2] = 0xC0;  // ██□□□□□□
    tile_collision_masks[113][3] = 0xC0;  // ██□□□□□□
    tile_collision_masks[113][4] = 0xC0;  // ██□□□□□□
    tile_collision_masks[113][5] = 0xC0;  // ██□□□□□□
    tile_collision_masks[113][6] = 0xC0;  // ██□□□□□□
    tile_collision_masks[113][7] = 0x80;  // █□□□□□□□
    // Tile 114
    tile_collision_masks[114][6] = 0xFF;  // ████████
    tile_collision_masks[114][7] = 0xFF;  // ████████
    // Rows [0, 1, 2, 3, 4, 5] are fully passable
    // Tile 115
    tile_collision_masks[115][0] = 0xFF;  // ████████
    tile_collision_masks[115][1] = 0xFF;  // ████████
    tile_collision_masks[115][2] = 0xFF;  // ████████
    tile_collision_masks[115][3] = 0xFF;  // ████████
    tile_collision_masks[115][4] = 0xFF;  // ████████
    tile_collision_masks[115][5] = 0xFF;  // ████████
    tile_collision_masks[115][6] = 0xFF;  // ████████
    tile_collision_masks[115][7] = 0xFF;  // ████████
    // Tile 117
    tile_collision_masks[117][0] = 0xF0;  // ████□□□□
    tile_collision_masks[117][1] = 0xF0;  // ████□□□□
    tile_collision_masks[117][2] = 0xF0;  // ████□□□□
    tile_collision_masks[117][3] = 0xF0;  // ████□□□□
    tile_collision_masks[117][4] = 0xF0;  // ████□□□□
    tile_collision_masks[117][5] = 0xF0;  // ████□□□□
    tile_collision_masks[117][6] = 0xF0;  // ████□□□□
    tile_collision_masks[117][7] = 0xF0;  // ████□□□□
    // Tile 118
    tile_collision_masks[118][0] = 0xF8;  // █████□□□
    tile_collision_masks[118][1] = 0xF8;  // █████□□□
    tile_collision_masks[118][2] = 0xF8;  // █████□□□
    tile_collision_masks[118][3] = 0xF8;  // █████□□□
    tile_collision_masks[118][4] = 0xF8;  // █████□□□
    tile_collision_masks[118][5] = 0xF8;  // █████□□□
    tile_collision_masks[118][6] = 0xF8;  // █████□□□
    tile_collision_masks[118][7] = 0xF8;  // █████□□□
    // Tile 119
    tile_collision_masks[119][0] = 0x7C;  // □█████□□
    tile_collision_masks[119][1] = 0x7C;  // □█████□□
    tile_collision_masks[119][2] = 0x7C;  // □█████□□
    tile_collision_masks[119][3] = 0x7C;  // □█████□□
    tile_collision_masks[119][4] = 0x7C;  // □█████□□
    tile_collision_masks[119][5] = 0x7C;  // □█████□□
    tile_collision_masks[119][6] = 0x7C;  // □█████□□
    tile_collision_masks[119][7] = 0x7C;  // □█████□□
    // Tile 120
    tile_collision_masks[120][0] = 0xBE;  // █□█████□
    tile_collision_masks[120][1] = 0xBE;  // █□█████□
    tile_collision_masks[120][2] = 0xBE;  // █□█████□
    tile_collision_masks[120][3] = 0xBE;  // █□█████□
    tile_collision_masks[120][4] = 0xBE;  // █□█████□
    tile_collision_masks[120][5] = 0xBE;  // █□█████□
    tile_collision_masks[120][6] = 0xBE;  // █□█████□
    tile_collision_masks[120][7] = 0xBE;  // █□█████□
    // Tile 121
    tile_collision_masks[121][0] = 0x5F;  // □█□█████
    tile_collision_masks[121][1] = 0x5F;  // □█□█████
    tile_collision_masks[121][2] = 0x5F;  // □█□█████
    tile_collision_masks[121][3] = 0x5F;  // □█□█████
    tile_collision_masks[121][4] = 0x5F;  // □█□█████
    tile_collision_masks[121][5] = 0x5F;  // □█□█████
    tile_collision_masks[121][6] = 0x5F;  // □█□█████
    tile_collision_masks[121][7] = 0x5F;  // □█□█████
    // Tile 122
    tile_collision_masks[122][0] = 0x2F;  // □□█□████
    tile_collision_masks[122][1] = 0x2F;  // □□█□████
    tile_collision_masks[122][2] = 0x2F;  // □□█□████
    tile_collision_masks[122][3] = 0x2F;  // □□█□████
    tile_collision_masks[122][4] = 0x2F;  // □□█□████
    tile_collision_masks[122][5] = 0x2F;  // □□█□████
    tile_collision_masks[122][6] = 0x2F;  // □□█□████
    tile_collision_masks[122][7] = 0x2F;  // □□█□████
    // Tile 123
    tile_collision_masks[123][0] = 0xC0;  // ██□□□□□□
    tile_collision_masks[123][1] = 0xC0;  // ██□□□□□□
    tile_collision_masks[123][2] = 0xC0;  // ██□□□□□□
    tile_collision_masks[123][3] = 0xC0;  // ██□□□□□□
    tile_collision_masks[123][4] = 0xC0;  // ██□□□□□□
    tile_collision_masks[123][5] = 0xC0;  // ██□□□□□□
    tile_collision_masks[123][6] = 0xC0;  // ██□□□□□□
    tile_collision_masks[123][7] = 0xC0;  // ██□□□□□□
    // Tile 124
    tile_collision_masks[124][0] = 0xE0;  // ███□□□□□
    tile_collision_masks[124][1] = 0xE0;  // ███□□□□□
    tile_collision_masks[124][2] = 0xE0;  // ███□□□□□
    tile_collision_masks[124][3] = 0xE0;  // ███□□□□□
    tile_collision_masks[124][4] = 0xE0;  // ███□□□□□
    tile_collision_masks[124][5] = 0xE0;  // ███□□□□□
    tile_collision_masks[124][6] = 0xE0;  // ███□□□□□
    tile_collision_masks[124][7] = 0xE0;  // ███□□□□□
    // Tile 125
    tile_collision_masks[125][0] = 0xE0;  // ███□□□□□
    tile_collision_masks[125][1] = 0xE0;  // ███□□□□□
    tile_collision_masks[125][2] = 0xE0;  // ███□□□□□
    tile_collision_masks[125][3] = 0xE0;  // ███□□□□□
    tile_collision_masks[125][4] = 0xE0;  // ███□□□□□
    tile_collision_masks[125][5] = 0xE0;  // ███□□□□□
    tile_collision_masks[125][6] = 0xE0;  // ███□□□□□
    tile_collision_masks[125][7] = 0xE0;  // ███□□□□□
    // Tile 126
    tile_collision_masks[126][0] = 0xFF;  // ████████
    tile_collision_masks[126][1] = 0xFF;  // ████████
    tile_collision_masks[126][2] = 0xFF;  // ████████
    tile_collision_masks[126][3] = 0xFF;  // ████████
    tile_collision_masks[126][4] = 0xFF;  // ████████
    tile_collision_masks[126][5] = 0xFF;  // ████████
    tile_collision_masks[126][6] = 0xFF;  // ████████
    tile_collision_masks[126][7] = 0xE0;  // ███□□□□□
    // Tile 127
    tile_collision_masks[127][0] = 0xFF;  // ████████
    tile_collision_masks[127][1] = 0xFF;  // ████████
    tile_collision_masks[127][2] = 0xFF;  // ████████
    tile_collision_masks[127][3] = 0xFF;  // ████████
    tile_collision_masks[127][4] = 0xFF;  // ████████
    tile_collision_masks[127][5] = 0xFF;  // ████████
    tile_collision_masks[127][6] = 0xFE;  // ███████□
    // Rows [7] are fully passable
    // Tile 128
    tile_collision_masks[128][0] = 0xF8;  // █████□□□
    tile_collision_masks[128][1] = 0xF8;  // █████□□□
    tile_collision_masks[128][2] = 0xF0;  // ████□□□□
    tile_collision_masks[128][3] = 0xF0;  // ████□□□□
    tile_collision_masks[128][4] = 0xE0;  // ███□□□□□
    tile_collision_masks[128][5] = 0xC0;  // ██□□□□□□
    tile_collision_masks[128][6] = 0x80;  // █□□□□□□□
    // Rows [7] are fully passable
    // Tile 131
    tile_collision_masks[131][0] = 0xFF;  // ████████
    tile_collision_masks[131][1] = 0xFF;  // ████████
    tile_collision_masks[131][2] = 0xFF;  // ████████
    tile_collision_masks[131][3] = 0xFF;  // ████████
    tile_collision_masks[131][4] = 0xFF;  // ████████
    tile_collision_masks[131][5] = 0xFF;  // ████████
    // Rows [6, 7] are fully passable
    // Tile 133
    tile_collision_masks[133][0] = 0x80;  // █□□□□□□□
    tile_collision_masks[133][1] = 0x80;  // █□□□□□□□
    tile_collision_masks[133][2] = 0x80;  // █□□□□□□□
    tile_collision_masks[133][3] = 0x80;  // █□□□□□□□
    tile_collision_masks[133][4] = 0x80;  // █□□□□□□□
    tile_collision_masks[133][5] = 0x80;  // █□□□□□□□
    // Rows [6, 7] are fully passable
    // Tile 135
    tile_collision_masks[135][0] = 0xFE;  // ███████□
    tile_collision_masks[135][1] = 0xFC;  // ██████□□
    tile_collision_masks[135][2] = 0xF8;  // █████□□□
    tile_collision_masks[135][3] = 0xF0;  // ████□□□□
    tile_collision_masks[135][4] = 0xE0;  // ███□□□□□
    tile_collision_masks[135][5] = 0xC0;  // ██□□□□□□
    // Rows [6, 7] are fully passable
    // Tile 137
    tile_collision_masks[137][0] = 0xFC;  // ██████□□
    tile_collision_masks[137][1] = 0xF0;  // ████□□□□
    tile_collision_masks[137][2] = 0x80;  // █□□□□□□□
    // Rows [3, 4, 5, 6, 7] are fully passable
    // Tile 141
    tile_collision_masks[141][0] = 0xFF;  // ████████
    tile_collision_masks[141][1] = 0xFF;  // ████████
    tile_collision_masks[141][2] = 0xFF;  // ████████
    tile_collision_masks[141][3] = 0xFE;  // ███████□
    tile_collision_masks[141][4] = 0xF0;  // ████□□□□
    tile_collision_masks[141][5] = 0x80;  // █□□□□□□□
    // Rows [6, 7] are fully passable
    // Tile 142
    tile_collision_masks[142][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[142][7] = 0x0F;  // □□□□████
    // Rows [0, 1, 2, 3, 4, 5] are fully passable
    // Tile 143
    tile_collision_masks[143][1] = 0x01;  // □□□□□□□█
    tile_collision_masks[143][2] = 0x03;  // □□□□□□██
    tile_collision_masks[143][3] = 0x07;  // □□□□□███
    tile_collision_masks[143][4] = 0x0F;  // □□□□████
    tile_collision_masks[143][5] = 0x0F;  // □□□□████
    tile_collision_masks[143][6] = 0x1F;  // □□□█████
    tile_collision_masks[143][7] = 0x3F;  // □□██████
    // Rows [0] are fully passable
    // Tile 144
    tile_collision_masks[144][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[144][7] = 0x01;  // □□□□□□□█
    // Rows [0, 1, 2, 3, 4, 5] are fully passable
    // Tile 145
    tile_collision_masks[145][0] = 0x01;  // □□□□□□□█
    tile_collision_masks[145][1] = 0x03;  // □□□□□□██
    tile_collision_masks[145][2] = 0x03;  // □□□□□□██
    tile_collision_masks[145][3] = 0x03;  // □□□□□□██
    tile_collision_masks[145][4] = 0x03;  // □□□□□□██
    tile_collision_masks[145][5] = 0x03;  // □□□□□□██
    tile_collision_masks[145][6] = 0x03;  // □□□□□□██
    tile_collision_masks[145][7] = 0x03;  // □□□□□□██
    // Tile 146
    tile_collision_masks[146][0] = 0x03;  // □□□□□□██
    tile_collision_masks[146][1] = 0x07;  // □□□□□███
    tile_collision_masks[146][2] = 0x07;  // □□□□□███
    tile_collision_masks[146][3] = 0x07;  // □□□□□███
    tile_collision_masks[146][4] = 0x07;  // □□□□□███
    tile_collision_masks[146][5] = 0x07;  // □□□□□███
    tile_collision_masks[146][6] = 0x07;  // □□□□□███
    tile_collision_masks[146][7] = 0x07;  // □□□□□███
    // Tile 147
    tile_collision_masks[147][0] = 0x07;  // □□□□□███
    tile_collision_masks[147][1] = 0x07;  // □□□□□███
    tile_collision_masks[147][2] = 0x07;  // □□□□□███
    tile_collision_masks[147][3] = 0x1F;  // □□□█████
    tile_collision_masks[147][4] = 0x1F;  // □□□█████
    tile_collision_masks[147][5] = 0x1F;  // □□□█████
    tile_collision_masks[147][6] = 0x1F;  // □□□█████
    tile_collision_masks[147][7] = 0x1F;  // □□□█████
    // Tile 148
    tile_collision_masks[148][0] = 0x1F;  // □□□█████
    tile_collision_masks[148][1] = 0x1F;  // □□□█████
    tile_collision_masks[148][2] = 0x1F;  // □□□█████
    tile_collision_masks[148][3] = 0x1F;  // □□□█████
    tile_collision_masks[148][4] = 0x1F;  // □□□█████
    tile_collision_masks[148][5] = 0x1F;  // □□□█████
    tile_collision_masks[148][6] = 0x1F;  // □□□█████
    tile_collision_masks[148][7] = 0x1F;  // □□□█████
    // Tile 149
    tile_collision_masks[149][0] = 0x3E;  // □□█████□
    tile_collision_masks[149][1] = 0x3E;  // □□█████□
    tile_collision_masks[149][2] = 0x3E;  // □□█████□
    tile_collision_masks[149][3] = 0x3E;  // □□█████□
    tile_collision_masks[149][4] = 0x3E;  // □□█████□
    tile_collision_masks[149][5] = 0x3E;  // □□█████□
    tile_collision_masks[149][6] = 0x3E;  // □□█████□
    tile_collision_masks[149][7] = 0x3E;  // □□█████□
    // Tile 150
    tile_collision_masks[150][0] = 0x7D;  // □█████□█
    tile_collision_masks[150][1] = 0x7D;  // □█████□█
    tile_collision_masks[150][2] = 0x7D;  // □█████□█
    tile_collision_masks[150][3] = 0x7D;  // □█████□█
    tile_collision_masks[150][4] = 0x7D;  // □█████□█
    tile_collision_masks[150][5] = 0x7D;  // □█████□█
    tile_collision_masks[150][6] = 0x7D;  // □█████□█
    tile_collision_masks[150][7] = 0x7D;  // □█████□█
    // Tile 151
    tile_collision_masks[151][0] = 0xFA;  // █████□█□
    tile_collision_masks[151][1] = 0xFA;  // █████□█□
    tile_collision_masks[151][2] = 0xFA;  // █████□█□
    tile_collision_masks[151][3] = 0xFA;  // █████□█□
    tile_collision_masks[151][4] = 0xFA;  // █████□█□
    tile_collision_masks[151][5] = 0xFA;  // █████□█□
    tile_collision_masks[151][6] = 0xFA;  // █████□█□
    tile_collision_masks[151][7] = 0xFA;  // █████□█□
    // Tile 152
    tile_collision_masks[152][0] = 0x01;  // □□□□□□□█
    tile_collision_masks[152][1] = 0x01;  // □□□□□□□█
    tile_collision_masks[152][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[152][3] = 0x01;  // □□□□□□□█
    tile_collision_masks[152][4] = 0x01;  // □□□□□□□█
    tile_collision_masks[152][5] = 0x01;  // □□□□□□□█
    tile_collision_masks[152][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[152][7] = 0x01;  // □□□□□□□█
    // Tile 153
    tile_collision_masks[153][0] = 0xF4;  // ████□█□□
    tile_collision_masks[153][1] = 0xF4;  // ████□█□□
    tile_collision_masks[153][2] = 0xF4;  // ████□█□□
    tile_collision_masks[153][3] = 0xF4;  // ████□█□□
    tile_collision_masks[153][4] = 0xF4;  // ████□█□□
    tile_collision_masks[153][5] = 0xF4;  // ████□█□□
    tile_collision_masks[153][6] = 0xF4;  // ████□█□□
    tile_collision_masks[153][7] = 0xF4;  // ████□█□□
    // Tile 154
    tile_collision_masks[154][0] = 0x03;  // □□□□□□██
    tile_collision_masks[154][1] = 0x03;  // □□□□□□██
    tile_collision_masks[154][2] = 0x03;  // □□□□□□██
    tile_collision_masks[154][3] = 0x03;  // □□□□□□██
    tile_collision_masks[154][4] = 0x03;  // □□□□□□██
    tile_collision_masks[154][5] = 0x03;  // □□□□□□██
    tile_collision_masks[154][6] = 0x03;  // □□□□□□██
    tile_collision_masks[154][7] = 0x03;  // □□□□□□██
    // Tile 155
    tile_collision_masks[155][0] = 0xE8;  // ███□█□□□
    tile_collision_masks[155][1] = 0xE8;  // ███□█□□□
    tile_collision_masks[155][2] = 0xE8;  // ███□█□□□
    tile_collision_masks[155][3] = 0xE8;  // ███□█□□□
    tile_collision_masks[155][4] = 0xE8;  // ███□█□□□
    tile_collision_masks[155][5] = 0xE8;  // ███□█□□□
    tile_collision_masks[155][6] = 0xE8;  // ███□█□□□
    tile_collision_masks[155][7] = 0xE8;  // ███□█□□□
    // Tile 156
    tile_collision_masks[156][0] = 0x07;  // □□□□□███
    tile_collision_masks[156][1] = 0x07;  // □□□□□███
    tile_collision_masks[156][2] = 0x07;  // □□□□□███
    tile_collision_masks[156][3] = 0x07;  // □□□□□███
    tile_collision_masks[156][4] = 0x07;  // □□□□□███
    tile_collision_masks[156][5] = 0x07;  // □□□□□███
    tile_collision_masks[156][6] = 0x07;  // □□□□□███
    tile_collision_masks[156][7] = 0x07;  // □□□□□███
    // Tile 157
    tile_collision_masks[157][0] = 0xD0;  // ██□█□□□□
    tile_collision_masks[157][1] = 0xD0;  // ██□█□□□□
    tile_collision_masks[157][2] = 0xD0;  // ██□█□□□□
    tile_collision_masks[157][3] = 0xD0;  // ██□█□□□□
    tile_collision_masks[157][4] = 0xD0;  // ██□█□□□□
    tile_collision_masks[157][5] = 0xD0;  // ██□█□□□□
    tile_collision_masks[157][6] = 0xD0;  // ██□█□□□□
    tile_collision_masks[157][7] = 0xD0;  // ██□█□□□□
    // Tile 158
    tile_collision_masks[158][0] = 0x0F;  // □□□□████
    tile_collision_masks[158][1] = 0x0F;  // □□□□████
    tile_collision_masks[158][2] = 0x0F;  // □□□□████
    tile_collision_masks[158][3] = 0x0F;  // □□□□████
    tile_collision_masks[158][4] = 0x0F;  // □□□□████
    tile_collision_masks[158][5] = 0x0F;  // □□□□████
    tile_collision_masks[158][6] = 0x0F;  // □□□□████
    tile_collision_masks[158][7] = 0x0F;  // □□□□████
    // Tile 159
    tile_collision_masks[159][0] = 0xA0;  // █□█□□□□□
    tile_collision_masks[159][1] = 0xA0;  // █□█□□□□□
    tile_collision_masks[159][2] = 0xA0;  // █□█□□□□□
    tile_collision_masks[159][3] = 0xA0;  // █□█□□□□□
    tile_collision_masks[159][4] = 0xA0;  // █□█□□□□□
    tile_collision_masks[159][5] = 0xA0;  // █□█□□□□□
    tile_collision_masks[159][6] = 0xA0;  // █□█□□□□□
    tile_collision_masks[159][7] = 0xA0;  // █□█□□□□□
    // Tile 160
    tile_collision_masks[160][0] = 0x0F;  // □□□□████
    tile_collision_masks[160][1] = 0x0F;  // □□□□████
    tile_collision_masks[160][2] = 0x0F;  // □□□□████
    tile_collision_masks[160][3] = 0x0F;  // □□□□████
    tile_collision_masks[160][4] = 0x0F;  // □□□□████
    tile_collision_masks[160][5] = 0x0F;  // □□□□████
    tile_collision_masks[160][6] = 0x0F;  // □□□□████
    tile_collision_masks[160][7] = 0x0F;  // □□□□████
    // Tile 161
    tile_collision_masks[161][0] = 0x0F;  // □□□□████
    tile_collision_masks[161][1] = 0x0F;  // □□□□████
    tile_collision_masks[161][2] = 0x0F;  // □□□□████
    tile_collision_masks[161][3] = 0x0F;  // □□□□████
    tile_collision_masks[161][4] = 0x0F;  // □□□□████
    tile_collision_masks[161][5] = 0x0F;  // □□□□████
    tile_collision_masks[161][6] = 0x0F;  // □□□□████
    tile_collision_masks[161][7] = 0x0F;  // □□□□████
    // Tile 162
    tile_collision_masks[162][0] = 0x40;  // □█□□□□□□
    // Rows [1, 2, 3, 4, 5, 6, 7] are fully passable
    // Tile 163
    tile_collision_masks[163][0] = 0x0F;  // □□□□████
    tile_collision_masks[163][1] = 0x0F;  // □□□□████
    tile_collision_masks[163][2] = 0x0F;  // □□□□████
    tile_collision_masks[163][3] = 0x0F;  // □□□□████
    tile_collision_masks[163][4] = 0x0F;  // □□□□████
    tile_collision_masks[163][5] = 0x07;  // □□□□□███
    tile_collision_masks[163][6] = 0x07;  // □□□□□███
    tile_collision_masks[163][7] = 0x07;  // □□□□□███
    // Tile 164
    tile_collision_masks[164][0] = 0x07;  // □□□□□███
    tile_collision_masks[164][1] = 0x07;  // □□□□□███
    tile_collision_masks[164][2] = 0x07;  // □□□□□███
    tile_collision_masks[164][3] = 0x07;  // □□□□□███
    tile_collision_masks[164][4] = 0x07;  // □□□□□███
    tile_collision_masks[164][5] = 0x07;  // □□□□□███
    tile_collision_masks[164][6] = 0x07;  // □□□□□███
    tile_collision_masks[164][7] = 0x03;  // □□□□□□██
    // Tile 165
    tile_collision_masks[165][0] = 0x03;  // □□□□□□██
    tile_collision_masks[165][1] = 0x03;  // □□□□□□██
    tile_collision_masks[165][2] = 0x03;  // □□□□□□██
    tile_collision_masks[165][3] = 0x03;  // □□□□□□██
    tile_collision_masks[165][4] = 0x03;  // □□□□□□██
    tile_collision_masks[165][5] = 0x03;  // □□□□□□██
    tile_collision_masks[165][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[165][7] = 0x01;  // □□□□□□□█
    // Tile 166
    tile_collision_masks[166][0] = 0x01;  // □□□□□□□█
    tile_collision_masks[166][1] = 0x01;  // □□□□□□□█
    // Rows [2, 3, 4, 5, 6, 7] are fully passable
    // Tile 167
    tile_collision_masks[167][7] = 0x80;  // █□□□□□□□
    // Rows [0, 1, 2, 3, 4, 5, 6] are fully passable
    // Tile 168
    tile_collision_masks[168][0] = 0x3F;  // □□██████
    tile_collision_masks[168][1] = 0x3F;  // □□██████
    tile_collision_masks[168][2] = 0x1F;  // □□□█████
    tile_collision_masks[168][3] = 0x1F;  // □□□█████
    tile_collision_masks[168][4] = 0x0F;  // □□□□████
    tile_collision_masks[168][5] = 0x07;  // □□□□□███
    tile_collision_masks[168][6] = 0x03;  // □□□□□□██
    tile_collision_masks[168][7] = 0x01;  // □□□□□□□█
    // Tile 169
    tile_collision_masks[169][6] = 0x80;  // █□□□□□□□
    tile_collision_masks[169][7] = 0xE0;  // ███□□□□□
    // Rows [0, 1, 2, 3, 4, 5] are fully passable
    // Tile 171
    tile_collision_masks[171][0] = 0xFF;  // ████████
    tile_collision_masks[171][1] = 0x7F;  // □███████
    tile_collision_masks[171][2] = 0x3F;  // □□██████
    tile_collision_masks[171][3] = 0x1F;  // □□□█████
    tile_collision_masks[171][4] = 0x0F;  // □□□□████
    tile_collision_masks[171][5] = 0x07;  // □□□□□███
    tile_collision_masks[171][6] = 0x01;  // □□□□□□□█
    // Rows [7] are fully passable
    // Tile 173
    tile_collision_masks[173][0] = 0x7F;  // □███████
    tile_collision_masks[173][1] = 0x1F;  // □□□█████
    tile_collision_masks[173][2] = 0x03;  // □□□□□□██
    // Rows [3, 4, 5, 6, 7] are fully passable
    // Tile 174
    tile_collision_masks[174][0] = 0xFF;  // ████████
    tile_collision_masks[174][1] = 0xFF;  // ████████
    tile_collision_masks[174][2] = 0xFF;  // ████████
    tile_collision_masks[174][3] = 0xFF;  // ████████
    tile_collision_masks[174][4] = 0x1F;  // □□□█████
    tile_collision_masks[174][5] = 0x03;  // □□□□□□██
    // Rows [6, 7] are fully passable
    // Tile 175
    tile_collision_masks[175][0] = 0xFF;  // ████████
    tile_collision_masks[175][1] = 0xFF;  // ████████
    tile_collision_masks[175][2] = 0xFF;  // ████████
    tile_collision_masks[175][3] = 0xFF;  // ████████
    tile_collision_masks[175][4] = 0xFF;  // ████████
    tile_collision_masks[175][5] = 0xFF;  // ████████
    tile_collision_masks[175][6] = 0xFF;  // ████████
    // Rows [7] are fully passable
    // Tile 176
    tile_collision_masks[176][0] = 0xFF;  // ████████
    tile_collision_masks[176][1] = 0xFF;  // ████████
    tile_collision_masks[176][2] = 0xFF;  // ████████
    tile_collision_masks[176][3] = 0xFF;  // ████████
    tile_collision_masks[176][4] = 0xFF;  // ████████
    tile_collision_masks[176][5] = 0xFF;  // ████████
    tile_collision_masks[176][6] = 0xFF;  // ████████
    tile_collision_masks[176][7] = 0x0F;  // □□□□████
    // Tile 182
    tile_collision_masks[182][7] = 0xFF;  // ████████
    // Rows [0, 1, 2, 3, 4, 5, 6] are fully passable
    // Tile 183
    tile_collision_masks[183][0] = 0xFF;  // ████████
    tile_collision_masks[183][1] = 0x7F;  // □███████
    tile_collision_masks[183][2] = 0x7F;  // □███████
    tile_collision_masks[183][3] = 0x80;  // █□□□□□□□
    tile_collision_masks[183][4] = 0xFF;  // ████████
    // Rows [5, 6, 7] are fully passable
    // Tile 184
    tile_collision_masks[184][0] = 0xFF;  // ████████
    tile_collision_masks[184][1] = 0xFC;  // ██████□□
    tile_collision_masks[184][2] = 0xFC;  // ██████□□
    tile_collision_masks[184][4] = 0xFC;  // ██████□□
    // Rows [3, 5, 6, 7] are fully passable
    // Tile 186
    tile_collision_masks[186][7] = 0x3F;  // □□██████
    // Rows [0, 1, 2, 3, 4, 5, 6] are fully passable
    // Tile 187
    tile_collision_masks[187][0] = 0xFF;  // ████████
    tile_collision_masks[187][1] = 0xFF;  // ████████
    tile_collision_masks[187][2] = 0x7F;  // □███████
    tile_collision_masks[187][3] = 0x7F;  // □███████
    tile_collision_masks[187][4] = 0x80;  // █□□□□□□□
    tile_collision_masks[187][5] = 0xFF;  // ████████
    // Rows [6, 7] are fully passable
    // Tile 188
    tile_collision_masks[188][0] = 0x80;  // █□□□□□□□
    // Rows [1, 2, 3, 4, 5, 6, 7] are fully passable
    // Tile 189
    tile_collision_masks[189][7] = 0xF8;  // █████□□□
    // Rows [0, 1, 2, 3, 4, 5, 6] are fully passable
    // Tile 190
    tile_collision_masks[190][0] = 0x0C;  // □□□□██□□
    tile_collision_masks[190][1] = 0x0C;  // □□□□██□□
    tile_collision_masks[190][2] = 0x0C;  // □□□□██□□
    tile_collision_masks[190][3] = 0x0C;  // □□□□██□□
    tile_collision_masks[190][4] = 0x0C;  // □□□□██□□
    tile_collision_masks[190][5] = 0x0C;  // □□□□██□□
    tile_collision_masks[190][6] = 0x0C;  // □□□□██□□
    tile_collision_masks[190][7] = 0x0C;  // □□□□██□□
    // Tile 191
    tile_collision_masks[191][0] = 0x03;  // □□□□□□██
    tile_collision_masks[191][1] = 0x03;  // □□□□□□██
    tile_collision_masks[191][2] = 0x03;  // □□□□□□██
    tile_collision_masks[191][3] = 0x03;  // □□□□□□██
    tile_collision_masks[191][4] = 0x03;  // □□□□□□██
    tile_collision_masks[191][5] = 0x03;  // □□□□□□██
    tile_collision_masks[191][6] = 0x03;  // □□□□□□██
    tile_collision_masks[191][7] = 0x03;  // □□□□□□██
    // Tile 192
    tile_collision_masks[192][0] = 0x01;  // □□□□□□□█
    tile_collision_masks[192][1] = 0x01;  // □□□□□□□█
    tile_collision_masks[192][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[192][3] = 0x01;  // □□□□□□□█
    tile_collision_masks[192][4] = 0x01;  // □□□□□□□█
    tile_collision_masks[192][5] = 0x01;  // □□□□□□□█
    tile_collision_masks[192][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[192][7] = 0x01;  // □□□□□□□█
    // Tile 193
    tile_collision_masks[193][0] = 0x80;  // █□□□□□□□
    tile_collision_masks[193][1] = 0x80;  // █□□□□□□□
    tile_collision_masks[193][2] = 0x80;  // █□□□□□□□
    tile_collision_masks[193][3] = 0x80;  // █□□□□□□□
    tile_collision_masks[193][4] = 0x80;  // █□□□□□□□
    tile_collision_masks[193][5] = 0x80;  // █□□□□□□□
    tile_collision_masks[193][6] = 0x80;  // █□□□□□□□
    tile_collision_masks[193][7] = 0x80;  // █□□□□□□□
    // Tile 194
    tile_collision_masks[194][0] = 0xC0;  // ██□□□□□□
    tile_collision_masks[194][1] = 0xC0;  // ██□□□□□□
    tile_collision_masks[194][2] = 0xC0;  // ██□□□□□□
    tile_collision_masks[194][3] = 0xC0;  // ██□□□□□□
    tile_collision_masks[194][4] = 0xC0;  // ██□□□□□□
    tile_collision_masks[194][5] = 0xC0;  // ██□□□□□□
    tile_collision_masks[194][6] = 0xC0;  // ██□□□□□□
    tile_collision_masks[194][7] = 0xC0;  // ██□□□□□□
    // Tile 195
    tile_collision_masks[195][0] = 0x60;  // □██□□□□□
    tile_collision_masks[195][1] = 0x60;  // □██□□□□□
    tile_collision_masks[195][2] = 0x60;  // □██□□□□□
    tile_collision_masks[195][3] = 0x60;  // □██□□□□□
    tile_collision_masks[195][4] = 0x60;  // □██□□□□□
    tile_collision_masks[195][5] = 0x60;  // □██□□□□□
    tile_collision_masks[195][6] = 0x60;  // □██□□□□□
    tile_collision_masks[195][7] = 0x60;  // □██□□□□□
    // Tile 196
    tile_collision_masks[196][0] = 0x30;  // □□██□□□□
    tile_collision_masks[196][1] = 0x30;  // □□██□□□□
    tile_collision_masks[196][2] = 0x30;  // □□██□□□□
    tile_collision_masks[196][3] = 0x30;  // □□██□□□□
    tile_collision_masks[196][4] = 0x30;  // □□██□□□□
    tile_collision_masks[196][5] = 0x30;  // □□██□□□□
    tile_collision_masks[196][6] = 0x30;  // □□██□□□□
    tile_collision_masks[196][7] = 0x30;  // □□██□□□□
    // Tile 197
    tile_collision_masks[197][0] = 0x18;  // □□□██□□□
    tile_collision_masks[197][1] = 0x18;  // □□□██□□□
    tile_collision_masks[197][2] = 0x18;  // □□□██□□□
    tile_collision_masks[197][3] = 0x18;  // □□□██□□□
    tile_collision_masks[197][4] = 0x18;  // □□□██□□□
    tile_collision_masks[197][5] = 0x18;  // □□□██□□□
    tile_collision_masks[197][6] = 0x18;  // □□□██□□□
    tile_collision_masks[197][7] = 0x18;  // □□□██□□□
    // Tile 198
    tile_collision_masks[198][2] = 0xFF;  // ████████
    tile_collision_masks[198][3] = 0xFF;  // ████████
    tile_collision_masks[198][4] = 0x87;  // █□□□□███
    tile_collision_masks[198][5] = 0xC3;  // ██□□□□██
    tile_collision_masks[198][6] = 0xE1;  // ███□□□□█
    // Rows [0, 1, 7] are fully passable
    // Tile 199
    tile_collision_masks[199][2] = 0xFF;  // ████████
    tile_collision_masks[199][3] = 0xFF;  // ████████
    tile_collision_masks[199][4] = 0x87;  // █□□□□███
    tile_collision_masks[199][5] = 0xC3;  // ██□□□□██
    tile_collision_masks[199][6] = 0xE1;  // ███□□□□█
    tile_collision_masks[199][7] = 0x80;  // █□□□□□□□
    // Rows [0, 1] are fully passable
    // Tile 200
    tile_collision_masks[200][0] = 0xFF;  // ████████
    // Rows [1, 2, 3, 4, 5, 6, 7] are fully passable
    // Tile 201
    tile_collision_masks[201][0] = 0xFF;  // ████████
    // Rows [1, 2, 3, 4, 5, 6, 7] are fully passable
    // Tile 202
    tile_collision_masks[202][0] = 0xF0;  // ████□□□□
    // Rows [1, 2, 3, 4, 5, 6, 7] are fully passable
    // Tile 205
    tile_collision_masks[205][7] = 0x01;  // □□□□□□□█
    // Rows [0, 1, 2, 3, 4, 5, 6] are fully passable
    // Tile 206
    tile_collision_masks[206][5] = 0x03;  // □□□□□□██
    tile_collision_masks[206][6] = 0x3F;  // □□██████
    tile_collision_masks[206][7] = 0xFF;  // ████████
    // Rows [0, 1, 2, 3, 4] are fully passable
    // Tile 207
    tile_collision_masks[207][5] = 0x01;  // □□□□□□□█
    tile_collision_masks[207][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[207][7] = 0x03;  // □□□□□□██
    // Rows [0, 1, 2, 3, 4] are fully passable
    // Tile 211
    tile_collision_masks[211][0] = 0xFF;  // ████████
    tile_collision_masks[211][1] = 0xFF;  // ████████
    tile_collision_masks[211][2] = 0xFC;  // ██████□□
    tile_collision_masks[211][3] = 0xFC;  // ██████□□
    tile_collision_masks[211][5] = 0xFC;  // ██████□□
    // Rows [4, 6, 7] are fully passable
    // Tile 213
    tile_collision_masks[213][0] = 0x30;  // □□██□□□□
    tile_collision_masks[213][1] = 0x30;  // □□██□□□□
    tile_collision_masks[213][2] = 0x30;  // □□██□□□□
    tile_collision_masks[213][3] = 0x30;  // □□██□□□□
    tile_collision_masks[213][4] = 0x30;  // □□██□□□□
    tile_collision_masks[213][5] = 0x30;  // □□██□□□□
    tile_collision_masks[213][6] = 0x30;  // □□██□□□□
    tile_collision_masks[213][7] = 0x30;  // □□██□□□□
    // Tile 214
    tile_collision_masks[214][0] = 0x60;  // □██□□□□□
    tile_collision_masks[214][1] = 0x60;  // □██□□□□□
    tile_collision_masks[214][2] = 0x60;  // □██□□□□□
    tile_collision_masks[214][3] = 0x60;  // □██□□□□□
    tile_collision_masks[214][4] = 0xC0;  // ██□□□□□□
    tile_collision_masks[214][5] = 0xC0;  // ██□□□□□□
    tile_collision_masks[214][6] = 0xC0;  // ██□□□□□□
    tile_collision_masks[214][7] = 0xC0;  // ██□□□□□□
    // Tile 215
    tile_collision_masks[215][0] = 0xC0;  // ██□□□□□□
    tile_collision_masks[215][1] = 0xC0;  // ██□□□□□□
    tile_collision_masks[215][2] = 0xC0;  // ██□□□□□□
    tile_collision_masks[215][3] = 0xC0;  // ██□□□□□□
    tile_collision_masks[215][4] = 0xC0;  // ██□□□□□□
    tile_collision_masks[215][5] = 0xC0;  // ██□□□□□□
    tile_collision_masks[215][6] = 0xC0;  // ██□□□□□□
    tile_collision_masks[215][7] = 0xC0;  // ██□□□□□□
    // Tile 216
    tile_collision_masks[216][0] = 0x01;  // □□□□□□□█
    tile_collision_masks[216][1] = 0x01;  // □□□□□□□█
    tile_collision_masks[216][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[216][3] = 0x01;  // □□□□□□□█
    tile_collision_masks[216][4] = 0x01;  // □□□□□□□█
    tile_collision_masks[216][5] = 0x01;  // □□□□□□□█
    tile_collision_masks[216][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[216][7] = 0x01;  // □□□□□□□█
    // Tile 217
    tile_collision_masks[217][0] = 0x80;  // █□□□□□□□
    tile_collision_masks[217][1] = 0x80;  // █□□□□□□□
    tile_collision_masks[217][2] = 0x80;  // █□□□□□□□
    tile_collision_masks[217][3] = 0x80;  // █□□□□□□□
    tile_collision_masks[217][4] = 0x80;  // █□□□□□□□
    tile_collision_masks[217][5] = 0x80;  // █□□□□□□□
    tile_collision_masks[217][6] = 0x80;  // █□□□□□□□
    tile_collision_masks[217][7] = 0x80;  // █□□□□□□□
    // Tile 218
    tile_collision_masks[218][5] = 0x80;  // █□□□□□□□
    tile_collision_masks[218][6] = 0x80;  // █□□□□□□□
    tile_collision_masks[218][7] = 0xC0;  // ██□□□□□□
    // Rows [0, 1, 2, 3, 4] are fully passable
    // Tile 220
    tile_collision_masks[220][0] = 0x1F;  // □□□█████
    // Rows [1, 2, 3, 4, 5, 6, 7] are fully passable
    // Tile 221
    tile_collision_masks[221][5] = 0xC0;  // ██□□□□□□
    tile_collision_masks[221][6] = 0xFC;  // ██████□□
    tile_collision_masks[221][7] = 0x7F;  // □███████
    // Rows [0, 1, 2, 3, 4] are fully passable
    // Tile 222
    tile_collision_masks[222][7] = 0x80;  // █□□□□□□□
    // Rows [0, 1, 2, 3, 4, 5, 6] are fully passable
    // Tile 227
    tile_collision_masks[227][0] = 0x30;  // □□██□□□□
    tile_collision_masks[227][1] = 0x30;  // □□██□□□□
    tile_collision_masks[227][2] = 0x30;  // □□██□□□□
    tile_collision_masks[227][3] = 0x30;  // □□██□□□□
    tile_collision_masks[227][4] = 0x30;  // □□██□□□□
    tile_collision_masks[227][5] = 0x30;  // □□██□□□□
    tile_collision_masks[227][6] = 0x30;  // □□██□□□□
    tile_collision_masks[227][7] = 0x30;  // □□██□□□□
    // Tile 228
    tile_collision_masks[228][0] = 0x0C;  // □□□□██□□
    tile_collision_masks[228][1] = 0x0C;  // □□□□██□□
    tile_collision_masks[228][2] = 0x0C;  // □□□□██□□
    tile_collision_masks[228][3] = 0x0C;  // □□□□██□□
    tile_collision_masks[228][4] = 0x0C;  // □□□□██□□
    tile_collision_masks[228][5] = 0x0C;  // □□□□██□□
    tile_collision_masks[228][6] = 0x0C;  // □□□□██□□
    tile_collision_masks[228][7] = 0x0C;  // □□□□██□□
    // Tile 231
    tile_collision_masks[231][0] = 0x18;  // □□□██□□□
    tile_collision_masks[231][1] = 0x18;  // □□□██□□□
    tile_collision_masks[231][2] = 0x18;  // □□□██□□□
    tile_collision_masks[231][3] = 0x18;  // □□□██□□□
    tile_collision_masks[231][4] = 0x18;  // □□□██□□□
    tile_collision_masks[231][5] = 0x18;  // □□□██□□□
    tile_collision_masks[231][6] = 0x18;  // □□□██□□□
    tile_collision_masks[231][7] = 0x18;  // □□□██□□□
    // Tile 232
    tile_collision_masks[232][0] = 0x06;  // □□□□□██□
    tile_collision_masks[232][1] = 0x06;  // □□□□□██□
    tile_collision_masks[232][2] = 0x06;  // □□□□□██□
    tile_collision_masks[232][3] = 0x06;  // □□□□□██□
    tile_collision_masks[232][4] = 0x06;  // □□□□□██□
    tile_collision_masks[232][5] = 0x06;  // □□□□□██□
    tile_collision_masks[232][6] = 0x06;  // □□□□□██□
    tile_collision_masks[232][7] = 0x06;  // □□□□□██□
    // Tile 233
    tile_collision_masks[233][0] = 0x03;  // □□□□□□██
    tile_collision_masks[233][1] = 0x03;  // □□□□□□██
    tile_collision_masks[233][2] = 0x03;  // □□□□□□██
    tile_collision_masks[233][3] = 0x03;  // □□□□□□██
    tile_collision_masks[233][4] = 0x03;  // □□□□□□██
    tile_collision_masks[233][5] = 0x03;  // □□□□□□██
    tile_collision_masks[233][6] = 0x03;  // □□□□□□██
    tile_collision_masks[233][7] = 0x03;  // □□□□□□██
    // Tile 234
    tile_collision_masks[234][0] = 0x01;  // □□□□□□□█
    tile_collision_masks[234][1] = 0x01;  // □□□□□□□█
    tile_collision_masks[234][2] = 0x01;  // □□□□□□□█
    tile_collision_masks[234][3] = 0x01;  // □□□□□□□█
    tile_collision_masks[234][4] = 0x01;  // □□□□□□□█
    tile_collision_masks[234][5] = 0x01;  // □□□□□□□█
    tile_collision_masks[234][6] = 0x01;  // □□□□□□□█
    tile_collision_masks[234][7] = 0x01;  // □□□□□□□█
    // Tile 236
    tile_collision_masks[236][0] = 0x30;  // □□██□□□□
    tile_collision_masks[236][1] = 0x30;  // □□██□□□□
    tile_collision_masks[236][2] = 0x30;  // □□██□□□□
    tile_collision_masks[236][3] = 0x30;  // □□██□□□□
    tile_collision_masks[236][4] = 0x30;  // □□██□□□□
    tile_collision_masks[236][5] = 0x30;  // □□██□□□□
    tile_collision_masks[236][6] = 0x30;  // □□██□□□□
    tile_collision_masks[236][7] = 0x30;  // □□██□□□□
    // Tile 237
    tile_collision_masks[237][0] = 0x18;  // □□□██□□□
    tile_collision_masks[237][1] = 0x18;  // □□□██□□□
    tile_collision_masks[237][2] = 0x18;  // □□□██□□□
    tile_collision_masks[237][3] = 0x18;  // □□□██□□□
    tile_collision_masks[237][4] = 0x18;  // □□□██□□□
    tile_collision_masks[237][5] = 0x18;  // □□□██□□□
    tile_collision_masks[237][6] = 0x18;  // □□□██□□□
    tile_collision_masks[237][7] = 0x18;  // □□□██□□□
    // Tile 238
    tile_collision_masks[238][0] = 0x0C;  // □□□□██□□
    tile_collision_masks[238][1] = 0x0C;  // □□□□██□□
    tile_collision_masks[238][2] = 0x0C;  // □□□□██□□
    tile_collision_masks[238][3] = 0x0C;  // □□□□██□□
    tile_collision_masks[238][4] = 0x0C;  // □□□□██□□
    tile_collision_masks[238][5] = 0x0C;  // □□□□██□□
    tile_collision_masks[238][6] = 0x0C;  // □□□□██□□
    tile_collision_masks[238][7] = 0x0C;  // □□□□██□□
    // Tile 239
    tile_collision_masks[239][0] = 0x06;  // □□□□□██□
    tile_collision_masks[239][1] = 0x06;  // □□□□□██□
    tile_collision_masks[239][2] = 0x06;  // □□□□□██□
    tile_collision_masks[239][3] = 0x06;  // □□□□□██□
    tile_collision_masks[239][4] = 0x06;  // □□□□□██□
    tile_collision_masks[239][5] = 0x06;  // □□□□□██□
    tile_collision_masks[239][6] = 0x06;  // □□□□□██□
    tile_collision_masks[239][7] = 0x06;  // □□□□□██□
    // Tile 240
    tile_collision_masks[240][0] = 0x03;  // □□□□□□██
    tile_collision_masks[240][1] = 0x03;  // □□□□□□██
    tile_collision_masks[240][2] = 0x03;  // □□□□□□██
    tile_collision_masks[240][3] = 0x03;  // □□□□□□██
    tile_collision_masks[240][4] = 0x03;  // □□□□□□██
    tile_collision_masks[240][5] = 0x03;  // □□□□□□██
    tile_collision_masks[240][6] = 0x03;  // □□□□□□██
    tile_collision_masks[240][7] = 0x03;  // □□□□□□██
}

void init_tile_properties(void) {
    tile_properties[0] = TERRAIN_WALL;
    tile_properties[1] = TERRAIN_ROAD;
    tile_properties[2] = TERRAIN_GRASS;
    tile_properties[3] = TERRAIN_WALL;
    tile_properties[4] = TERRAIN_ROAD;
    tile_properties[5] = TERRAIN_ROAD;
    tile_properties[6] = TERRAIN_ROAD;
    tile_properties[7] = TERRAIN_GRASS;
    tile_properties[8] = TERRAIN_ROAD;
    tile_properties[9] = TERRAIN_GRASS;
    tile_properties[10] = TERRAIN_ROAD;
    tile_properties[11] = TERRAIN_GRASS;
    tile_properties[12] = TERRAIN_GRASS;
    tile_properties[13] = TERRAIN_ROAD;
    tile_properties[14] = TERRAIN_ROAD;
    tile_properties[15] = TERRAIN_ROAD;
    tile_properties[16] = TERRAIN_WALL;
    tile_properties[17] = TERRAIN_ROAD;
    tile_properties[18] = TERRAIN_ROAD;
    tile_properties[19] = TERRAIN_WALL;
    tile_properties[20] = TERRAIN_WALL;
    tile_properties[21] = TERRAIN_WALL;
    tile_properties[22] = TERRAIN_WALL;
    tile_properties[23] = TERRAIN_ROAD;
    tile_properties[24] = TERRAIN_ROAD;
    tile_properties[25] = TERRAIN_GRASS;
    tile_properties[26] = TERRAIN_ROAD;
    tile_properties[27] = TERRAIN_ROAD;
    tile_properties[28] = TERRAIN_GRASS;
    tile_properties[29] = TERRAIN_GRASS;
    tile_properties[30] = TERRAIN_WALL;
    tile_properties[31] = TERRAIN_WALL;
    tile_properties[32] = TERRAIN_ROAD;
    tile_properties[33] = TERRAIN_ROAD;
    tile_properties[34] = TERRAIN_ROAD;
    tile_properties[35] = TERRAIN_WALL;
    tile_properties[36] = TERRAIN_ROAD;
    tile_properties[37] = TERRAIN_ROAD;
    tile_properties[38] = TERRAIN_ROAD;
    tile_properties[39] = TERRAIN_WALL;
    tile_properties[40] = TERRAIN_ROAD;
    tile_properties[41] = TERRAIN_ROAD;
    tile_properties[42] = TERRAIN_ROAD;
    tile_properties[43] = TERRAIN_ROAD;
    tile_properties[44] = TERRAIN_ROAD;
    tile_properties[45] = TERRAIN_ROAD;
    tile_properties[46] = TERRAIN_WALL;
    tile_properties[47] = TERRAIN_WALL;
    tile_properties[48] = TERRAIN_WALL;
    tile_properties[49] = TERRAIN_WALL;
    tile_properties[50] = TERRAIN_ROAD;
    tile_properties[51] = TERRAIN_ROAD;
    tile_properties[52] = TERRAIN_ROAD;
    tile_properties[53] = TERRAIN_ROAD;
    tile_properties[54] = TERRAIN_ROAD;
    tile_properties[55] = TERRAIN_WALL;
    tile_properties[56] = TERRAIN_WALL;
    tile_properties[57] = TERRAIN_WALL;
    tile_properties[58] = TERRAIN_WALL;
    tile_properties[59] = TERRAIN_WALL;
    tile_properties[60] = TERRAIN_ROAD;
    tile_properties[61] = TERRAIN_ROAD;
    tile_properties[62] = TERRAIN_ROAD;
    tile_properties[63] = TERRAIN_ROAD;
    tile_properties[64] = TERRAIN_ROAD;
    tile_properties[65] = TERRAIN_WALL;
    tile_properties[66] = TERRAIN_WALL;
    tile_properties[67] = TERRAIN_ROAD;
    tile_properties[68] = TERRAIN_ROAD;
    tile_properties[69] = TERRAIN_ROAD;
    tile_properties[70] = TERRAIN_ROAD;
    tile_properties[71] = TERRAIN_ROAD;
    tile_properties[72] = TERRAIN_ROAD;
    tile_properties[73] = TERRAIN_ROAD;
    tile_properties[74] = TERRAIN_WALL;
    tile_properties[75] = TERRAIN_WALL;
    tile_properties[76] = TERRAIN_WALL;
    tile_properties[77] = TERRAIN_ROAD;
    tile_properties[78] = TERRAIN_WALL;
    tile_properties[79] = TERRAIN_GRASS;
    tile_properties[80] = TERRAIN_GRASS;
    tile_properties[81] = TERRAIN_GRASS;
    tile_properties[82] = TERRAIN_WALL;
    tile_properties[83] = TERRAIN_WALL;
    tile_properties[84] = TERRAIN_GRASS;
    tile_properties[85] = TERRAIN_ROAD;
    tile_properties[86] = TERRAIN_ROAD;
    tile_properties[87] = TERRAIN_ROAD;
    tile_properties[88] = TERRAIN_GRASS;
    tile_properties[89] = TERRAIN_ROAD;
    tile_properties[90] = TERRAIN_ROAD;
    tile_properties[91] = TERRAIN_ROAD;
    tile_properties[92] = TERRAIN_WALL;
    tile_properties[93] = TERRAIN_ROAD;
    tile_properties[94] = TERRAIN_ROAD;
    tile_properties[95] = TERRAIN_ROAD;
    tile_properties[96] = TERRAIN_WALL;
    tile_properties[97] = TERRAIN_WALL;
    tile_properties[98] = TERRAIN_WALL;
    tile_properties[99] = TERRAIN_ROAD;
    tile_properties[100] = TERRAIN_WALL;
    tile_properties[101] = TERRAIN_GRASS;
    tile_properties[102] = TERRAIN_GRASS;
    tile_properties[103] = TERRAIN_GRASS;
    tile_properties[104] = TERRAIN_GRASS;
    tile_properties[105] = TERRAIN_GRASS;
    tile_properties[106] = TERRAIN_ROAD;
    tile_properties[107] = TERRAIN_GRASS;
    tile_properties[108] = TERRAIN_ROAD;
    tile_properties[109] = TERRAIN_WALL;
    tile_properties[110] = TERRAIN_ROAD;
    tile_properties[111] = TERRAIN_GRASS;
    tile_properties[112] = TERRAIN_GRASS;
    tile_properties[113] = TERRAIN_GRASS;
    tile_properties[114] = TERRAIN_ROAD;
    tile_properties[115] = TERRAIN_WALL;
    tile_properties[116] = TERRAIN_GRASS;
    tile_properties[117] = TERRAIN_WALL;
    tile_properties[118] = TERRAIN_WALL;
    tile_properties[119] = TERRAIN_WALL;
    tile_properties[120] = TERRAIN_WALL;
    tile_properties[121] = TERRAIN_WALL;
    tile_properties[122] = TERRAIN_WALL;
    tile_properties[123] = TERRAIN_GRASS;
    tile_properties[124] = TERRAIN_GRASS;
    tile_properties[125] = TERRAIN_GRASS;
    tile_properties[126] = TERRAIN_WALL;
    tile_properties[127] = TERRAIN_WALL;
    tile_properties[128] = TERRAIN_GRASS;
    tile_properties[129] = TERRAIN_GRASS;
    tile_properties[130] = TERRAIN_GRASS;
    tile_properties[131] = TERRAIN_WALL;
    tile_properties[132] = TERRAIN_GRASS;
    tile_properties[133] = TERRAIN_GRASS;
    tile_properties[134] = TERRAIN_GRASS;
    tile_properties[135] = TERRAIN_GRASS;
    tile_properties[136] = TERRAIN_GRASS;
    tile_properties[137] = TERRAIN_GRASS;
    tile_properties[138] = TERRAIN_GRASS;
    tile_properties[139] = TERRAIN_GRASS;
    tile_properties[140] = TERRAIN_GRASS;
    tile_properties[141] = TERRAIN_WALL;
    tile_properties[142] = TERRAIN_GRASS;
    tile_properties[143] = TERRAIN_GRASS;
    tile_properties[144] = TERRAIN_GRASS;
    tile_properties[145] = TERRAIN_GRASS;
    tile_properties[146] = TERRAIN_GRASS;
    tile_properties[147] = TERRAIN_WALL;
    tile_properties[148] = TERRAIN_WALL;
    tile_properties[149] = TERRAIN_WALL;
    tile_properties[150] = TERRAIN_WALL;
    tile_properties[151] = TERRAIN_WALL;
    tile_properties[152] = TERRAIN_GRASS;
    tile_properties[153] = TERRAIN_WALL;
    tile_properties[154] = TERRAIN_GRASS;
    tile_properties[155] = TERRAIN_WALL;
    tile_properties[156] = TERRAIN_GRASS;
    tile_properties[157] = TERRAIN_ROAD;
    tile_properties[158] = TERRAIN_WALL;
    tile_properties[159] = TERRAIN_ROAD;
    tile_properties[160] = TERRAIN_WALL;
    tile_properties[161] = TERRAIN_WALL;
    tile_properties[162] = TERRAIN_ROAD;
    tile_properties[163] = TERRAIN_GRASS;
    tile_properties[164] = TERRAIN_GRASS;
    tile_properties[165] = TERRAIN_GRASS;
    tile_properties[166] = TERRAIN_GRASS;
    tile_properties[167] = TERRAIN_ROAD;
    tile_properties[168] = TERRAIN_WALL;
    tile_properties[169] = TERRAIN_ROAD;
    tile_properties[170] = TERRAIN_GRASS;
    tile_properties[171] = TERRAIN_WALL;
    tile_properties[172] = TERRAIN_GRASS;
    tile_properties[173] = TERRAIN_GRASS;
    tile_properties[174] = TERRAIN_WALL;
    tile_properties[175] = TERRAIN_WALL;
    tile_properties[176] = TERRAIN_WALL;
    tile_properties[177] = TERRAIN_GRASS;
    tile_properties[178] = TERRAIN_GRASS;
    tile_properties[179] = TERRAIN_GRASS;
    tile_properties[180] = TERRAIN_GRASS;
    tile_properties[181] = TERRAIN_GRASS;
    tile_properties[182] = TERRAIN_ROAD;
    tile_properties[183] = TERRAIN_GRASS;
    tile_properties[184] = TERRAIN_GRASS;
    tile_properties[185] = TERRAIN_GRASS;
    tile_properties[186] = TERRAIN_ROAD;
    tile_properties[187] = TERRAIN_WALL;
    tile_properties[188] = TERRAIN_GRASS;
    tile_properties[189] = TERRAIN_ROAD;
    tile_properties[190] = TERRAIN_GRASS;
    tile_properties[191] = TERRAIN_GRASS;
    tile_properties[192] = TERRAIN_GRASS;
    tile_properties[193] = TERRAIN_ROAD;
    tile_properties[194] = TERRAIN_ROAD;
    tile_properties[195] = TERRAIN_ROAD;
    tile_properties[196] = TERRAIN_ROAD;
    tile_properties[197] = TERRAIN_ROAD;
    tile_properties[198] = TERRAIN_GRASS;
    tile_properties[199] = TERRAIN_GRASS;
    tile_properties[200] = TERRAIN_ROAD;
    tile_properties[201] = TERRAIN_ROAD;
    tile_properties[202] = TERRAIN_ROAD;
    tile_properties[203] = TERRAIN_ROAD;
    tile_properties[204] = TERRAIN_ROAD;
    tile_properties[205] = TERRAIN_GRASS;
    tile_properties[206] = TERRAIN_GRASS;
    tile_properties[207] = TERRAIN_GRASS;
    tile_properties[208] = TERRAIN_GRASS;
    tile_properties[209] = TERRAIN_GRASS;
    tile_properties[210] = TERRAIN_GRASS;
    tile_properties[211] = TERRAIN_WALL;
    tile_properties[212] = TERRAIN_GRASS;
    tile_properties[213] = TERRAIN_GRASS;
    tile_properties[214] = TERRAIN_GRASS;
    tile_properties[215] = TERRAIN_GRASS;
    tile_properties[216] = TERRAIN_ROAD;
    tile_properties[217] = TERRAIN_GRASS;
    tile_properties[218] = TERRAIN_GRASS;
    tile_properties[219] = TERRAIN_ROAD;
    tile_properties[220] = TERRAIN_ROAD;
    tile_properties[221] = TERRAIN_GRASS;
    tile_properties[222] = TERRAIN_GRASS;
    tile_properties[223] = TERRAIN_ROAD;
    tile_properties[224] = TERRAIN_ROAD;
    tile_properties[225] = TERRAIN_ROAD;
    tile_properties[226] = TERRAIN_ROAD;
    tile_properties[227] = TERRAIN_GRASS;
    tile_properties[228] = TERRAIN_ROAD;
    tile_properties[229] = TERRAIN_ROAD;
    tile_properties[230] = TERRAIN_ROAD;
    tile_properties[231] = TERRAIN_ROAD;
    tile_properties[232] = TERRAIN_ROAD;
    tile_properties[233] = TERRAIN_ROAD;
    tile_properties[234] = TERRAIN_ROAD;
    tile_properties[235] = TERRAIN_ROAD;
    tile_properties[236] = TERRAIN_GRASS;
    tile_properties[237] = TERRAIN_ROAD;
    tile_properties[238] = TERRAIN_ROAD;
    tile_properties[239] = TERRAIN_ROAD;
    tile_properties[240] = TERRAIN_ROAD;
    tile_properties[241] = TERRAIN_GRASS;
    tile_properties[242] = TERRAIN_GRASS;
    tile_properties[243] = TERRAIN_FINISH;  // Finish line
    tile_properties[244] = TERRAIN_FINISH;  // Finish line
    tile_properties[245] = TERRAIN_FINISH;  // Finish line
    tile_properties[246] = TERRAIN_FINISH;  // Finish line
    tile_properties[247] = TERRAIN_FINISH;  // Finish line
    tile_properties[248] = TERRAIN_FINISH;  // Finish line
}

// Total tiles processed: 249
