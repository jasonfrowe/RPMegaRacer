# RP Mega Racer

A high-performance, top-down arcade racing game for the [**RP6502 PicoComputer**](https://picocomputer.github.io/), heavily inspired by Atari's classic *Super Sprint* and the *Micro Machines* series.

<!-- rp6502
preset: llvm-mos/Release
publish: RPMegaRacer.zip
frames: 120
-->
[![RP Mega Racer](https://jasonfrowe.github.io/RPMegaRacer/RPMegaRacer/screenshot.png)](https://jasonfrowe.github.io/RPMegaRacer/RPMegaRacer/)

[Play it in your browser](https://jasonfrowe.github.io/RPMegaRacer/RPMegaRacer/). On each push to `main`, `.github/workflows/web.yml` builds the game and publishes this web player and its screenshot to GitHub Pages.


## About

**RP Mega Racer** pushes the limits of the RP6502, utilizing its unique multi-core architecture to deliver a smooth 60 FPS racing experience. Featuring hardware-accelerated affine (rotating) sprites, multi-plane scrolling backgrounds, and a unique hybrid audio engine, it brings 16-bit arcade vibes to the 8-bit 6502 world.

## Features

- **Smooth 60 FPS Gameplay** on the 8 MHz 6502, with room to spare.
- **Arcade Handling**: the car grips and drifts, turns tighter at speed, and can always turn and back away from a wall.
- **Walls That Don't Grab**: hit a wall and you slide along it. Walls are checked against a 2-pixel grid built for each track, so no car can slip through a thin barrier or get wedged in one.
- **Fair Contact**: cars nudge each other instead of pinballing, so you can lean on a rival and get past.
- **Hardware-Accelerated Graphics**: 320x240, Mode 4 affine sprites for the rotating cars and a Mode 2 tile map for the scrolling track.
- **Racing Line AI**: each track gets a smooth racing line, worked out at build time from its walls, with corner speeds and the room either side. The 3 AI drivers each take their own lane, pass on whichever side is open, and back off rather than ram when there's no way by.
- **DRS (Drag Reduction System)**: a 2.5 second power boost for passing. It charges slowly anywhere behind the leader, and quickly when you're close behind the car ahead.
- **Gentle Rubber Banding**: the AI gives or takes at most about 10% of engine power to keep the race close, and never closes on you while your DRS is open.
- **Track Progression**: win a 5 lap race to advance to the next track. Lose, and you go back to the start!
- **Audio**: OPL2 FM music and an engine note that follows your speed.

## Controls

The game takes a keyboard or a USB gamepad. Run **gamepad_mapper** to remap a gamepad; it saves `JOYSTICK.DAT`.

| Action | Keyboard | Gamepad (default) |
| :--- | :--- | :--- |
| **Steer** | Left / Right, A / D | D-pad, left stick (proportional) |
| **Gas** | Space, Z, Up, W | A, R2, d-pad up |
| **Brake / Reverse** | X, Down, S | B, L2, d-pad down |
| **DRS** | C, Left Shift | X |
| **Rescue** | R | Y |
| **Start / Pause** | Enter, P | Start |

### DRS (Drag Reduction System)
The bar under the lap counter charges while you're behind the leader: slowly on your own, and fast when you're **IN RANGE** (close behind the car ahead). When it flashes **READY**, press DRS for a 2.5 second boost.

### Rescue
Stuck? Press Rescue to be put back on the racing line, just ahead, clear of other cars.

## Adding New Tracks

See [MakeTrack.md](MakeTrack.md). In short, a track folder in `tracks/` needs `map.bin`, `tiles.bin`, `collision.bin`, `properties.bin` and `waypoints.json`, a rough loop of points in race order that starts on the finish line. The build turns those into the wall grid, the grass map and the AI racing line (`tools/make_track.py`). Add the folder's number to the `foreach` in `CMakeLists.txt` and raise `NUM_TRACKS` in `src/track.h`.

## Building the Game

The project uses the [RP6502 SDK](https://picocomputer.github.io/sdk.html) (from the [rp6502-sdk template](https://github.com/picocomputer/rp6502-sdk)) with llvm-mos.

### Requirements

- llvm-mos for RP6502 (the SDK installs it in `~/.rp6502/llvm-mos`)
- CMake 3.21+, and Make or Ninja
- Python 3. To change a track you also need Pillow and NumPy in the project's `.venv` (`python3 -m venv .venv && .venv/bin/pip install -r requirements.txt`); with them, the build remakes each track's `walls.bin`, `grass.bin` and `path.bin`. Without them it uses the committed copies, which is what the GitHub build does.

### Build and Run

```bash
cmake --preset llvm-mos/Release
cmake --build --preset llvm-mos/Release
./tools/rp6502-emu build/llvm-mos/release/RPMegaRacer.rp6502
```

This builds `RPMegaRacer.rp6502` (the game, with every asset inside it), `gamepad_mapper.rp6502`, and the web player in `build/llvm-mos/release/web/`. The first configure fetches the emulator into `tools/`. In VS Code, the RP6502-EMU, RP6502-PICO and RP6502-WEB launch entries run it in the emulator, on a Picocomputer, or in a browser. Update the SDK tools with `cmake -P tools/rp6502.cmake`.

### Testing

- `tools/simrace.sh <track> [drs]` races all four cars on autopilot in the headless emulator and prints each lap, each finish (with wall and car contacts, AI reversals and rescues), and any dropped frames. With `drs`, the player's car uses DRS.
- `tools/wallskim.c` runs the car physics natively and drives along every wall of a track, leaning slightly into it, and reports how much speed the cars keep. Build and run it with `cc -O1 -Isrc -o build/wallskim tools/wallskim.c src/car.c src/trig.c && build/wallskim tracks/track01 4`.
- `tools/playtest.py race --track N --out DIR` plays a race with screenshots. `tools/playtest.py my_script.txt` runs your own emulator script, with `{symbol}` filled in from the build.

## Technical Details

- **CPU**: 65C02 at 8 MHz (llvm-mos)
- **Resolution**: 320x240; RGB555 sprites, 4-bit tiles, 8-bit text HUD
- **Physics**: positions in 1/256 px, velocities in 1/256 px per frame. Grip removes part of the sideways slide each frame. Cars move one axis at a time, at most one wall cell per step, against an 8-point circle.
- **Memory**: the XRAM layout is one struct in `src/xram.h`, mapped for the build with `rp6502_map()`.

## Credits

- **Development**: Jason F. Rowe
- **Hardware**: RP6502 PicoComputer by Picocomputer.
- **Inspiration**: Atari Super Sprint, Micro Machines (NES).

## License

This project is licensed under the GNU General Public License - see the LICENSE file for details.