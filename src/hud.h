#ifndef HUD_H
#define HUD_H

#include <stdint.h>

/* ANSI colours of the built-in text palette. */
#define HUD_COL_BG 0
#define HUD_COL_GREY 7
#define HUD_COL_DARK 8
#define HUD_COL_RED 9
#define HUD_COL_GREEN 10
#define HUD_COL_YELLOW 11
#define HUD_COL_BLUE 12
#define HUD_COL_MAGENTA 13
#define HUD_COL_CYAN 14
#define HUD_COL_WHITE 15

void hud_print(uint8_t x, uint8_t y, const char *str, uint8_t fg, uint8_t bg);
void hud_center(uint8_t y, const char *str, uint8_t fg);
void hud_clear(void);
void hud_clear_row(uint8_t y);

void hud_race(void);       /* lap, time, place, DRS */
void hud_countdown(uint16_t t);
void hud_title(void);
void hud_results(void);

#endif
