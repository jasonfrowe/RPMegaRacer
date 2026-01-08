#include <stdint.h>
#include "player.h"
#include "constants.h"

// Starting position for the player's car
uint8_t startX = SCREEN_WIDTH / 2;
uint8_t startY = SCREEN_HEIGHT / 2; 

Car car = {0};

void init_player(Car* car, uint8_t startX, uint8_t startY) {
    car->x = startX;
    car->y = startY;
    car->velocity = 0;
    car->angle = 0;
    car->acceleration = 0;
}