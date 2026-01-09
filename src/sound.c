#include <stdint.h>
#include <rp6502.h>
#include "sound.h"
#include "constants.h"


void init_psg(void)
{
    // Enable PSG at XRAM address PSG_XRAM_ADDR
    xregn(0, 1, 0x00, 1, PSG_XRAM_ADDR);
    
    // Clear all 8 channels (64 bytes total)
    RIA.addr0 = PSG_XRAM_ADDR;
    RIA.step0 = 1;
    for (uint8_t i = 0; i < 64; i++) {
        RIA.rw0 = 0;
    }
}

/**
 * Stop sound on a channel
 */
static void stop_sound(uint8_t channel)
{
    if (channel > 7) return;
    
    uint16_t psg_addr = PSG_XRAM_ADDR + (channel * 8) + 6;  // pan_gate offset
    RIA.addr0 = psg_addr;
    RIA.rw0 = 0x00;  // Gate off (release)

    // Tiny delay: ~50–100 cycles ≈ 15–30 µs (safe, <1 audio sample)
    // Start small; increase if still unreliable
    volatile uint8_t delay = 40;  // Adjust 20–80 based on testing
    while (delay--) ;
}

#define ENG_CHAN 0
#define ENG_BASE_FREQ 40   // Lower base for a deep growl
#define ENG_BASE_VOL 4     // Lower is louder in your previous logic (1-15)

// Current engine pitch and phase
static uint8_t engine_vibration = 0;

void update_engine_sound(uint16_t velocity_mag) {
    uint16_t psg_addr = PSG_XRAM_ADDR + (ENG_CHAN * 8);
    
    // 1. Calculate RPM based on velocity
    // Velocity magnitude comes from our 8.8 physics
    uint16_t rpm = (velocity_mag >> 4); 

    // 2. Engine "Jitter" (Subtle vibration at idle)
    engine_vibration += 2; // Constant slow cycle
    int8_t jitter = (engine_vibration & 0x40) ? 1 : 0; 

    // 3. Calculate Pitch
    // We want a nice linear climb as the car accelerates
    uint16_t freq = ENG_BASE_FREQ + (rpm << 1) + jitter;
    uint16_t freq_val = freq * 3; // PSG scaling
    
    // 4. Dynamic Volume (Optional)
    // Make the engine slightly louder (smaller value) when moving fast
    uint8_t dynamic_vol = ENG_BASE_VOL;
    if (rpm > 20) dynamic_vol = 2; // Full roar
    if (rpm == 0) dynamic_vol = 6; // Quiet idle

    // --- WRITE TO RIA PSG REGISTERS ---
    // Note: We use ADDR0/RW0. Ensure this is called during VBlank
    // to avoid interfering with Sprite/Tile updates!
    
    RIA.addr0 = psg_addr;
    RIA.step0 = 1;
    
    // Freq Low/High
    RIA.rw0 = freq_val & 0xFF;
    RIA.rw0 = (freq_val >> 8) & 0xFF;
    
    // Duty Cycle (For Square/Sawtooth, 128 is 50%)
    RIA.rw0 = 128;
    
    // Vol/Attack (Attack 0 = Instant)
    RIA.rw0 = (dynamic_vol << 4) | 0;
    
    // Decay/Sustain (Keep it sustained)
    RIA.rw0 = (dynamic_vol << 4) | 0;
    
    // Waveform: 
    // PSG_WAVE_SAWTOOTH (2) is best for engines.
    // PSG_WAVE_SQUARE (1) is more "NES" style.
    RIA.rw0 = (PSG_WAVE_SAWTOOTH << 4) | 0;
    
    // Gate On
    RIA.rw0 = 0x01;
}

void stop_engine_sound(void) {
    stop_sound(ENG_CHAN);
}