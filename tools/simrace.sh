#!/bin/sh
# Races the AI round a track in the headless emulator, flat out, and
# prints each lap and finish:  tools/simrace.sh <track> [drs]
# With drs, the player's car (car0) uses DRS to pass.
cd "$(dirname "$0")/.." || exit 1
exec ./tools/rp6502-emu build/llvm-mos/release/RPMegaRacer.rp6502 \
    --headless --mute --seed 1 --phi2 0 -- race "$1" $2 | grep -v '^Music'
