#ifndef SOUND_H
#define SOUND_H

// Waveform types
typedef enum {
    PSG_WAVE_SINE = 0,
    PSG_WAVE_SQUARE = 1,
    PSG_WAVE_SAWTOOTH = 2,
    PSG_WAVE_TRIANGLE = 3,
    PSG_WAVE_NOISE = 4
} PSGWaveform;

extern void init_psg(void);
extern void update_engine_sound(uint16_t velocity_mag);
extern void stop_engine_sound(void);


#endif // SOUND_H