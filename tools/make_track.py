#!/usr/bin/env python3
"""Build the runtime data of a track: the wall bitmap and the AI racing line.

    make_track.py <track_dir> <out_dir> [--png file.png]

Reads map.bin, tiles.bin, collision.bin, properties.bin and waypoints.json
from <track_dir>, and writes to <out_dir>:

  walls.bin  The walls at 2x2 pixel cells, 256x192 bits, one bit per cell,
             row major, bit 7 of each byte the leftmost cell. A cell is a
             wall when any of its 4 pixels is. The cars collide with this.
             Cells are never thinner than one cell and stay 8-connected, so
             a car that moves at most one cell per test can't slip through.

  path.bin   The racing line the AI drives, as a closed loop of N points
             about 8 px apart, starting at the finish line:
               uint16 N
               int16  x[N], y[N]   pixels
               uint8  heading[N]   game angle to the next point (0 = up,
                                   64 = left, counter clockwise)
               uint8  room_l[N]    how far a car may move off the line to
               uint8  room_r[N]    its left / right and still be clear
               uint8  speed[N]     target speed, 1/64 px per frame

waypoints.json is a rough guide in race order, the first point on the
finish line. The racing line comes from it by an elastic band: the loop
is pulled tight, which takes the inside of each corner, while the walls
push it back to a safe distance.
"""

import argparse
import json
import math
import struct
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parent))
from trackdata import Track, TERRAIN_FINISH, TERRAIN_GRASS, WORLD_W, WORLD_H  # noqa: E402

CELL = 2
CELLS_W, CELLS_H = WORLD_W // CELL, WORLD_H // CELL

CLEARANCE = 13.0    # px from the line to a wall, where the track allows
CAR_ROOM = 8.0      # px a car's centre keeps from a wall when off the line
SPACING = 8.0       # px between points of the racing line
MAX_POINTS = 250

# Speed model, in px and frames. Keep near the car in src/car.c.
TURN_RATE = 0.060   # rad per frame the AI is happy to turn at speed
BRAKE = 0.045       # px per frame per frame of braking
TOP_SPEED = 3.9     # px per frame, the cap on a hint


def wall_cells(track):
    w = track.walls()
    return w.reshape(CELLS_H, CELL, CELLS_W, CELL).any(axis=(1, 3))


def edt_1d(f):
    """Squared distance transform of a 1-D sampled function (Felzenszwalb)."""
    n = len(f)
    d = np.zeros(n)
    v = np.zeros(n, dtype=int)
    z = np.zeros(n + 1)
    k = 0
    v[0] = 0
    z[0] = -math.inf
    z[1] = math.inf
    for q in range(1, n):
        while True:
            s = ((f[q] + q * q) - (f[v[k]] + v[k] * v[k])) / (2 * q - 2 * v[k])
            if s <= z[k]:
                k -= 1
            else:
                break
        k += 1
        v[k] = q
        z[k] = s
        z[k + 1] = math.inf
    k = 0
    for q in range(n):
        while z[k + 1] < q:
            k += 1
        d[q] = (q - v[k]) ** 2 + f[v[k]]
    return d


def distance_field(walls):
    """Euclidean distance in px from each pixel to the nearest wall pixel."""
    big = 1e9
    f = np.where(walls, 0.0, big)
    h, w = f.shape
    tmp = np.zeros_like(f)
    for x in range(w):
        tmp[:, x] = edt_1d(f[:, x])
    out = np.zeros_like(f)
    for y in range(h):
        out[y, :] = edt_1d(tmp[y, :])
    return np.sqrt(out)


def sample(field, x, y):
    """Bilinear sample of a field at fractional pixel positions."""
    h, w = field.shape
    x = np.clip(x, 0, w - 1.001)
    y = np.clip(y, 0, h - 1.001)
    x0 = np.floor(x).astype(int)
    y0 = np.floor(y).astype(int)
    fx = x - x0
    fy = y - y0
    a = field[y0, x0] * (1 - fx) + field[y0, x0 + 1] * fx
    b = field[y0 + 1, x0] * (1 - fx) + field[y0 + 1, x0 + 1] * fx
    return a * (1 - fy) + b * fy


