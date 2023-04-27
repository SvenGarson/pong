#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

/* Includes */
#include <pong_bool.h>

/* Interface function definitions */
pong_bool_te audio_player_initialize(void);
int audio_player_register_sound_effect(const char * p_sound_effect_filename);
int audio_player_play_sound_effect(int sound_effect_id);
void audio_player_cleanup(void);

#endif