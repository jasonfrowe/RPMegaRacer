
import struct

def get_terrain_at(x, y, world_map, tile_properties, tile_collision_masks):
    if x < 0 or y < 0 or x >= 512 or y >= 384:
        return 1, "BOUNDS" # Wall

    tx = x >> 3
    ty = y >> 3
    px = x & 7
    py = y & 7

    map_index = ty * 64 + tx
    if map_index >= len(world_map):
        return 1, "MAP_INDEX_ERR"
    
    tile_id = world_map[map_index]

    # Check collision mask
    row_mask = 0
    if tile_id < len(tile_collision_masks):
        row_mask = tile_collision_masks[tile_id][py]
    
    collision_source = "PROP"
    is_wall = False

    if row_mask != 0:
        pixel_mask = 0x80 >> px
        if row_mask & pixel_mask:
            is_wall = True
            collision_source = "MASK"
    else:
        # Fallback to properties
        prop = 1 # Default Wall
        if tile_id < len(tile_properties):
            prop = tile_properties[tile_id]
        
        if prop == 2: # TERRAIN_WALL is 2
             is_wall = True
             collision_source = "PROP_WALL"
        else:
             collision_source = f"PROP_{prop}"

    return is_wall, collision_source, tile_id

def main():
    try:
        with open("tracks/track01/map.bin", "rb") as f:
            world_map = list(f.read())
        
        with open("tracks/track01/properties.bin", "rb") as f:
            tile_properties = list(f.read())
            # Pad to 256 with 1 (WALL)
            while len(tile_properties) < 256:
                tile_properties.append(1)

        with open("tracks/track01/collision.bin", "rb") as f:
            raw_masks = f.read()
            tile_collision_masks = []
            for i in range(0, len(raw_masks), 8):
                tile_collision_masks.append(list(raw_masks[i:i+8]))
            # Pad
            while len(tile_collision_masks) < 256:
                tile_collision_masks.append([0]*8)

    except Exception as e:
        print(f"Error reading files: {e}")
        return

    # Positions to check (x, y)
    # Player: 245, 70
    # AI 1: 245, 40
    # AI 2: 245, 50
    # AI 3: 245, 60
    
    positions = [
        ("Player", 245, 70),
        ("AI 1", 245, 40),
        ("AI 2", 245, 50),
        ("AI 3", 245, 60)
    ]

    print(f"{'Entity':<10} | {'X':<4} | {'Y':<4} | {'TileID':<6} | {'IsWall':<6} | {'Source':<10}")
    print("-" * 60)

    for name, x, y in positions:
        # Check center + 8 logic from is_colliding_fast/ai
        # Actually is_colliding_fast checks corners around center.
        # Let's check the exact coordinate first.
        
        is_wall, source, tile_id = get_terrain_at(x, y, world_map, tile_properties, tile_collision_masks)
        print(f"{name:<10} | {x:<4} | {y:<4} | {tile_id:<6} | {is_wall!s:<6} | {source:<10}")

        # Check corners to simulate is_colliding_fast
        cx = x + 8
        cy = y + 8
        H = 4
        corners = [
            (cx - H, cy - H), (cx + H, cy - H),
            (cx - H, cy + H), (cx + H, cy + H)
        ]
        
        collision_count = 0
        for ctx, cty in corners:
            w, _, tid = get_terrain_at(ctx, cty, world_map, tile_properties, tile_collision_masks)
            if w:
                collision_count += 1
                # print(f"  Corner ({ctx},{cty}) Tile {tid}: Wall")
        
        if collision_count > 0:
            print(f"  -> Colliding at {collision_count} corners!")

if __name__ == "__main__":
    main()
