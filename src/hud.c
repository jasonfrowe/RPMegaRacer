#include <rp6502.h>
#include <string.h>
#include "hud.h"
#include "race.h"
#include "track.h"
#include "xram.h"

void hud_print(uint8_t x, uint8_t y, const char *str, uint8_t fg, uint8_t bg)
{
    if (y >= HUD_ROWS || x >= HUD_COLS)
        return;
    RIA.addr0 = XRAM_HUD + (y * HUD_COLS + x) * sizeof(mode1_8bpp_data_t);
    RIA.step0 = 1;
    while (*str && x < HUD_COLS) {
        RIA.rw0 = (uint8_t)*str++;
        RIA.rw0 = fg;
        RIA.rw0 = bg;
        x++;
    }
}

void hud_center(uint8_t y, const char *str, uint8_t fg)
{
    hud_print((uint8_t)((HUD_COLS - strlen(str)) / 2), y, str, fg, HUD_COL_BG);
}

void hud_clear_row(uint8_t y)
{
    xram0_set(XRAM_HUD + y * HUD_COLS * sizeof(mode1_8bpp_data_t), 0,
              HUD_COLS * sizeof(mode1_8bpp_data_t));
}

void hud_clear(void)
{
    xram0_set(XRAM_HUD, 0, HUD_ROWS * HUD_COLS * sizeof(mode1_8bpp_data_t));
}

static char *put2(char *p, uint8_t v)
{
    *p++ = (char)('0' + v / 10);
    *p++ = (char)('0' + v % 10);
    return p;
}

static const char *const ordinal[] = {"1ST", "2ND", "3RD", "4TH"};
static uint8_t hud_turn;

void hud_race(void)
{
    const Racer *p = &racers[PLAYER];
    char buf[16];

    /* The clock every frame; the rest takes turns, to spread the work. */
    char *q = put2(buf, race_minutes);
    *q++ = ':';
    q = put2(q, race_seconds);
    *q++ = ':';
    q = put2(q, (uint8_t)(race_frames * 5 / 3)); /* hundredths */
    *q = 0;
    hud_print(16, 0, buf, HUD_COL_WHITE, HUD_COL_BG);

    switch (hud_turn++ & 3) {
    case 0: {
        /* LAP 1/5 and POS 1ST */
        int8_t lap = p->lap < 1 ? 1 : (p->lap > LAPS ? LAPS : p->lap);
        memcpy(buf, "LAP 1/5", 8);
        buf[4] = (char)('0' + lap);
        buf[6] = (char)('0' + LAPS);
        hud_print(1, 0, buf, lap == LAPS ? HUD_COL_YELLOW : HUD_COL_WHITE, HUD_COL_BG);
        hud_print(32, 0, "POS", HUD_COL_WHITE, HUD_COL_BG);
        hud_print(36, 0, ordinal[p->place - 1], p->place == 1 ? HUD_COL_GREEN : HUD_COL_YELLOW, HUD_COL_BG);
        break;
    }
    case 1: {
        /* DRS [==========] and its state */
        uint8_t seg, col;
        const char *state;
        if (p->drs_timer) {
            seg = (uint8_t)((p->drs_timer * 10 + DRS_TIME - 1) / DRS_TIME);
            col = HUD_COL_YELLOW;
            state = "OPEN    ";
        } else if (p->drs_charge >= DRS_FULL) {
            seg = 10;
            col = (RIA.vsync & 8) ? HUD_COL_CYAN : HUD_COL_WHITE;
            state = "READY   ";
        } else {
            seg = (uint8_t)(p->drs_charge / (DRS_FULL / 10));
            col = p->drs_in_range ? HUD_COL_GREEN : HUD_COL_RED;
            state = p->drs_in_range ? "IN RANGE" : (p->place == 1 ? "LEADING " : "        ");
        }
        char bar[13];
        bar[0] = '[';
        for (uint8_t i = 1; i <= 10; i++)
            bar[i] = i <= seg ? (char)0xDB : '-';
        bar[11] = ']';
        bar[12] = 0;
        hud_print(1, 1, "DRS", HUD_COL_WHITE, HUD_COL_BG);
        hud_print(5, 1, bar, col, HUD_COL_BG);
        hud_print(18, 1, state, col, HUD_COL_BG);
        break;
    }
    case 2: {
        /* Speed, about MPH */
        int16_t sp = p->car.speed < 0 ? -p->car.speed : p->car.speed;
        uint8_t mph = (uint8_t)(sp >> 2);
        buf[0] = (char)(mph >= 100 ? '0' + mph / 100 : ' ');
        buf[1] = (char)(mph >= 10 ? '0' + mph / 10 % 10 : ' ');
        buf[2] = (char)('0' + mph % 10);
        memcpy(buf + 3, " MPH", 5);
        hud_print(32, 1, buf, HUD_COL_CYAN, HUD_COL_BG);
        break;
    }
    case 3:
        if (p->lap == LAPS && !p->finished)
            hud_center(4, (RIA.vsync & 16) ? "FINAL LAP" : "         ", HUD_COL_YELLOW);
        else
            hud_clear_row(4);
        break;
    }
}

void hud_countdown(uint16_t t)
{
    /* t counts down from 240: 3, 2, 1 a second each, then GO. */
    if (t > 180)
        hud_center(13, "  3  ", HUD_COL_RED);
    else if (t > 120)
        hud_center(13, "  2  ", HUD_COL_YELLOW);
    else if (t > 60)
        hud_center(13, "  1  ", HUD_COL_WHITE);
    else
        hud_center(13, " GO! ", HUD_COL_GREEN);
}

void hud_title(void)
{
    static const uint8_t rainbow[] = {
        196, 202, 208, 214, 220, 226, 190, 154, 118, 82, 46, 47, 48, 49, 50,
        51, 45, 39, 33, 27, 21, 57, 93, 129, 165, 201, 200, 199, 198, 197,
    };
    uint8_t col = rainbow[(RIA.vsync >> 2) % sizeof(rainbow)];
    hud_center(18, " PRESS FIRE TO START ", col);
}

void hud_results(void)
{
    const Racer *p = &racers[PLAYER];
    hud_center(11, " RACE FINISHED ", HUD_COL_CYAN);
    if (p->place == 1) {
        hud_center(13, " YOU WON! ", HUD_COL_GREEN);
        hud_center(15, current_track < NUM_TRACKS ? " ON TO THE NEXT TRACK " : " CHAMPION OF EVERY TRACK ", HUD_COL_YELLOW);
    } else {
        char buf[] = " YOU FINISHED 0TH ";
        memcpy(buf + 14, ordinal[p->place - 1], 3);
        hud_center(13, buf, HUD_COL_RED);
        hud_center(15, " WIN TO ADVANCE ", HUD_COL_GREY);
    }
}
