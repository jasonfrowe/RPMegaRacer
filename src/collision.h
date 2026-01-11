#ifndef COLLISION_H
#define COLLISION_H

#include "player.h"
extern void resolve_car_collision(Car *c1, Car *c2);
extern void resolve_ai_ai_collision(AICar *a1, AICar *a2);


#endif // COLLISION_H