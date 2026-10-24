#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

#define GAMEPAD_COUNT 4

/* Gamepad bits, per https://picocomputer.github.io/ria.html#gamepads */
#define GP_DPAD_UP 0x01
#define GP_DPAD_DOWN 0x02
#define GP_DPAD_LEFT 0x04
#define GP_DPAD_RIGHT 0x08
#define GP_CONNECTED 0x80

#define GP_LSTICK_UP 0x01
#define GP_LSTICK_DOWN 0x02
#define GP_LSTICK_LEFT 0x04
#define GP_LSTICK_RIGHT 0x08
#define GP_RSTICK_UP 0x10
#define GP_RSTICK_DOWN 0x20
#define GP_RSTICK_LEFT 0x40
#define GP_RSTICK_RIGHT 0x80

#define GP_BTN_A 0x01
#define GP_BTN_B 0x02
#define GP_BTN_C 0x04
#define GP_BTN_X 0x08
#define GP_BTN_Y 0x10
#define GP_BTN_Z 0x20
#define GP_BTN_L1 0x40
#define GP_BTN_R1 0x80

#define GP_BTN_L2 0x01
#define GP_BTN_R2 0x02
#define GP_BTN_SELECT 0x04
#define GP_BTN_START 0x08
#define GP_BTN_HOME 0x10

/* Which gamepad byte a mapping reads (ButtonMapping.gamepad_button). */
#define GP_FIELD_DPAD 0
#define GP_FIELD_STICKS 1
#define GP_FIELD_BTN0 2
#define GP_FIELD_BTN1 3

/* The order is shared with gamepad_mapper.c and JOYSTICK.DAT: don't reorder. */
typedef enum
{
    ACTION_THRUST,         /* up: accelerate */
    ACTION_REVERSE_THRUST, /* down: brake / reverse */
    ACTION_ROTATE_LEFT,
    ACTION_ROTATE_RIGHT,
    ACTION_FIRE,       /* gas */
    ACTION_SUPER_FIRE, /* brake / reverse */
    ACTION_ALT_FIRE,   /* DRS */
    ACTION_RESCUE,
    ACTION_PAUSE,
    ACTION_COUNT
} GameAction;

typedef struct
{
    uint8_t keyboard_key;
    uint8_t keyboard_key2;
    uint8_t gamepad_button; /* GP_FIELD_* */
    uint8_t gamepad_mask;
} ButtonMapping;

void input_init(void);
void input_update(void);
bool is_action_pressed(uint8_t player_id, GameAction action);
/* Down this frame and not the last. */
bool is_action_just_pressed(GameAction action);
bool key_pressed(uint8_t code);

/* What the driver asks of the car this frame. */
bool input_gas(void);
bool input_brake(void);
int8_t input_steer(void); /* -16..16, + is left */

#endif
