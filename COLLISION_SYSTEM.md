# Collision System Documentation

## Overview

The collision system uses **pixel-perfect sub-tile collision masks** to determine which parts of each 8×8 tile are solid barriers vs. passable terrain.

## How It Works

### Collision Masks
- Each tile has an 8-byte collision mask (one byte per row)
- Each byte represents 8 pixels: `Bit 7 = left pixel, Bit 0 = right pixel`
- `1` = solid/barrier, `0` = passable

Example:
```c
tile_collision_masks[42][0] = 0xF0;  // ████□□□□ (left half solid)
tile_collision_masks[42][1] = 0xF0;  // ████□□□□
// ... etc
```

### Terrain Classification

Based on color indices in `Track_A_tiles.bin`:
- **Road** (passable): color indices 1, 2, 10, 11
- **Grass** (passable, slowdown): color index 8
- **Barrier** (solid): all other color indices

### Auto-Generation

Run `tools/generate_collision_masks.py` to regenerate collision masks from tile graphics:

```bash
python3 tools/generate_collision_masks.py images/Track_A_tiles.bin > src/collision_masks_generated.c
```

Then append the generated function to `src/track.c`.

## Current Status

**Tiles processed:** 124 tiles  
**All tiles are fully solid** (0xFF patterns) - no partial tiles in current tileset

This means your barriers are currently solid blocks. To create diagonal walls, curves, or partial barriers:

1. Edit tiles in Aseprite to have mixed colors (barrier + road/grass in same tile)
2. Regenerate collision masks using the Python script
3. Rebuild the project

## Memory Usage

- **2048 bytes**: 256 tiles × 8 bytes per tile
- All 256 tile slots initialized, but only 124 currently have graphics

## API

### `get_terrain_at(uint16_t x, uint16_t y)`
Returns terrain type at pixel coordinates (x, y):
- Checks the specific pixel within the tile
- Returns `TERRAIN_WALL` if pixel is solid
- Returns `TERRAIN_ROAD` or `TERRAIN_GRASS` if passable

### `init_track_physics()`
Initializes the collision system:
1. Clears all collision masks
2. Calls `init_tile_collision_masks()` to load generated masks
3. Sets up `tile_properties[]` array for fallback terrain types

## Future Enhancements

If you create tiles with diagonal walls or curves:
- The system will automatically detect passable vs. solid pixels
- Car will smoothly navigate around barrier edges
- No code changes needed - just regenerate masks from new tile graphics
