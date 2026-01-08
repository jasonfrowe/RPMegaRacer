#include <stdint.h>
#include "track.h"

uint8_t world_map[3072];

uint8_t tile_properties[256];

void init_track_physics(void) {

    tile_properties[1] = TERRAIN_ROAD;  // Tile 1 is road
    tile_properties[2] = TERRAIN_GRASS; // Tile 2 is grass

    for (int i = 3; i < 243; i++) {
        tile_properties[i] = TERRAIN_WALL; // Tiles 3-242 are walls
    }

    for (int i = 243; i < 249; i++) {
        tile_properties[i] = TERRAIN_ROAD; // Tiles 243-248 the finish line (road)
    }

}

uint8_t get_terrain_at(uint16_t x, uint16_t y) {
    // 1. Clamp to world bounds
    if (x >= 512 || y >= 384) return TERRAIN_WALL;

    // 2. Convert pixels to tile coordinates (8x8 tiles)
    uint8_t tx = x >> 3; // x / 8
    uint8_t ty = y >> 3; // y / 8

    // 3. Get Tile ID from the map (Width is 64, 1 byte per tile)
    uint16_t map_index = ty * 64 + tx;
    uint8_t tile_id = world_map[map_index];

    // 4. Look up property
    return tile_properties[tile_id];
}