def resample(pts, spacing):
    """Resample a closed polyline to points `spacing` apart."""
    closed = np.vstack([pts, pts[:1]])
    seg = np.linalg.norm(np.diff(closed, axis=0), axis=1)
    total = seg.sum()
    n = max(8, int(round(total / spacing)))
    cum = np.concatenate([[0], np.cumsum(seg)])
    t = np.linspace(0, total, n, endpoint=False)
    x = np.interp(t, cum, closed[:, 0])
    y = np.interp(t, cum, closed[:, 1])
    return np.stack([x, y], axis=1)


def segment_hits_wall(walls, a, b):
    n = int(max(abs(b[0] - a[0]), abs(b[1] - a[1]))) * 2 + 1
    xs = np.linspace(a[0], b[0], n).round().astype(int)
    ys = np.linspace(a[1], b[1], n).round().astype(int)
    ok = (xs >= 0) & (xs < WORLD_W) & (ys >= 0) & (ys < WORLD_H)
    if not ok.all():
        return True
    return walls[ys, xs].any()


def laplacian(pts):
    return (np.roll(pts, 1, axis=0) + np.roll(pts, -1, axis=0)) / 2 - pts


def elastic_band(guide, walls, dist, iters=1500, fair_iters=1500):
    """Pull the loop tight (shortest line), then fair it (least curvature)."""
    pts = resample(np.array(guide, dtype=float), 3.0)
    for i in range(len(pts)):
        if segment_hits_wall(walls, pts[i], pts[(i + 1) % len(pts)]):
            sys.exit(f"waypoints.json: the guide crosses a wall near {pts[i].round()}")
    gy, gx = np.gradient(dist)
    for it in range(iters + fair_iters):
        if it < iters:
            new = pts + 0.35 * laplacian(pts)
        else:
            # Least curvature over a wide window, so corners get wide arcs.
            spread = pts
            for _ in range(3):
                spread = spread + 0.5 * laplacian(spread)
            new = pts - 0.5 * laplacian(laplacian(spread))
        d = sample(dist, new[:, 0], new[:, 1])
        short = np.maximum(CLEARANCE - d, 0)
        g = np.stack([sample(gx, new[:, 0], new[:, 1]), sample(gy, new[:, 0], new[:, 1])], axis=1)
        gn = np.linalg.norm(g, axis=1, keepdims=True) + 1e-9
        new = new + 0.6 * short[:, None] * g / gn
        # Never step into, or through, a wall.
        step = np.clip(new - pts, -0.75, 0.75)
        new = pts + step
        d_new = sample(dist, new[:, 0], new[:, 1])
        keep = d_new > 1.5
        pts = np.where(keep[:, None], new, pts)
        if it % 25 == 24:
            pts = resample(pts, 3.0)
    return pts


def game_angle(dx, dy):
    """Game angle of a direction: forward is (-sin, -cos), 256 per turn."""
    a = math.atan2(-dx, -dy)
    return int(round(a * 256 / (2 * math.pi))) & 0xFF


def room(dist, p, nx, ny):
    """How far a car's centre may go along (nx, ny) and keep clear."""
    r = 0.0
    while r < 60:
        q = p + (r + 1) * np.array([nx, ny])
        if sample(dist, np.array([q[0]]), np.array([q[1]]))[0] < CAR_ROOM:
            break
        r += 1
    return int(r)


def finish_centre(track, start):
    """Centre of the finish line patch that the first waypoint is on."""
    fin = track.terrain == TERRAIN_FINISH
    sx, sy = int(start[0]), int(start[1])
    ys, xs = np.nonzero(fin)
    if len(xs) == 0:
        return np.array(start, dtype=float)
    # The patch connected to the finish pixel nearest the first waypoint.
    i = np.argmin((xs - sx) ** 2 + (ys - sy) ** 2)
    seen = np.zeros_like(fin)
    stack = [(ys[i], xs[i])]
    seen[ys[i], xs[i]] = True
    pts = []
    while stack:
        y, x = stack.pop()
        pts.append((x, y))
        for ny, nx in ((y + 1, x), (y - 1, x), (y, x + 1), (y, x - 1)):
            if 0 <= ny < WORLD_H and 0 <= nx < WORLD_W and fin[ny, nx] and not seen[ny, nx]:
                seen[ny, nx] = True
                stack.append((ny, nx))
    return np.array(pts, dtype=float).mean(axis=0)


