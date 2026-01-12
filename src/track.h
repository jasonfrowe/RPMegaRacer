#ifndef TRACK_H
#define TRACK_H

#define TERRAIN_ROAD  0
#define TERRAIN_GRASS 1
#define TERRAIN_WALL  2
#define TERRAIN_BOOST 3
#define TERRAIN_FINISH  3  // Checkpoint 0
#define TERRAIN_CP1     4  // Checkpoint 1
#define TERRAIN_CP2     5  // Checkpoint 2

extern uint8_t world_map[3072];
extern uint8_t tile_properties[256];
extern uint8_t tile_collision_masks[256][8];
extern void init_track_physics(void);
extern void load_track_data(const char* track_dir);
extern uint8_t get_terrain_at(int16_t x, int16_t y);

#endif // TRACK_H