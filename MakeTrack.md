# How to Create a New Race Track

## Prerequisites
- **Aseprite**, for the tiles and the map.
- **Python 3** with Pillow and NumPy (`.venv/bin/pip install -r requirements.txt`).

## What a Track Folder Holds

`tracks/trackNN/`:

| File | Made by | Use |
| :--- | :--- | :--- |
| `map.bin` | Aseprite + `tools/export_map.lua` | 64x48 tile map |
| `tiles.bin` | `tools/convert_sprite.py` | the 8x8 tiles, 4 bits per pixel |
| `collision.bin`, `properties.bin` | `tools/process_track.py` | which pixels are wall, which tiles are grass |
| `waypoints.json` | you | a rough loop in race order, starting on the finish line |

The build runs `tools/make_track.py` on each folder to make what the game loads:

- `walls.bin`: the walls on a 2x2 pixel grid. The cars collide with this.
- `grass.bin`: one bit per tile, for grass drag.
- `path.bin`: the AI racing line, about 8 px between points, starting at the finish line, with the heading, the room either side and a target speed at each point.

## Steps

1. **Map**: draw the 64x48 map in Aseprite and export it with `export_map.lua` to `tracks/trackNN/map.bin`.
2. **Tiles** (only if you changed them):
   ```bash
   ./tools/convert_sprite.py --mode tile -o tracks/trackNN/tiles.bin Sprites/Track_A_tiles.png
   ```
3. **Walls and terrain**:
   ```bash
   ./tools/process_track.py tracks/trackNN/tiles.bin tracks/trackNN
   ```
   Road is colours 1 and 2, grass 3 and 8, everything else is wall. The finish line tiles are listed in `process_track.py` and in `tools/trackdata.py`.
4. **Waypoints**: write `tracks/trackNN/waypoints.json`, a list of `[x, y]` pixels (0-511, 0-383) going round the track in race order. The first point must be on the finish line, and the line between points must not cross a wall. A dozen or so points are enough: they only guide the racing line, which is worked out from the walls.
5. **Check the racing line**:
   ```bash
   .venv/bin/python tools/make_track.py tracks/trackNN /tmp/trackNN --png /tmp/trackNN.png
   ```
   The picture shows the line coloured by speed (red slow, green fast), with the room either side in blue.
6. **Add it to the build**: add `NN` to the `foreach` in `CMakeLists.txt` and raise `NUM_TRACKS` in `src/track.h`.
7. **Race it**: `tools/simrace.sh NN` should finish with few wall contacts and no rescues.

The grid is laid out behind the first racing line point, facing along the line.
