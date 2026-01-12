#include <stdint.h>
#include "sound.h"
#include "opl.h"

// Channel 8 specific offsets
#define CH8_MOD_OFF 0x12
#define CH8_CAR_OFF 0x15
#define CH8_FREQ_LO 0xA8
#define CH8_FREQ_HI 0xB8
#define CH8_FEEDBACK 0xC8

void init_opl2_engine_sound(void) {
    // 1. Setup the "Grit" (Modulator - Op 18)
    opl_write(0x20 + CH8_MOD_OFF, 0x01); // Multiplier 1
    opl_write(0x40 + CH8_MOD_OFF, 0x2F); // Volume (some attenuation for grit)
    opl_write(0x60 + CH8_MOD_OFF, 0xF0); // Instant Attack
    opl_write(0x80 + CH8_MOD_OFF, 0xFF); // Max Sustain
    opl_write(0xE0 + CH8_MOD_OFF, 0x00); // Sine

    // 2. Setup the "Body" (Carrier - Op 21)
    opl_write(0x20 + CH8_CAR_OFF, 0x01); // Multiplier 1
    opl_write(0x40 + CH8_CAR_OFF, 0x00); // Max Volume
    opl_write(0x60 + CH8_CAR_OFF, 0xF0); // Instant Attack
    opl_write(0x80 + CH8_CAR_OFF, 0xFF); // Max Sustain
    opl_write(0xE0 + CH8_CAR_OFF, 0x00); // Sine

    // 3. Feedback (0x0E = Feedback 7, FM Connection)
    // Feedback 7 is what creates the "rough" engine growl.
    opl_write(CH8_FEEDBACK, 0x0E); 

    // 4. Initial Key On at Octave 1
    // 0x20 = Key On, 0x04 = Block 1
    opl_write(CH8_FREQ_HI, 0x24); 
}

void update_engine_sound(uint16_t velocity_mag) {
    // Normalize velocity (assuming 10.6 or 8.8) to a useful OPL F-Number
    // Low speed -> F-Number 150
    // High speed -> F-Number ~500
    uint16_t f_number = 150 + (velocity_mag << 1);

    if (f_number > 1023) f_number = 1023;

    // Write frequency
    opl_write(CH8_FREQ_LO, (uint8_t)(f_number & 0xFF));
    
    // Write Block/Key (Maintaining Octave 1 + Key-On)
    // We use a low octave (Block 1) to get that deep displacement hum
    uint8_t b8_val = 0x20 | (1 << 2) | (uint8_t)(f_number >> 8);
    opl_write(CH8_FREQ_HI, b8_val);
    
    // Important: Update your shadow register so opl_silence_all doesn't
    // get confused if it ever tries to read back Channel 8's state
    shadow_b0[8] = b8_val & 0x1F;
}


void stop_engine_sound(void) {
    // Clear the Key-On bit in 0xB8
    opl_write(0xB8, 0x00);
}