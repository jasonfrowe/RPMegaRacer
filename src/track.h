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
extern void load_track(int track_id);
extern void load_track_data(const char* track_dir);
extern void load_waypoints(const char* filename);
extern uint8_t get_terrain_at(int16_t x, int16_t y);

extern uint16_t g_num_active_waypoints;

#endif // TRACK_H