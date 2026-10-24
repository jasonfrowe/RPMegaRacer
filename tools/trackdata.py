"""Shared loaders for a track folder: map, tiles, collision and terrain.

The rules here match src/track.c, so the tools see the same walls,
grass and finish line that the game does.
"""

from pathlib import Path

import numpy as np

MAP_W, MAP_H = 64, 48            # tiles
WORLD_W, WORLD_H = MAP_W * 8, MAP_H * 8

TERRAIN_ROAD, TERRAIN_GRASS, TERRAIN_WALL, TERRAIN_FINISH = 0, 1, 2, 3

# RGB555 palette from src/main.c (bit 5 is the alpha bit and is ignored here).
PALETTE = [
    0x0000, 0x0020, 0x41A7, 0x1AE0, 0x72AA, 0x0038, 0x003E, 0x0372,
    0x2C60, 0x35AE, 0x053C, 0x073E, 0x93AE, 0xC4B4, 0xD534, 0xF7BE,
]


def rgb(c):
    r = c & 0x1F
    g = (c >> 6) & 0x1F
    b = (c >> 11) & 0x1F
    return (r * 255 // 31, g * 255 // 31, b * 255 // 31)


def finish_tiles(track_id):
    if track_id == 3:
        return {14, 15, 44, 45}
    return set(range(243, 249))


class Track:
    def __init__(self, folder):
        folder = Path(folder)
        self.folder = folder
        self.id = int(folder.name[-2:])
        self.map = np.frombuffer((folder / "map.bin").read_bytes(), dtype=np.uint8).reshape(MAP_H, MAP_W)
        self.tiles = (folder / "tiles.bin").read_bytes()
        col = (folder / "collision.bin").read_bytes()
        self.masks = np.zeros((256, 8), dtype=np.uint8)
        n = len(col) // 8
        self.masks[:n] = np.frombuffer(col[: n * 8], dtype=np.uint8).reshape(n, 8)
        props = (folder / "properties.bin").read_bytes()
        self.props = np.full(256, TERRAIN_WALL, dtype=np.uint8)
        self.props[: len(props)] = np.frombuffer(props, dtype=np.uint8)
        self.terrain = self._terrain()

    def _terrain(self):
        """Per-pixel terrain, same rules as get_terrain_at() in track.c."""
        fin = finish_tiles(self.id)
        t = np.zeros((WORLD_H, WORLD_W), dtype=np.uint8)
        for ty in range(MAP_H):
            for tx in range(MAP_W):
                tid = int(self.map[ty, tx])
                block = np.full((8, 8), self.props[tid], dtype=np.uint8)
                if tid in fin:
                    block[:] = TERRAIN_FINISH
                else:
                    for py in range(8):
                        row = int(self.masks[tid, py])
                        for px in range(8):
                            if row & (0x80 >> px):
                                block[py, px] = TERRAIN_WALL
                t[ty * 8:ty * 8 + 8, tx * 8:tx * 8 + 8] = block
        return t

    def walls(self):
        return self.terrain == TERRAIN_WALL

    def image(self):
        """The track as the VGA draws it, as an RGB array."""
        pal = np.array([rgb(c) for c in PALETTE], dtype=np.uint8)
        img = np.zeros((WORLD_H, WORLD_W, 3), dtype=np.uint8)
        for ty in range(MAP_H):
            for tx in range(MAP_W):
                tid = int(self.map[ty, tx])
                data = self.tiles[tid * 32: tid * 32 + 32]
                if len(data) < 32:
                    continue
                for py in range(8):
                    for bx in range(4):
                        b = data[py * 4 + bx]
                        img[ty * 8 + py, tx * 8 + bx * 2] = pal[b >> 4]
                        img[ty * 8 + py, tx * 8 + bx * 2 + 1] = pal[b & 15]
        return img
