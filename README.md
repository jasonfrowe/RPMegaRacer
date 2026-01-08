# RP Mega Racer

A top-down arcade racing game for the RP6502 PicoComputer, inspired by Atari's classic Sprint Racer series.

## About

RP Mega Racer brings the excitement of vintage arcade racing to the RP6502 PicoComputer. Race against the clock on challenging tracks filled with obstacles, tight turns, and hairpin curves. Navigate your race car with precision using keyboard controls while enjoying authentic OPL2 synthesized sound effects and music.

## Features

- **Classic Top-Down Racing Action**: Experience the thrill of vintage arcade racing with smooth scrolling graphics
- **Challenging Tracks**: Multiple race tracks with varying difficulty and layouts
- **Realistic Physics**: Momentum-based driving that rewards skillful control
- **OPL2 Audio**: Authentic retro sound effects and music powered by OPL2 synthesis
- **Keyboard Controls**: Responsive steering and acceleration using standard keyboard input
- **Score System**: Race against the clock and your best times

## Controls

- **Arrow Keys**: Steer your vehicle left/right, accelerate/brake
- **ESC**: Pause/Menu

## Building the Game

This project uses CMake and LLVM-MOS to build for the RP6502 PicoComputer.

### Requirements

- [LLVM-MOS](https://llvm-mos.org/wiki/Welcome) toolchain installed
- CMake 3.18 or newer
- Python 3 with pyserial for uploading to hardware
- [RP6502 PicoComputer](https://picocomputer.github.io)

### Build Instructions

```bash
mkdir -p build
cd build
cmake ..
make
```

The compiled `.rp6502` file will be ready to upload to your PicoComputer.

### Running on Hardware

Use F5 in VSCode to build and upload directly to your RP6502 PicoComputer connected via USB.

## Development

The game is written in C using the LLVM-MOS compiler targeting the 65C02 CPU. Graphics are rendered to the RP6502's canvas system, and audio is generated using the onboard OPL2 synthesizer.

## Credits

Inspired by Atari's Sprint series and classic top-down racing games of the 1970s-80s.

## License

See LICENSE file for details.
