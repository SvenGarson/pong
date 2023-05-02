#ifndef GAMEPLAY_DEPENDENCIES
#define GAMEPLAY_DEPENDENCIES

/* Includes */
#include <pong_bool.h>
#include <batcher.h>
#include <audio_player.h>
#include <input_mapper.h>

/* Datatypes */
struct gameplay_dependencies_batcher {
	void (* color)(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
	void (* text)(const char * p_text, int base_x, int base_y, int font_height);
	pong_bool_te (* text_region)(const char * p_text,int base_x,int base_y, int font_height, struct region2Di * p_out_region);
	void (* quadf)(float min_x, float min_y, float max_x, float max_y);
};

struct gameplay_dependencies_audio {
	pong_bool_te (* play_sound_effect)(enum audio_player_sfx_type sfx_type);
};

struct gameplay_dependencies_input {
	pong_bool_te (* key_none)(enum input_mapper_key_type custom_key_type);
	pong_bool_te (* key_pressed)(enum input_mapper_key_type custom_key_type);
	pong_bool_te (* key_held)(enum input_mapper_key_type custom_key_type);
	pong_bool_te (* key_released)(enum input_mapper_key_type custom_key_type);
};

struct gameplay_dependencies_windowing {
	int window_width;
	int window_height;
	void (* hook_close_window)(void);
	pong_bool_te (* hook_window_is_fullscreen)(void);
	void (* hook_window_disable_fullscreen)(void);
	pong_bool_te (* hook_window_set_fullscreen)(void);
	pong_bool_te (* hook_window_set_desktop_display_mode)(void);
	void (* hook_window_set_display_mode)(const SDL_DisplayMode * p_desired_display_mode);
	int (* hook_window_number_of_display_modes)(void);
};

#endif