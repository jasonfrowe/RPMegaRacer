# Collision System

## Walls

`tools/make_track.py` builds a wall grid for each track from the tile collision masks: 256x192 cells of 2x2 pixels, one bit each (`walls.bin`, 6 KB). A cell is a wall if any of its pixels is.

A car is a circle of 8 points, radius 5 px, around its centre (`car_blocked()` in `src/car.c`). Each frame the car moves along x, then along y, at most 2 px (one cell) per step. An axis that would hit a wall stops, bounces back a little (`WALL_BOUNCE`), and the other axis keeps going with a little scrape (`WALL_SCRAPE`), so a car slides along a wall instead of sticking.

Because a step never crosses more than one cell, and the grid keeps even a 1-pixel diagonal barrier connected, a car can't pass through a thin wall.

If a car ever does end up in a wall (it shouldn't), `car_unstick()` moves it to the nearest clear spot.

## Cars

Two cars touch when their centres are closer than 10 px (`car_contact()`). They're pushed apart along the line between them, and only the part of their speed along that line is traded, with a low bounce (`CAR_BOUNCE`). There's no spin and no stun, so a nudge is a nudge. In a shove with the AI, the player gives a little less ground.

## Terrain

Grass (`grass.bin`, one bit per tile) adds drag and cuts grip.
