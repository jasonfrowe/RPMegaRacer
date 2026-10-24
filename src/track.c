#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <rp6502.h>
#include "track.h"
#include "xram.h"

uint8_t walls[WALL_ROWS][WALL_COLS];
uint8_t grass[48][8];

uint8_t path_n;
int16_t path_x[PATH_MAX];
int16_t path_y[PATH_MAX];
uint8_t path_heading[PATH_MAX];
uint8_t path_room_l[PATH_MAX];
uint8_t path_room_r[PATH_MAX];
uint8_t path_speed[PATH_MAX];

uint8_t current_track = 1;

const uint8_t bit_mask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

static int open_part(uint8_t id, const char *part)
{
    char name[32];
    sprintf(name, "ROM:track%02d_%s.bin", id, part);
    int fd = open(name, O_RDONLY);
    if (fd < 0)
        printf("Can't open %s\n", name);
    return fd;
}

static void read_all(int fd, void *dest, uint16_t count)
{
    uint8_t *p = dest;
    while (count) {
        int n = read(fd, p, count > 256 ? 256 : count);
        if (n <= 0)
            break;
        p += n;
        count -= (uint16_t)n;
    }
}

static void load_xram(uint8_t id, const char *part, uint16_t addr, uint16_t count)
{
    int fd = open_part(id, part);
    if (fd < 0)
        return;
    read_xram(addr, count, fd);
    close(fd);
}

static void load_ram(uint8_t id, const char *part, void *dest, uint16_t count)
{
    int fd = open_part(id, part);
    if (fd < 0)
        return;
    read_all(fd, dest, count);
    close(fd);
}

static void load_path(uint8_t id)
{
    int fd = open_part(id, "path");
    path_n = 0;
    if (fd < 0)
        return;
    uint16_t n = 0;
    read_all(fd, &n, 2);
    if (n > PATH_MAX)
        n = PATH_MAX;
    read_all(fd, path_x, n * 2);
    read_all(fd, path_y, n * 2);
    read_all(fd, path_heading, n);
    read_all(fd, path_room_l, n);
    read_all(fd, path_room_r, n);
    read_all(fd, path_speed, n);
    close(fd);
    path_n = (uint8_t)n;
}

void track_load(uint8_t id)
{
    current_track = id;
    load_xram(id, "map", XRAM_TRACK_MAP, MAP_BYTES);
    load_xram(id, "tiles", XRAM_TRACK_TILES, TILESET_BYTES);
    memset(walls, 0xFF, sizeof(walls));
    load_ram(id, "walls", walls, sizeof(walls));
    memset(grass, 0, sizeof(grass));
    load_ram(id, "grass", grass, sizeof(grass));
    load_path(id);
}

uint8_t path_nearest(int16_t x, int16_t y)
{
    uint8_t best = 0;
    uint16_t best_d = 0xFFFF;
    for (uint8_t i = 0; i < path_n; i++) {
        int16_t dx = path_x[i] - x;
        int16_t dy = path_y[i] - y;
        if (dx < 0)
            dx = -dx;
        if (dy < 0)
            dy = -dy;
        /* Octagonal distance: no multiplies, good to within 8%. */
        uint16_t d = dx > dy ? dx + (dy >> 1) : dy + (dx >> 1);
        if (d < best_d) {
            best_d = d;
            best = i;
        }
    }
    return best;
}
