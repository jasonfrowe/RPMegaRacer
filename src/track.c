#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <rp6502.h>
#include <fcntl.h>
#include <unistd.h>
#include "track.h"
#include "constants.h"

uint8_t world_map[3072];
uint8_t tile_properties[256];

// Collision masks: 8 rows per tile, 8 bits per row
// Bit 7 (0x80) = leftmost pixel, Bit 0 (0x01) = rightmost pixel
// 1 = solid/wall, 0 = passable
uint8_t tile_collision_masks[256][8];

// Helper to load file directly to XRAM
void load_file_to_xram(const char* filename, uint16_t dest_addr, uint16_t max_size) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Error opening %s\n", filename);
        return;
    }
    
    // We can't read directly to XRAM with standard read(), 
    // so we read to a small buffer and write to XRAM via RIA
    uint8_t buffer[128];
    uint16_t total_read = 0;
    
    RIA.addr0 = dest_addr;
    RIA.step0 = 1;

    while (total_read < max_size) {
        int bytes = read(fd, buffer, sizeof(buffer));
        if (bytes <= 0) break;
        
        for (int i = 0; i < bytes; i++) {
            RIA.rw0 = buffer[i];
        }
        total_read += bytes;
    }
    
    close(fd);
    printf("Loaded %s to XRAM 0x%04X (%d bytes)\n", filename, dest_addr, total_read);
}

// Helper to load file directly to RAM
void load_file_to_ram(const char* filename, void* dest, uint16_t max_size) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Error opening %s\n", filename);
        return;
    }
    
    int bytes = read(fd, dest, max_size);
    if (bytes < 0) printf("Error reading %s\n", filename);
    else printf("Loaded %s to RAM (%d bytes)\n", filename, bytes);
    
    close(fd);
}

void load_track_data(const char* track_dir) {
    char path[64];
    
    // 1. Load Map to XRAM (and RAM copy)
    sprintf(path, "%s/map.bin", track_dir);
    load_file_to_xram(path, TRACK_MAP_ADDR, 3072);
    
    // Also read map into RAM for quick collision lookups
    // (We could read back from XRAM, but reading file is fine too)
    load_file_to_ram(path, world_map, sizeof(world_map));

    // 2. Load Tiles to XRAM
    sprintf(path, "%s/tiles.bin", track_dir);
    load_file_to_xram(path, TRACK_DATA, TRACK_DATA_SIZE);

    // 3. Load Collision Masks to RAM
    sprintf(path, "%s/collision.bin", track_dir);
    load_file_to_ram(path, tile_collision_masks, sizeof(tile_collision_masks));

    // 4. Load Properties to RAM
    sprintf(path, "%s/properties.bin", track_dir);
    load_file_to_ram(path, tile_properties, sizeof(tile_properties));
}

#include "ai.h"

uint16_t g_num_active_waypoints = NUM_WAYPOINTS;
int current_track_id = 1;

void load_waypoints(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Error opening %s\n", filename);
        return;
    }

    uint16_t file_count = 0;
    // 1. Read header (2 bytes)
    read(fd, &file_count, 2);

    printf("Loading Waypoints: File has %d\n", file_count);

    if (file_count > NUM_WAYPOINTS) {
        printf("Warning: Truncating waypoints to %d\n", NUM_WAYPOINTS);
        g_num_active_waypoints = NUM_WAYPOINTS;
    } else {
        g_num_active_waypoints = file_count;
    }

    // 2. Read the binary data directly into the array
    read(fd, waypoints, g_num_active_waypoints * sizeof(Waypoint));
    
    close(fd);
}

// Track the currently loaded track to avoid redundant loads
static int last_loaded_track_id = -1;

void load_track(int track_id) {
    if (track_id == last_loaded_track_id) {
        printf("Track %d already loaded, skipping.\n", track_id);
        return;
    }

    // Defaults (in case load fails or partial load)
    memset(tile_collision_masks, 0, sizeof(tile_collision_masks));
    for (int i = 0; i < 256; i++) tile_properties[i] = TERRAIN_WALL;

    char track_dir[32];
    sprintf(track_dir, "tracks/track%02d", track_id);
    
    // Load Map, Tiles, Collision, Properties
    load_track_data(track_dir);

    // Load Waypoints
    char waypoints_file[64];
    sprintf(waypoints_file, "%s/waypoints.bin", track_dir);
    load_waypoints(waypoints_file);

    last_loaded_track_id = track_id;
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

    // FIX: Explicitly report Finish Line tiles regardless of properties.bin
    if (current_track_id == 3) {
         if (tile_id == 14 || tile_id == 15 || tile_id == 44 || tile_id == 45) {
            return TERRAIN_FINISH;
        }
    } else {
        // Tiles 243-248 are defined as FINISH_LINE_TILES in the generator for Tracks 1 & 2
        if (tile_id >= 243 && tile_id <= 248) {
            return TERRAIN_FINISH;
        }
    }

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
