#!/usr/bin/env python3
"""Run an emulator script against the built ROM, with symbols filled in.

The emulator's --script language addresses memory by number, and the
numbers move every build. Write {name} in a script instead, and this
fills in the address of that symbol from the ELF. {name+1} works too.

Built-in scripts (pass the name instead of a file):
  race     the AI drives every car, the player's too, for a whole race on
           --track; prints each lap and finish from the console, and the
           dropped frame count, and saves screenshots to --out
  drive    the player holds the gas and steers nowhere for a few seconds,
           then reverses: a car pinned on a wall must get free

Usage:
  tools/playtest.py race [--track N] [--out DIR]
  tools/playtest.py my_script.txt [--out DIR]
  tools/playtest.py race --phi2 4000   # the 6502 at half speed: does it keep up?
"""

import argparse
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build" / "llvm-mos" / "release"
ROM = BUILD / "RPMegaRacer.rp6502"
ELF = BUILD / "RPMegaRacer.elf"
NM = Path.home() / ".rp6502" / "llvm-mos" / "bin" / "llvm-nm"
EMU = ROOT / "tools" / "rp6502-emu"

# STATE_FINISHED is 3.
BUILTIN = {
    "race": (
        "run 30\npoke {autopilot} 1\n{pick_track}"
        "press space\nrun 2\nrelease space\n"
        + "".join(f"run 600\nshot \"{{out}}/race_{i:02}.png\"\n" for i in range(1, 9))
        + "wait {game_state} 3 9000\nrun 2\ndump {lag_frames} 1\n"
    ),
    "drive": (
        "run 30\n{pick_track}press space\nrun 2\nrelease space\nrun 240\n"
        "press space\nrun 240\nshot \"{out}/drive_1.png\"\n"
        "press left\nrun 40\nrelease left\nrun 200\nshot \"{out}/drive_2.png\"\n"
        "release space\npress x\nrun 60\nrelease x\npress space right\nrun 50\nrelease right\nrun 200\n"
        "shot \"{out}/drive_3.png\"\nrelease space\ndump {lag_frames} 1\n"
    ),
}


def symbols():
    out = subprocess.run([str(NM), str(ELF)], capture_output=True, text=True, check=True).stdout
    syms = {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3:
            syms[parts[2]] = int(parts[0], 16)
    return syms


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("script")
    ap.add_argument("--out", default=tempfile.gettempdir())
    ap.add_argument("--seed", default="3")
    ap.add_argument("--track", type=int, default=1, help="the track a built-in script races (1 up)")
    ap.add_argument("--phi2", help="6502 clock in kHz (default 8000); lower it to measure CPU headroom")
    args = ap.parse_args()
    text = BUILTIN.get(args.script)
    if text is None:
        text = Path(args.script).read_text()
    syms = symbols()

    def sub(m):
        name, plus = m.group(1), m.group(2)
        if name == "out":
            return args.out
        if name == "pick_track":
            # track_load() runs on the title screen through a debug hook.
            return f"poke {{debug_track}} {args.track}\nrun 2\n" if args.track != 1 else ""
        if name not in syms:
            sys.exit(f"unknown symbol {name}")
        return hex(syms[name] + (int(plus) if plus else 0))

    # Two passes: {pick_track} expands to more {symbols}.
    pattern = r"\{(\w+)(?:\+(\d+))?\}"
    text = re.sub(pattern, sub, re.sub(pattern, lambda m: sub(m) if m.group(1) == "pick_track" else m.group(0), text))
    with tempfile.NamedTemporaryFile("w", suffix=".txt", delete=False) as f:
        f.write(text)
    cmd = [str(EMU), str(ROM), "--seed", args.seed, "--mute", "--script", f.name]
    if args.phi2:
        cmd += ["--phi2", args.phi2]
    r = subprocess.run(cmd)
    sys.exit(r.returncode)


if __name__ == "__main__":
    main()
