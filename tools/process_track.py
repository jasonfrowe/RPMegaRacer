#!/usr/bin/env python3
"""
Process track tiles to generate collision masks, properties, and map binaries.
Based on generate_collision_masks.py logic but outputs binary files.

Usage: ./process_track.py <tiles.bin> <output_dir>
"""

import sys
import os
import struct

def process_track(bin_file, output_dir):
    # Color definitions
    ROAD_COLORS = {1, 2}           # Road (passable, no slowdown)
    TERRAIN_COLORS = {3, 8}        # Terrain/grass (passable, slowdown)
    PASSABLE_COLORS = ROAD_COLORS | TERRAIN_COLORS  # All passable colors
    
    # Special tiles that are always 100% road regardless of pixel colors
    FINISH_LINE_TILES = set(range(243, 249))  # Tiles 243-248
    
    with open(bin_file, 'rb') as f:
        tile_data = f.read()
    
    num_tiles = len(tile_data) // 32
    print(f"Processing {num_tiles} tiles from {bin_file}...")

    collision_masks = bytearray()
    tile_properties = bytearray()
    
    for tile_id in range(num_tiles):
        offset = tile_id * 32
        tile_bytes = tile_data[offset:offset + 32]
        
        # --- 1. Generate Collision Masks (8 bytes per tile) ---
        tile_mask_bytes = bytearray(8)
        
        # Special case: finish line tiles are always 100% passable
        if tile_id in FINISH_LINE_TILES:
            pass # Leave as 0x00 (all passable)
        else:
            for row in range(8):
                row_mask = 0
                row_offset = row * 4
                
                for byte_idx in range(4):
                    byte_val = tile_bytes[row_offset + byte_idx]
                    pixel1 = (byte_val >> 4) & 0x0F
                    pixel2 = byte_val & 0x0F
                    
                    col = byte_idx * 2
                    
                    if pixel1 not in PASSABLE_COLORS:
                        row_mask |= (0x80 >> col)
                    
                    if pixel2 not in PASSABLE_COLORS:
                        row_mask |= (0x80 >> (col + 1))
                
                tile_mask_bytes[row] = row_mask
        
        collision_masks.extend(tile_mask_bytes)

        # --- 2. Generate Properties (1 byte per tile) ---
        # 0=Road, 1=Grass, 2=Wall (Must match track.h)
        TERRAIN_ROAD = 0
        TERRAIN_GRASS = 1
        TERRAIN_WALL = 2

        prop_val = TERRAIN_WALL

        if tile_id in FINISH_LINE_TILES:
             prop_val = TERRAIN_ROAD
        else:
            road_count = 0
            terrain_count = 0
            total_pixels = 64
            
            for row in range(8):
                row_offset = row * 4
                for byte_idx in range(4):
                    byte_val = tile_bytes[row_offset + byte_idx]
                    pixel1 = (byte_val >> 4) & 0x0F
                    pixel2 = byte_val & 0x0F
                    
                    if pixel1 in ROAD_COLORS: road_count += 1
                    elif pixel1 in TERRAIN_COLORS: terrain_count += 1
                        
                    if pixel2 in ROAD_COLORS: road_count += 1
                    elif pixel2 in TERRAIN_COLORS: terrain_count += 1
            
            passable_count = road_count + terrain_count
            
            if passable_count > total_pixels // 2:
                if terrain_count > road_count:
                    prop_val = TERRAIN_GRASS
                else:
                    prop_val = TERRAIN_ROAD
            else:
                prop_val = TERRAIN_WALL
        
        tile_properties.append(prop_val)

    # Write Outputs
    col_path = os.path.join(output_dir, "collision.bin")
    with open(col_path, 'wb') as f:
        f.write(collision_masks)
    print(f"Wrote {len(collision_masks)} bytes to {col_path}")

    prop_path = os.path.join(output_dir, "properties.bin")
    with open(prop_path, 'wb') as f:
        f.write(tile_properties)
    print(f"Wrote {len(tile_properties)} bytes to {prop_path}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <path_to_Track_A_tiles.bin> <output_dir>")
        sys.exit(1)
    
    bin_file = sys.argv[1]
    out_dir = sys.argv[2]
    
    if not os.path.exists(bin_file):
        print(f"Error: File not found: {bin_file}")
        sys.exit(1)
    
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)
    
    process_track(bin_file, out_dir)
