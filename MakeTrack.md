# How to Create a New Race Track

This guide outlines the procedure for creating a new race map for RPMegaRacer. 

## Prerequisites
- **Aseprite**: For editing tiles and exporting the map.
- **Python 3**: For running the conversion tools.
- **Project Tools**: Located in the `tools/` directory.

## Directory Structure
Tracks are located in `tracks/<track_name>/`. Each track directory should contain:
- `map.bin`: The 64x48 tilemap indices.
- `tiles.bin`: The visual tile data (reused from `images/track_A_tiles.bin` if using the standard set).
- `collision.bin`: Generated collision masks.
- `properties.bin`: Generated tile properties (Road/Grass/Wall).
- `waypoints.bin`: AI navigation points.
- `waypoints.json`: Source file for waypoints (recommended).

## Step-by-Step Guide

### 1. Create the Map in Aseprite
1.  Open `Sprites/Track_Map_A.aseprite` (or create a new 64x48 pixel image).
2.  Use the pencil tool to draw your track using the existing tileset indices.
3.  **Export the Map**:
    - Select the Tilemap layer.
    - Go to `File > Scripts > Open Scripts Folder` if you haven't installed `tools/export_map.lua`.
    - Run the `export_map.lua` script (File > Scripts > export_map).
    - Save as `tracks/<your_track>/map.bin`.

### 2. Generate Tile Data (Start Here if making NEW tiles)
If you are modifying the graphical tiles themselves:
1.  Edit `Sprites/Track_A_tiles.aseprite`.
2.  Export as PNG: `Sprites/Track_A_tiles.png`.
3.  Run the converter:
    ```bash
    ./tools/convert_sprite.py --mode tile -o images/Track_A_tiles.bin Sprites/Track_A_tiles.png
    ```
4.  Copy this binary to your track folder if you want a self-contained track, or referencing it from `images/` is fine if the code supports it (currently code loads from specific track dir, so copy it or symlink).
    ```bash
    cp images/Track_A_tiles.bin tracks/<your_track>/tiles.bin
    ```

### 3. Generate Physics Data (Collision & Properties)
The game needs to know which tiles are walls, road, or grass. This is generated automatically from the tile graphics.

Run `process_track.py` with your tiles binary and the output directory:
```bash
./tools/process_track.py images/Track_A_tiles.bin tracks/<your_track>
```
This will create `collision.bin` and `properties.bin` in the destination folder.

### 4. Create AI Waypoints
Waypoints guide the AI cars.
1.  Create a JSON file `tracks/<your_track>/waypoints.json` with a list of `[x, y]` coordinates.
    - Coordinates are in **pixels** (0-511, 0-383).
    - Example:
    ```json
    [
        [255, 60],
        [120, 50],
        [81, 81]
    ]
    ```
2.  Run the packer tool:
    ```bash
    ./tools/pack_waypoints.py tracks/<your_track>/waypoints.bin tracks/<your_track>/waypoints.json
    ```

## Example: Creating Track 02

```bash
# 1. Create directory
mkdir -p tracks/track02

# 2. Export map from Aseprite to tracks/track02/map.bin

# 3. Use standard tiles
cp images/Track_A_tiles.bin tracks/track02/tiles.bin

# 4. Generate physics
./tools/process_track.py tracks/track02/tiles.bin tracks/track02

# 5. Create waypoints
echo "[[245, 60], [245, 100], [300, 100]]" > tracks/track02/waypoints.json
./tools/pack_waypoints.py tracks/track02/waypoints.bin tracks/track02/waypoints.json
```

## Loading the New Track
To test your new track, update `src/track.c` -> `init_track_physics()`:

```c
load_track_data("tracks/track02");
load_waypoints("tracks/track02/waypoints.bin");
```
