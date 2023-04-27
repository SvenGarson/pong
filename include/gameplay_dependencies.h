#ifndef GAMEPLAY_DEPENDENCIES
#define GAMEPLAY_DEPENDENCIES

/* Includes */
#include <batcher.h>
#include <pong_bool.h>

/* Datatypes */
struct gameplay_dependencies_batcher {
	void (* color)(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
	void (* text)(const char * p_text, int base_x, int base_y, int font_height);
	void (* quadf)(float min_x, float min_y, float max_x, float max_y);
};

struct gameplay_dependencies_audio {
	/* Sound effect handles */
	int SFX_BALL_HIT;
	int SFX_SCORED;
	/* Interface functions */
	int (* play_sound_effect)(int sound_effect_id);
};

/*
		- key states
			+ none
			+ pressed
			+ held
			+ released

		- logic
			- key down
				+ [none, released] -> pressed
			- key up
				+ [pressed] -> released
*/
struct gameplay_dependencies_input {

};

#endif