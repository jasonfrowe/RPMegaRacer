/* Wall skim test: runs src/car.c natively and drives a car along every wall
 * of a track, from every other racing line point, on each side, leaning a
 * little into the wall the way a player squeezing past would. It reports
 * how much speed the cars keep. Use it to tune the wall settings in car.h.
 *
 *   cc -O1 -Isrc -o build/wallskim tools/wallskim.c src/car.c src/trig.c
 *   build/wallskim tracks/track01 [lean angle units, default 4] [frames, default 90]
 *
 * A start speed of 450 is near top speed; "dropped below 250" counts runs
 * that lost about half of it. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "car.h"
#include "track.h"
#include "trig.h"

uint8_t walls[WALL_ROWS][WALL_COLS];
uint8_t grass[48][8];
uint8_t path_n;
int16_t path_x[PATH_MAX], path_y[PATH_MAX];
uint8_t path_heading[PATH_MAX], path_room_l[PATH_MAX], path_room_r[PATH_MAX], path_speed[PATH_MAX];
const uint8_t bit_mask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

static void load(const char *dir)
{
    char p[256];
    FILE *f;
    sprintf(p, "%s/walls.bin", dir); f = fopen(p, "rb"); fread(walls, 1, sizeof walls, f); fclose(f);
    sprintf(p, "%s/grass.bin", dir); f = fopen(p, "rb"); fread(grass, 1, sizeof grass, f); fclose(f);
    sprintf(p, "%s/path.bin", dir); f = fopen(p, "rb");
    uint16_t n; fread(&n, 2, 1, f); path_n = n;
    fread(path_x, 2, n, f); fread(path_y, 2, n, f); fread(path_heading, 1, n, f);
    fread(path_room_l, 1, n, f); fread(path_room_r, 1, n, f); fread(path_speed, 1, n, f);
    fclose(f);
}

int main(int argc, char **argv)
{
    int angle = argc > 2 ? atoi(argv[2]) : 4;
    int frames = argc > 3 ? atoi(argv[3]) : 90;
    load(argv[1]);
    int runs = 0, stops = 0, hits = 0;
    long kept = 0;
    int worst = 9999, worst_i = -1;
    for (int i = 0; i < path_n; i += 2) {
        for (int side = -1; side <= 1; side += 2) {
            uint8_t h = path_heading[i];
            int room = side > 0 ? path_room_r[i] : path_room_l[i];
            /* Start 3 px out past the edge of the AI's room: close to the wall. */
            int off = side * (room + 3);
            int x = path_x[i] + (off * COS(h) >> 7);
            int y = path_y[i] - (off * SIN(h) >> 7);
            Car c;
            car_place(&c, x, y, (uint8_t)(h - side * angle)); /* nose toward that wall */
            c.accel = ACCEL_BASE;
            uint8_t a = (uint8_t)(c.heading >> 8);
            c.vx = -(450 * SIN(a)) / 127;
            c.vy = -(450 * COS(a)) / 127;
            c.speed = 450;
            c.gas = true;
            int min_sp = 9999, wall = 0;
            uint8_t wp = (uint8_t)i;
            for (int f = 0; f < frames; f++) {
                /* A driver hugging that wall: follow the line at this offset,
                 * leaning angle units into the wall. */
                int px = PX(c.x), py = PX(c.y);
                for (int k = 0; k < 3; k++) {
                    uint8_t n = path_next(wp);
                    int dn = (path_x[n]-px)*(path_x[n]-px)+(path_y[n]-py)*(path_y[n]-py);
                    int d0 = (path_x[wp]-px)*(path_x[wp]-px)+(path_y[wp]-py)*(path_y[wp]-py);
                    if (dn <= d0) wp = n; else break;
                }
                uint8_t t = path_add(wp, 3);
                uint8_t th = path_heading[t];
                int troom = side > 0 ? path_room_r[t] : path_room_l[t];
                int toff = side * (troom + 3);
                int tx = path_x[t] + (toff * COS(th) >> 7);
                int ty = path_y[t] - (toff * SIN(th) >> 7);
                int8_t err = (int8_t)(heading_to(tx - px, ty - py) - side * angle - (uint8_t)(c.heading >> 8));
                int st = err * 2;
                if (st > 16) st = 16;
                if (st < -16) st = -16;
                c.steer = st;
                car_update(&c);
                if (c.wall_hit == 7) wall++;
                if (c.speed < min_sp) min_sp = c.speed;
            }
            runs++;
            kept += c.speed;
            hits += wall ? 1 : 0;
            if (min_sp < 250) stops++;
            if (min_sp < worst) { worst = min_sp; worst_i = i * 2 + (side > 0); }
        }
    }
    printf("%-10s angle %d: runs %d, touched wall %d, dropped below 250: %d, mean end speed %ld, worst %d (point %d %s)\n",
           argv[1] + strlen(argv[1]) - 7, angle, runs, hits, stops, kept / runs, worst, worst_i / 2, worst_i & 1 ? "right" : "left");
    return 0;
}