def build(track_dir, out_dir, png=None):
    track = Track(track_dir)
    cells = wall_cells(track)
    walls_px = np.repeat(np.repeat(cells, CELL, axis=0), CELL, axis=1)
    dist = distance_field(walls_px)

    guide = json.loads((Path(track_dir) / "waypoints.json").read_text())
    line = elastic_band(guide, walls_px, dist)

    spacing = max(SPACING, float(np.linalg.norm(np.diff(np.vstack([line, line[:1]]), axis=0), axis=1).sum()) / MAX_POINTS)
    pts = resample(line, spacing)

    # Point 0 is where the line crosses the finish.
    fc = finish_centre(track, guide[0])
    start = int(np.argmin(np.linalg.norm(pts - fc, axis=1)))
    pts = np.roll(pts, -start, axis=0)
    n = len(pts)

    nxt = np.roll(pts, -1, axis=0)
    d = nxt - pts
    theta = np.arctan2(d[:, 1], d[:, 0])
    heading = [game_angle(dx, dy) for dx, dy in d]

    # Curvature from the turn over a window either side, then a speed.
    w = 2
    turn = np.zeros(n)
    for i in range(n):
        a = theta[(i - w) % n]
        b = theta[(i + w - 1) % n]
        turn[i] = abs((b - a + math.pi) % (2 * math.pi) - math.pi) / (2 * w - 1)
    kappa = turn / spacing
    v = np.minimum(TOP_SPEED, TURN_RATE / np.maximum(kappa, 1e-6))
    # Brake ahead of the corners: two passes round the loop.
    for _ in range(2):
        for i in range(n - 1, -1, -1):
            j = (i + 1) % n
            v[i] = min(v[i], math.sqrt(v[j] ** 2 + 2 * BRAKE * spacing))

    room_l, room_r = [], []
    for i in range(n):
        dx, dy = d[i] / (np.linalg.norm(d[i]) + 1e-9)
        # Right of travel is (-dy, dx) in screen coordinates (y down).
        room_r.append(room(dist, pts[i], -dy, dx))
        room_l.append(room(dist, pts[i], dy, -dx))

    out = Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)

    bits = np.packbits(cells, axis=1)
    (out / "walls.bin").write_bytes(bits.tobytes())

    # Grass, one bit per 8x8 tile, from the tile properties.
    tiles_grass = track.props[track.map] == TERRAIN_GRASS
    (out / "grass.bin").write_bytes(np.packbits(tiles_grass, axis=1).tobytes())

    blob = bytearray(struct.pack("<H", n))
    xy = pts.round().astype(int)
    blob += struct.pack(f"<{n}h", *xy[:, 0])
    blob += struct.pack(f"<{n}h", *xy[:, 1])
    blob += bytes(heading)
    blob += bytes(min(255, r) for r in room_l)
    blob += bytes(min(255, r) for r in room_r)
    blob += bytes(min(255, int(s * 64)) for s in v)
    (out / "path.bin").write_bytes(blob)

    print(f"{Path(track_dir).name}: {n} points, {spacing:.1f} px apart, "
          f"lap {n * spacing:.0f} px, slowest hint {v.min():.2f} px/f, "
          f"narrowest room {min(min(room_l), min(room_r))} px")

    if png:
        from PIL import Image, ImageDraw
        img = track.image().copy()
        img[walls_px] = (img[walls_px] * 0.3 + np.array([255, 0, 0]) * 0.7).astype(np.uint8)
        s = 2
        im = Image.fromarray(img).resize((WORLD_W * s, WORLD_H * s), Image.NEAREST)
        dr = ImageDraw.Draw(im)
        for i in range(n):
            x, y = pts[i]
            dx, dy = d[i] / (np.linalg.norm(d[i]) + 1e-9)
            rx, ry = -dy, dx
            dr.line([((x - rx * room_l[i]) * s, (y - ry * room_l[i]) * s),
                     ((x + rx * room_r[i]) * s, (y + ry * room_r[i]) * s)], fill=(60, 60, 140))
        for i in range(n):
            x, y = pts[i]
            x2, y2 = pts[(i + 1) % n]
            t = min(1.0, v[i] / 3.0)
            col = (int(255 * (1 - t)), int(255 * t), 0)
            dr.line([(x * s, y * s), (x2 * s, y2 * s)], fill=col, width=3)
        x, y = pts[0]
        dr.ellipse((x * s - 6, y * s - 6, x * s + 6, y * s + 6), outline=(255, 255, 255), width=2)
        im.save(png)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("track_dir")
    ap.add_argument("out_dir")
    ap.add_argument("--png", help="also draw the racing line over the track")
    args = ap.parse_args()
    build(args.track_dir, args.out_dir, args.png)


if __name__ == "__main__":
    main()
