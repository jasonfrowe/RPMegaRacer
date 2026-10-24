# Collision System

## Walls

`tools/make_track.py` builds a wall grid for each track from the tile collision masks: 256x192 cells of 2x2 pixels, one bit each (`walls.bin`, 6 KB). A cell is a wall if any of its pixels is.

A car is a circle of 8 points, radius 4 px, around its centre (`car_blocked()` in `src/car.c`). Each frame the car moves along x, then along y, at most 2 px (one cell) per step. Running close to a wall is meant to be easy, since passing often means squeezing by on the inside:

- **Repulsion**: a ring of 8 points at 7 px pushes the car gently off any wall it nears (`WALL_REPEL`), so it eases away before touching.
- **Bumps**: a step blocked by a wall that a sideways shift of up to 3 px clears (`WALL_STEP`) is a bump in the wall, and the car rides over it instead of stopping.
- **Touches**: the speed into the wall stops. Below `WALL_TOUCH` that's all; harder hits bounce back a little (`WALL_BOUNCE`) and lose speed along the wall in proportion to how hard they hit (`WALL_FRICTION_SHIFT`).
- **Lining up**: a hit at a shallow angle turns the car a little to run along the wall (`WALL_ALIGN_RANGE`, `WALL_ALIGN_STEP`).

`tools/wallskim.c` measures all of this on every wall of a track.

Because a step never crosses more than one cell, and the grid keeps even a 1-pixel diagonal barrier connected, a car can't pass through a thin wall.

If a car ever does end up in a wall (it shouldn't), `car_unstick()` moves it to the nearest clear spot.

## Cars

Two cars touch when their centres are closer than 10 px (`car_contact()`). They're pushed apart along the line between them, and only the part of their speed along that line is traded, with a low bounce (`CAR_BOUNCE`). There's no spin and no stun, so a nudge is a nudge. In a shove with the AI, the player gives a little less ground.

## Terrain

Grass (`grass.bin`, one bit per tile) adds drag and cuts grip.
