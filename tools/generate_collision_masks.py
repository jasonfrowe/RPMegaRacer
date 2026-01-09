#!/usr/bin/env python3
"""
Generate collision masks from Track_A_tiles.bin

Road colors: 1, 2, 10, 11 (passable)
Grass colors: 8 (passable)
Barrier colors: everything else (solid)

Output: C code for tile_collision_masks array
"""

import sys
import os

def generate_collision_masks(bin_file):
    """
    Read Track_A_tiles.bin and generate collision masks.
    
    Format:
    - Each tile is 8x8 pixels = 32 bytes (4-bit packed format)
    - Each row is 4 bytes (2 pixels per byte, packed as high/low nibbles)
    - Each byte contains: (pixel1 << 4) | pixel2
    - Collision mask: 1 bit per pixel, 8 bytes per tile
    - Bit set (1) = solid/barrier, Bit clear (0) = passable
    """
    
    # Color definitions
    ROAD_COLORS = {1, 2}           # Road (passable, no slowdown)
    TERRAIN_COLORS = {3, 8}        # Terrain/grass (passable, slowdown)
    PASSABLE_COLORS = ROAD_COLORS | TERRAIN_COLORS  # All passable colors
    
    # Special tiles that are always 100% road regardless of pixel colors
    FINISH_LINE_TILES = set(range(243, 249))  # Tiles 243-248
    
    with open(bin_file, 'rb') as f:
        tile_data = f.read()
    
    # Calculate number of tiles (32 bytes per tile in packed format)
    num_tiles = len(tile_data) // 32
    print(f"// Generated from {os.path.basename(bin_file)}")
    print(f"// Found {num_tiles} tiles ({len(tile_data)} bytes)")
    print()
    print("void init_tile_collision_masks(void) {")
    
    for tile_id in range(num_tiles):
        offset = tile_id * 32
        tile_bytes = tile_data[offset:offset + 32]
        
        # Special case: finish line tiles are always 100% passable
        if tile_id in FINISH_LINE_TILES:
            # No collision mask needed - will use tile_properties fallback
            continue
        
        # Check if this tile has any barriers
        has_barriers = False
        collision_rows = []
        
        for row in range(8):
            row_mask = 0
            row_offset = row * 4  # 4 bytes per row
            
            # Each row has 4 bytes, each byte contains 2 pixels
            for byte_idx in range(4):
                byte_val = tile_bytes[row_offset + byte_idx]
                # High nibble is first pixel, low nibble is second pixel
                pixel1 = (byte_val >> 4) & 0x0F
                pixel2 = byte_val & 0x0F
                
                col = byte_idx * 2
                
                # Check first pixel (left)
                if pixel1 not in PASSABLE_COLORS:
                    has_barriers = True
                    row_mask |= (0x80 >> col)
                
                # Check second pixel (right)
                if pixel2 not in PASSABLE_COLORS:
                    has_barriers = True
                    row_mask |= (0x80 >> (col + 1))
            
            collision_rows.append(row_mask)
        
        # Only output tiles that have barriers
        if has_barriers:
            print(f"    // Tile {tile_id}")
            for row_idx, row_mask in enumerate(collision_rows):
                if row_mask != 0:  # Only print non-zero rows
                    print(f"    tile_collision_masks[{tile_id}][{row_idx}] = 0x{row_mask:02X};", end="")
                    # Visual representation
                    visual = ''.join(['█' if (row_mask & (0x80 >> i)) else '□' for i in range(8)])
                    print(f"  // {visual}")
            
            # Print empty rows for clarity
            if any(mask == 0 for mask in collision_rows):
                zero_rows = [i for i, mask in enumerate(collision_rows) if mask == 0]
                if len(zero_rows) < 8:  # Don't print if all rows have data
                    print(f"    // Rows {zero_rows} are fully passable")
    
    print("}")
    print()
    
    # Now generate tile_properties based on majority color
    print("void init_tile_properties(void) {")
    for tile_id in range(num_tiles):
        # Special case: finish line tiles are always road
        if tile_id in FINISH_LINE_TILES:
            print(f"    tile_properties[{tile_id}] = TERRAIN_ROAD;  // Finish line")
            continue
            
        offset = tile_id * 32
        tile_bytes = tile_data[offset:offset + 32]
        
        # Count passable vs barrier pixels
        road_count = 0
        terrain_count = 0
        total_pixels = 64
        
        for row in range(8):
            row_offset = row * 4
            for byte_idx in range(4):
                byte_val = tile_bytes[row_offset + byte_idx]
                pixel1 = (byte_val >> 4) & 0x0F
                pixel2 = byte_val & 0x0F
                
                if pixel1 in ROAD_COLORS:
                    road_count += 1
                elif pixel1 in TERRAIN_COLORS:
                    terrain_count += 1
                    
                if pixel2 in ROAD_COLORS:
                    road_count += 1
                elif pixel2 in TERRAIN_COLORS:
                    terrain_count += 1
        
        passable_count = road_count + terrain_count
        
        # If more than half the pixels are passable, it's road/terrain
        if passable_count > total_pixels // 2:
            if terrain_count > road_count:
                print(f"    tile_properties[{tile_id}] = TERRAIN_GRASS;")
            else:
                print(f"    tile_properties[{tile_id}] = TERRAIN_ROAD;")
        else:
            print(f"    tile_properties[{tile_id}] = TERRAIN_WALL;")
    
    print("}")
    print()
    print(f"// Total tiles processed: {num_tiles}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <path_to_Track_A_tiles.bin>")
        sys.exit(1)
    
    bin_file = sys.argv[1]
    
    if not os.path.exists(bin_file):
        print(f"Error: File not found: {bin_file}")
        sys.exit(1)
    
    generate_collision_masks(bin_file)
