#ifndef TRIG_H
#define TRIG_H

#include <stdint.h>

/* Game angles are 256 per turn: 0 is up, 64 is left (counter clockwise).
 * Forward for angle a is (-sin a, -cos a) in screen coordinates. */
extern const int8_t SIN_LUT[256];
extern const int16_t TX_LUT[256];
extern const int16_t TY_LUT[256];

#define SIN(a) (SIN_LUT[(uint8_t)(a)])
#define COS(a) (SIN_LUT[(uint8_t)((a) + 64)])

/* Standard angle of (dx, dy): 0 = +x, 64 = +y. */
uint8_t atan2_8(int16_t dy, int16_t dx);

/* Game angle that points along (dx, dy). */
uint8_t heading_to(int16_t dx, int16_t dy);

#endif
