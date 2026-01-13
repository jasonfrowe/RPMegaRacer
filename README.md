# RP Mega Racer

A high-performance, top-down arcade racing game for the [**RP6502 PicoComputer**](https://picocomputer.github.io/), heavily inspired by Atari's classic *Super Sprint* and the *Micro Machines* series.

## About

**RP Mega Racer** pushes the limits of the RP6502, utilizing its unique multi-core architecture to deliver a smooth 60 FPS racing experience. Featuring hardware-accelerated affine (rotating) sprites, multi-plane scrolling backgrounds, and a unique hybrid audio engine, it brings 16-bit arcade vibes to the 8-bit 6502 world.

## Features

- **Smooth 60 FPS Gameplay**: Optimized 10.6 fixed-point physics and axis-separated collision logic.
- **Hardware-Accelerated Graphics**: High-resolution 320x240 display using Mode 4 Affine Sprites for smooth car rotation and Mode 2 Tilemaps for scrolling tracks.
- **Multi-Track Support**: Dynamic track loading allows for an unlimited number of courses.
- **Track Progression**: Win a 5-Lap race to advance to the next track. Lose, and you go back to the start!
- **Hybrid Audio Engine**: 
  - **OPL2 (FM Synthesis)**: Dedicated FPGA-based OPL2 card for high-quality background music and a dynamic, pitch-shifting engine growl on Channel 8.
  - **RIA PSG**: Utilizing the onboard Programmable Sound Generator for "crunchy" arcade sound effects like tire screeches and wall impacts.
- **DRS (Drag Reduction System)**: A tactical catch-up mechanic. If you aren't in the lead, your battery charges—activate it for a significant top-speed boost!
- **Competitive AI**: 3 AI racers with "rubberbanding" logic that adapts to your skill level, ensuring every race is a nail-biter.
- **Advanced Collision System**: Arcade-style "rubber" walls that bounce you back into the action, designed to prevent the "stuck-on-wall" frustrations of vintage racers.

## Adding New Tracks

The game is designed to be easily extensible. To add a new track:

1.  **Create a Folder**: Create a new directory in `tracks/` named sequentially (e.g., `track03`).
2.  **Add Assets**: The folder must contain the following binary files (generated via the tools in `tools/`):
    - `map.bin` (Tilemap indices)
    - `tiles.bin` (Tile pixel data)
    - `collision.bin` (Collision masks)
    - `properties.bin` (Terrain properties)
    - `waypoints.bin` (AI pathfinding nodes)
3.  **Update Config**: Open `src/track.h` and increase the `NUM_TRACKS` constant to reflect the new total.
4.  **Build**: Recompile the game. The logic will automatically include the new track in the rotation.

## Controls

The game supports both Keyboard and standard USB Gamepads. Use the included **Gamepad Mapper** to customize your layout.

| Action | Keyboard | Gamepad (Default) |
| :--- | :--- | :--- |
| **Steer Left/Right** | Left / Right Arrows | D-Pad Left / Right |
| **Accelerate (Gas)** | Z / Space | Button A |
| **Reverse** | X | Button X |
| **Activate DRS** | C / Left Shift | Button Y |
| **Rescue (Teleport)** | R | Button B |
| **Pause / Start** | ESC | Start |

### DRS (Drag Reduction System)
Watch the **DRS Meter** on your HUD. It charges automatically whenever you are behind the leader. Once the bar flashes **Cyan**, press the DRS button to activate a 2-second speed boost.

### Rescue System
If you get stuck behind a barrier or rammed into a corner by the AI, press the **Rescue** button to instantly teleport to the nearest safe waypoint on the track.

## Building the Game

This project uses CMake and LLVM-MOS to target the 65C02 CPU.

### Requirements

- [LLVM-MOS SDK](https://llvm-mos.org/)
- CMake 3.18+

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

This will produce two main files:
1. `RPMegaRacer.rp6502`: The main game.
2. `gamepad_mapper.rp6502`: Use this to calibrate your controller.

**Note**: Ensure the `tracks/` directory is copied to your RP6502 storage so the game can load the level data.

## Technical Details

- **CPU**: 65C02 (via LLVM-MOS)
- **Resolution**: 320x240 pixels
- **Colors**: 16-bit RGB555 for Sprites, 4-bit Indexed for Tiles
- **Memory**: Intensive use of RIA XRAM for sprite attribute tables and tilemap data.
- **Physics**: Sub-pixel movement using 10.6 fixed-point math to maintain accuracy while avoiding 32-bit overhead.

## Credits

- **Development**: Jason F. Rowe
- **Hardware**: RP6502 PicoComputer by Picocomputer.
- **Inspiration**: Atari Super Sprint, Micro Machines (NES).

## License

This project is licensed under the GNU General Public License - see the LICENSE file for details.