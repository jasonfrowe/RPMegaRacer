#!/usr/bin/env python3
"""
Pack waypoints into a binary file.
Format: [num_waypoints: uint16, waypoints: StructArray(x: int16, y: int16, ...)]

Usage: ./pack_waypoints.py <output_file>
"""

import sys
import struct

# Data extracted from src/ai.c (to be updated if source changes)
# struct Waypoint {
#     int16_t x;
#     int16_t y;
#     uint8_t suggested_speed; // 0-255? or float?
# };

# Extracted from ai.c
# These appear to be in tile coordinates (or close to it)
WAYPOINTS_DATA = [
    (255, 60), (120, 50), (81, 81), (60, 192), (48, 255), (88, 295), 
    (247, 315), (375, 328), (431, 295), (435, 175), (445, 116), (417, 80)
]

def pack_waypoints(output_file):
    with open(output_file, 'wb') as f:
        # Header: Number of waypoints (uint16_t)
        f.write(struct.pack('<H', len(WAYPOINTS_DATA)))
        
        # Data: x (int16), y (int16)
        # Assuming struct Waypoint { int16_t x; int16_t y; }; based on initialization in ai.c
        for wp in WAYPOINTS_DATA:
            x, y = wp
            f.write(struct.pack('<hh', x, y))
    
    print(f"Wrote {len(WAYPOINTS_DATA)} waypoints to {output_file}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <output_file>")
        sys.exit(1)
        
    pack_waypoints(sys.argv[1])
