#ifndef COLLISION_H
#define COLLISION_H

#include "player.h"
extern void resolve_player_ai_collision(Car *p, AICar *ai);
extern void resolve_ai_ai_collision(AICar *a1, AICar *a2);


#endif // COLLISION_H