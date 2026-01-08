#ifndef PLAYER_H
#define PLAYER_H

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t velocity;
    uint8_t angle;
    uint8_t acceleration;
} Car;

extern uint8_t startX;
extern uint8_t startY;
extern Car car;

#endif // PLAYER_H