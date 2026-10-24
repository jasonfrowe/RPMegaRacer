#ifndef RACE_H
#define RACE_H

#include <stdbool.h>
#include <stdint.h>
#include "car.h"
#include "xram.h"

#define LAPS 5
#define PLAYER 0

/* DRS: a 2.5 second boost the player charges up while behind. */
#define DRS_FULL 240
#define DRS_TIME 150
#define DRS_ACCEL 72     /* extra engine power while it's open */
#define DRS_RANGE 6      /* racing line points (~48 px) to the car ahead */

typedef struct
{
    Car car;

    /* Progress */
    uint8_t wp;    /* nearest racing line point */
    int8_t lap;    /* times over the line: 1 on the first lap */
    uint8_t place; /* 1..NUM_CARS */
    bool finished;

    /* AI */
    int8_t lane;      /* px right of the racing line */
    int8_t lane_goal;
    int8_t lane_home;
    uint16_t skill;   /* engine power before rubber banding */
    int8_t pace;      /* corner speed trim, 1/256 of the hint */
    uint8_t stuck;    /* frames trying to move and not moving */
    uint8_t reverse;  /* frames left backing out of trouble */
    uint16_t idle;    /* frames since the last new racing line point */

    /* DRS (the player's) */
    uint8_t drs_charge;
    uint8_t drs_timer;
    bool drs_in_range;
} Racer;

typedef enum
{
    STATE_TITLE,
    STATE_COUNTDOWN,
    STATE_RACING,
    STATE_FINISHED,
} GameState;

extern Racer racers[NUM_CARS];
extern GameState game_state;
extern uint16_t state_timer;
extern uint8_t finish_order;  /* how many cars have finished */
extern uint8_t race_minutes, race_seconds, race_frames;
extern volatile bool autopilot;       /* the AI drives the player's car */
extern volatile bool autopilot_drs;   /* ...and uses DRS */
extern bool race_log;                 /* print laps to the console */
extern bool paused;

void race_setup(void);
void race_step(void);         /* one frame of a running race */
int16_t race_progress(const Racer *r);
void race_rescue(Racer *r);

/* AI, in ai.c */
void ai_init(Racer *r, uint8_t n);
void ai_drive(Racer *r);

#endif
