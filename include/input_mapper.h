#ifndef INPUT_MAPPER_H
#define INPUT_MAPPER_H

/* Includes */
#include <SDL2/SDL.h>
#include <pong_bool.h>

/* Datatypes */
enum input_mapper_key_type {
	INPUT_MAPPER_KEY_TYPE_LEFT_PADDLE_UP,
	INPUT_MAPPER_KEY_TYPE_LEFT_PADDLE_DOWN,
	INPUT_MAPPER_KEY_TYPE_RIGHT_PADDLE_UP,
	INPUT_MAPPER_KEY_TYPE_RIGHT_PADDLE_DOWN,
	INPUT_MAPPER_KEY_TYPE_QUIT_APPLICATION,
	INPUT_MAPPER_KEY_TYPE_MENU_SELECT,
	INPUT_MAPPER_KEY_TYPE_MENU_RETURN,
	INPUT_MAPPER_KEY_TYPE_MENU_UP,
	INPUT_MAPPER_KEY_TYPE_MENU_DOWN,
	INPUT_MAPPER_KEY_TYPE_MENU_LEFT,
	INPUT_MAPPER_KEY_TYPE_MENU_RIGHT,
	INPUT_MAPPER_KEY_TYPE_COUNT
};

enum input_mapper_key_state {
	INPUT_MAPPER_KEY_STATE_NONE,
	INPUT_MAPPER_KEY_STATE_PRESSED,
	INPUT_MAPPER_KEY_STATE_HELD,
	INPUT_MAPPER_KEY_STATE_RELEASED
};

struct input_mapper_key_mapping {
	enum input_mapper_key_type custom_key_type;
	SDL_Scancode associated_scancode;
	pong_bool_te usable;
};

struct input_mapper_instance {
	struct input_mapper_key_mapping * p_mappings;
	enum input_mapper_key_state * p_states;
};

/* Function prototypes */
pong_bool_te input_mapper_create(struct input_mapper_instance * p_out_instance);
void input_mapper_destroy(const struct input_mapper_instance * p_instance);
void input_mapper_set_intermediate_state
(
	struct input_mapper_instance * p_instance,
	const uint8_t * p_keyboard_state
);
pong_bool_te input_mapper_custom_key_state_none
(
	struct input_mapper_instance * p_instance,
	enum input_mapper_key_type custom_key_type
);
pong_bool_te input_mapper_custom_key_state_pressed
(
	struct input_mapper_instance * p_instance,
	enum input_mapper_key_type custom_key_type
);
pong_bool_te input_mapper_custom_key_state_held
(
	struct input_mapper_instance * p_instance,
	enum input_mapper_key_type custom_key_type
);
pong_bool_te input_mapper_custom_key_state_released
(
	struct input_mapper_instance * p_instance,
	enum input_mapper_key_type custom_key_type
);

#endif