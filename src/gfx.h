#ifndef GFX_H
#define GFX_H

#include <stdint.h>

void gfx_init(void);

/* Writes the sprites and scroll that gfx_prepare() worked out. Call
 * straight after vsync, so a frame never shows half of each. */
void gfx_commit(void);

/* Works out the camera and the car sprites for the next frame. */
void gfx_prepare(void);

void gfx_title_show(void);
void gfx_title_hide(void);

#endif
