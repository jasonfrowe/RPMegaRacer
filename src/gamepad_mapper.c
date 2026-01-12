/*
 * Gamepad Button Mapping Tool for RPMegaRacer
 * Walks through UP, DOWN, LEFT, RIGHT, A, B, X, Y, START
 */

#include <rp6502.h> 
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "input.h" // Ensure this defines GameAction enum

// Gamepad input structure
static gamepad_t gamepad[GAMEPAD_COUNT];

#define JOYSTICK_CONFIG_FILE "JOYSTICK.DAT"

// These labels describe what we are asking the user to press.
// The index here will correspond to the enum order in your game.
static const char* prompt_labels[] = {
    "D-PAD UP",
    "D-PAD DOWN", 
    "D-PAD LEFT",
    "D-PAD RIGHT",
    "BUTTON A Gas",
    "BUTTON B Reverse",
    "BUTTON X Nitro",
    "BUTTON Y Rescue",
    "START BUTTON",
    NULL 
};

// Map the walkthrough index above to your game's Action Enum
// Adjust these IDs to match your GameAction enum exactly!
static uint8_t action_map[] = {
    ACTION_THRUST,           // 1. UP   
    ACTION_REVERSE_THRUST,   // 2. DOWN
    ACTION_ROTATE_LEFT,      // 3. LEFT
    ACTION_ROTATE_RIGHT,     // 4. RIGHT
    ACTION_FIRE,             // 5. A
    ACTION_SUPER_FIRE,       // 6. B
    ACTION_ALT_FIRE,         // 7. X
    ACTION_RESCUE,           // 8. Y
    ACTION_PAUSE,            // 9. START
    ACTION_COUNT 
};

typedef struct {
    uint8_t action_id;  
    uint8_t field;      // 0=dpad, 1=sticks, 2=btn0, 3=btn1
    uint8_t mask;       
} JoystickMapping;

static JoystickMapping mappings[10];
static uint8_t num_mappings = 0;

// Helper to check for button transitions
static bool wait_for_any_button(uint8_t* field, uint8_t* mask) {
    uint8_t vsync_last = RIA.vsync;
    uint8_t prev_dpad = 0, prev_sticks = 0, prev_btn0 = 0, prev_btn1 = 0;
    
    while (true) {
        if (RIA.vsync == vsync_last) continue;
        vsync_last = RIA.vsync;
        
        RIA.addr0 = GAMEPAD_INPUT;
        RIA.step0 = 1;
        uint8_t d = RIA.rw0 & 0x0F;
        uint8_t s = RIA.rw0;
        uint8_t b0 = RIA.rw0;
        uint8_t b1 = RIA.rw0;
        
        // Detect transitions (pressed this frame, wasn't pressed last frame)
        if ((d & ~prev_dpad) != 0)  { *field = 0; *mask = d & ~prev_dpad;  return true; }
        if ((s & ~prev_sticks) != 0){ *field = 1; *mask = s & ~prev_sticks; return true; }
        if ((b0 & ~prev_btn0) != 0) { *field = 2; *mask = b0 & ~prev_btn0; return true; }
        if ((b1 & ~prev_btn1) != 0) { *field = 3; *mask = b1 & ~prev_btn1; return true; }
        
        prev_dpad = d; prev_sticks = s; prev_btn0 = b0; prev_btn1 = b1;
    }
}

int main(void) {
    printf("\f"); // Clear screen
    printf("=== RP6502 Gamepad Mapper ===\n\n");
    
    xregn(0, 0, 0, 1, KEYBOARD_INPUT);
    xregn(0, 0, 2, 1, GAMEPAD_INPUT);
    
    printf("Press any button to begin...\n");
    uint8_t f, m;
    wait_for_any_button(&f, &m);

    for (uint8_t i = 0; prompt_labels[i] != NULL; i++) {
        printf("PRESS: %s\n", prompt_labels[i]);
        
        wait_for_any_button(&f, &m);
        
        mappings[num_mappings].action_id = action_map[i];
        mappings[num_mappings].field = f;
        mappings[num_mappings].mask = m;
        num_mappings++;
        
        // Debounce: Wait for release
        while (true) {
            RIA.addr0 = GAMEPAD_INPUT;
            RIA.step0 = 1;
            uint8_t d = RIA.rw0 & 0x0F;
            uint8_t s = RIA.rw0;
            uint8_t b0 = RIA.rw0;
            uint8_t b1 = RIA.rw0;
            bool held = false;
            if (f == 0 && (d & m)) held = true;
            if (f == 1 && (s & m)) held = true;
            if (f == 2 && (b0 & m)) held = true;
            if (f == 3 && (b1 & m)) held = true;
            if (!held) break;
        }
    }
    
    // Save binary data
    FILE* fp = fopen(JOYSTICK_CONFIG_FILE, "wb");
    if (fp) {
        // Match the expected format: 1 byte for count, then mapping structs
        fputc(num_mappings, fp);
        fwrite(mappings, sizeof(JoystickMapping), num_mappings, fp);
        fclose(fp);
        printf("\nSaved to %s!\n", JOYSTICK_CONFIG_FILE);
    } else {
        printf("\nError: Save failed.\n");
    }

    printf("\nPress any key to exit.");
    return 0;
}