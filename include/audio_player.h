#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

/* Includes */
#include <pong_bool.h>

/* Datatypes */
enum audio_player_sfx_type {
	AUDIO_PLAYER_SFX_TYPE_PADDLE_HIT,
	AUDIO_PLAYER_SFX_TYPE_SCORE,
	AUDIO_PLAYER_SFX_TYPE_COUNT
};

/* Interface function definitions */
pong_bool_te audio_player_initialize(void);
pong_bool_te audio_player_play_sound_effect(enum audio_player_sfx_type sfx_type);
void audio_player_cleanup(void);

#endif