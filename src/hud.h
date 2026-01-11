#ifndef HUD_H
#define HUD_H

// HUD Text Plane
#define MESSAGE_WIDTH 40
#define MESSAGE_HEIGHT 30
#define MESSAGE_LENGTH (MESSAGE_WIDTH * MESSAGE_HEIGHT + 1) // +1 for null terminator

extern char message[MESSAGE_LENGTH + 1]; // +1 for null terminator

#define HUD_COL_BG      0   // Black
#define HUD_COL_GREY    7   // Light Grey
#define HUD_COL_BLACK   8   // Dark Grey
#define HUD_COL_RED     9   // Bright Red
#define HUD_COL_GREEN   10  // Bright Green
#define HUD_COL_YELLOW  11  // Bright Yellow (Fixed)
#define HUD_COL_BLUE    12  // Bright Blue
#define HUD_COL_MAGENTA 13  // Bright Magenta
#define HUD_COL_CYAN    14  // Bright Cyan   (Fixed)
#define HUD_COL_WHITE   15  // Bright White

#define HUD_ROW 0
#define HUD_COL_LAPS 1
#define HUD_COL_MSG  15
#define HUD_COL_TIME 30

extern void hud_print(uint8_t x, uint8_t y, const char* str, uint8_t fg, uint8_t bg);
extern void hud_refresh_stats(uint8_t lap, uint16_t speed);
extern void update_countdown_display(uint16_t delay);
extern void update_title_screen(void);
extern void update_finished_screen(void);

#endif // HUD_H