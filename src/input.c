#include <fcntl.h>
#include <rp6502.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include "input.h"
#include "usb_hid_keys.h"
#include "xram.h"

static ButtonMapping button_mappings[ACTION_COUNT];
static gamepad_player_t pad[GAMEPAD_COUNT];
static uint8_t keystates[32];
static uint16_t held_now, held_last;

bool key_pressed(uint8_t code)
{
    return KEYBOARD_PRESSED(keystates, code);
}

static void map(GameAction a, uint8_t key, uint8_t key2, uint8_t field, uint8_t mask)
{
    button_mappings[a].keyboard_key = key;
    button_mappings[a].keyboard_key2 = key2;
    button_mappings[a].gamepad_button = field;
    button_mappings[a].gamepad_mask = mask;
}

static void reset_button_mappings(void)
{
    map(ACTION_THRUST, KEY_UP, KEY_W, GP_FIELD_DPAD, GP_DPAD_UP);
    map(ACTION_REVERSE_THRUST, KEY_DOWN, KEY_S, GP_FIELD_DPAD, GP_DPAD_DOWN);
    map(ACTION_ROTATE_LEFT, KEY_LEFT, KEY_A, GP_FIELD_DPAD, GP_DPAD_LEFT);
    map(ACTION_ROTATE_RIGHT, KEY_RIGHT, KEY_D, GP_FIELD_DPAD, GP_DPAD_RIGHT);
    map(ACTION_FIRE, KEY_SPACE, KEY_Z, GP_FIELD_BTN0, GP_BTN_A);
    map(ACTION_SUPER_FIRE, KEY_X, KEY_X, GP_FIELD_BTN0, GP_BTN_B);
    map(ACTION_ALT_FIRE, KEY_C, KEY_LEFTSHIFT, GP_FIELD_BTN0, GP_BTN_X);
    map(ACTION_RESCUE, KEY_R, KEY_R, GP_FIELD_BTN0, GP_BTN_Y);
    map(ACTION_PAUSE, KEY_ENTER, KEY_P, GP_FIELD_BTN1, GP_BTN_START);
}

/* JOYSTICK.DAT, written by gamepad_mapper: a count, then that many
 * {action, field, mask} triples. It overrides the gamepad half only. */
static void load_joystick_config(void)
{
    int fd = open("JOYSTICK.DAT", O_RDONLY);
    if (fd < 0)
        return;
    uint8_t n = 0;
    uint8_t m[3];
    if (read(fd, &n, 1) == 1) {
        while (n--) {
            if (read(fd, m, 3) != 3)
                break;
            if (m[0] < ACTION_COUNT && m[1] <= GP_FIELD_BTN1) {
                button_mappings[m[0]].gamepad_button = m[1];
                button_mappings[m[0]].gamepad_mask = m[2];
            }
        }
    }
    close(fd);
}

void input_init(void)
{
    xreg_ria_keyboard(XRAM_KEYBOARD);
    xreg_ria_gamepad(XRAM_GAMEPAD);
    reset_button_mappings();
    load_joystick_config();
}

void input_update(void)
{
    RIA.addr0 = XRAM_KEYBOARD;
    RIA.step0 = 1;
    for (uint8_t i = 0; i < 32; i++)
        keystates[i] = RIA.rw0;
    RIA.addr0 = XRAM_GAMEPAD;
    uint8_t *p = (uint8_t *)pad;
    for (uint8_t i = 0; i < sizeof(pad); i++)
        p[i] = RIA.rw0;

    held_last = held_now;
    held_now = 0;
    for (uint8_t a = 0; a < ACTION_COUNT; a++)
        if (is_action_pressed(0, (GameAction)a))
            held_now |= 1u << a;
}

#define AXIS_DEADZONE 32

bool is_action_pressed(uint8_t player_id, GameAction action)
{
    if (player_id >= GAMEPAD_COUNT || action >= ACTION_COUNT)
        return false;
    ButtonMapping *m = &button_mappings[action];
    if (player_id == 0 && (key_pressed(m->keyboard_key) || key_pressed(m->keyboard_key2)))
        return true;

    gamepad_player_t *g = &pad[player_id];
    if (!(g->dpad & GP_CONNECTED))
        return false;

    uint8_t v = 0;
    switch (m->gamepad_button) {
    case GP_FIELD_DPAD: v = g->dpad; break;
    case GP_FIELD_STICKS: v = g->sticks; break;
    case GP_FIELD_BTN0: v = g->btn0; break;
    case GP_FIELD_BTN1: v = g->btn1; break;
    }
    if (v & m->gamepad_mask)
        return true;

    /* Driving also takes the d-pad and either stick, whatever the mapping. */
    switch (action) {
    case ACTION_THRUST:
        return (g->dpad & GP_DPAD_UP) || (g->sticks & (GP_LSTICK_UP | GP_RSTICK_UP));
    case ACTION_REVERSE_THRUST:
        return (g->dpad & GP_DPAD_DOWN) || (g->sticks & (GP_LSTICK_DOWN | GP_RSTICK_DOWN));
    case ACTION_ROTATE_LEFT:
        return (g->dpad & GP_DPAD_LEFT) || (g->sticks & GP_RSTICK_LEFT) || g->lx < -AXIS_DEADZONE;
    case ACTION_ROTATE_RIGHT:
        return (g->dpad & GP_DPAD_RIGHT) || (g->sticks & GP_RSTICK_RIGHT) || g->lx > AXIS_DEADZONE;
    case ACTION_FIRE:
        return (g->btn1 & GP_BTN_R2) != 0;
    case ACTION_SUPER_FIRE:
        return (g->btn1 & GP_BTN_L2) != 0;
    default:
        return false;
    }
}

bool is_action_just_pressed(GameAction action)
{
    uint16_t bit = 1u << action;
    return (held_now & bit) && !(held_last & bit);
}

bool input_gas(void)
{
    return is_action_pressed(0, ACTION_FIRE) || is_action_pressed(0, ACTION_THRUST);
}

bool input_brake(void)
{
    return is_action_pressed(0, ACTION_SUPER_FIRE) || is_action_pressed(0, ACTION_REVERSE_THRUST);
}

int8_t input_steer(void)
{
    /* The left stick steers in proportion, on a curve that's gentle near
     * the middle; anything else is full lock. */
    gamepad_player_t *g = &pad[0];
    if ((g->dpad & GP_CONNECTED) && !(g->dpad & (GP_DPAD_LEFT | GP_DPAD_RIGHT)) &&
        (g->lx < -AXIS_DEADZONE || g->lx > AXIS_DEADZONE)) {
        int16_t x = g->lx;
        int16_t s = -(x * (x < 0 ? -x : x)) / 1000;
        if (s > 16) s = 16;
        if (s < -16) s = -16;
        return (int8_t)s;
    }
    int8_t s = 0;
    if (is_action_pressed(0, ACTION_ROTATE_LEFT))
        s += 16;
    if (is_action_pressed(0, ACTION_ROTATE_RIGHT))
        s -= 16;
    return s;
}
