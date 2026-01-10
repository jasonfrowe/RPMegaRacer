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


#endif // HUD_H