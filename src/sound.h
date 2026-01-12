#ifndef SOUND_H
#define SOUND_H

extern void init_opl2_engine_sound(void);
extern void update_engine_sound(uint16_t velocity_mag);
extern void stop_engine_sound(void);


#endif // SOUND_H