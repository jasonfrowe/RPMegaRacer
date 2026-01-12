import sys
import struct
import json
import argparse

# Default for testing if no file provided
DEFAULT_WAYPOINTS = [
    (255, 60), (120, 50), (81, 81), (60, 192), (48, 255), (88, 295), 
    (247, 315), (375, 328), (431, 295), (435, 175), (445, 116), (417, 80)
]

def pack_waypoints(input_file, output_file):
    waypoints = DEFAULT_WAYPOINTS
    
    if input_file:
        try:
            with open(input_file, 'r') as f:
                data = json.load(f)
                # Expecting format: [[x,y], [x,y], ...]
                waypoints = data
        except Exception as e:
            print(f"Error reading {input_file}: {e}")
            sys.exit(1)

    with open(output_file, 'wb') as f:
        # Header: Number of waypoints (uint16_t)
        f.write(struct.pack('<H', len(waypoints)))
        
        # Data: x (int16), y (int16)
        for wp in waypoints:
            x, y = wp[0], wp[1]
            f.write(struct.pack('<hh', x, y))
    
    print(f"Wrote {len(waypoints)} waypoints to {output_file}")

def main():
    parser = argparse.ArgumentParser(description="Pack ID waypoints into binary.")
    parser.add_argument("output", help="Output .bin file")
    parser.add_argument("input", nargs='?', help="Input .json file (optional, defaults to Track01 data)")
    
    args = parser.parse_args()
    pack_waypoints(args.input, args.output)

if __name__ == "__main__":
    main()
