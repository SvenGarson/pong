/* Includes */
#include <input_mapper.h>
#include <stdlib.h>

/* Private helper functions */
static pong_bool_te custom_key_type_valid(enum input_mapper_key_type custom_key_type)
{
	return custom_key_type >= 0 && custom_key_type < INPUT_MAPPER_KEY_TYPE_COUNT;
}

static void register_usable_mapping
(
	struct input_mapper_key_mapping * p_mappings,
	enum input_mapper_key_type custom_key_type,
	int scancode_to_associate
)
{
	if (p_mappings == NULL)
		return;

	if (!custom_key_type_valid(custom_key_type))
	{
		fprintf(stderr, "\n[Input mapper] Could not register invalid custom key type %d", custom_key_type);
		return;
	}

	/* Custom key type is valid */
	struct input_mapper_key_mapping * const p_updated_mapping = p_mappings + custom_key_type;
	p_updated_mapping->custom_key_type = custom_key_type;
	p_updated_mapping->associated_scancode = scancode_to_associate;
	p_updated_mapping->usable = PONG_TRUE;
}

static pong_bool_te input_mapper_custom_key_in_state
(
	struct input_mapper_instance * p_instance,
	enum input_mapper_key_type custom_key_type,
	enum input_mapper_key_state requested_state
)
{
	/* Something wrong with the instance */
	if (p_instance == NULL || p_instance->p_mappings == NULL || p_instance->p_states == NULL)
		return PONG_FALSE;

	/* Ignore invalid custom key types */
	if (!custom_key_type_valid(custom_key_type))
		return PONG_FALSE;

	return (p_instance->p_states[custom_key_type] == requested_state) ? PONG_TRUE : PONG_FALSE;
}

/* Function definitions */
pong_bool_te input_mapper_create(struct input_mapper_instance * p_out_instance)
{
	/* Build contextual to SDL2 scancode key relationship */
	p_out_instance->p_mappings = malloc(sizeof(struct input_mapper_key_mapping) * INPUT_MAPPER_KEY_TYPE_COUNT);
	if (p_out_instance->p_mappings == NULL)
	{
		fprintf(stderr, "\n[Input mapper] Could not allocate input key mappings");
		return PONG_FALSE;
	}
	
	/* Build a list of all possible mappings and mark them as un-usable */
	for (int mapping_index = 0; mapping_index < INPUT_MAPPER_KEY_TYPE_COUNT; mapping_index++)
	{
		struct input_mapper_key_mapping * const p_initial_mapping = p_out_instance->p_mappings + mapping_index;
		p_initial_mapping->custom_key_type = mapping_index;
		p_initial_mapping->associated_scancode = - 1;
		p_initial_mapping->usable = PONG_FALSE;
	}

	/* Register the key mappings to update the state for and mark then as usable */
	register_usable_mapping(p_out_instance->p_mappings, INPUT_MAPPER_KEY_TYPE_LEFT_PADDLE_UP, SDL_SCANCODE_LSHIFT);
	register_usable_mapping(p_out_instance->p_mappings, INPUT_MAPPER_KEY_TYPE_LEFT_PADDLE_DOWN, SDL_SCANCODE_LCTRL);
	register_usable_mapping(p_out_instance->p_mappings, INPUT_MAPPER_KEY_TYPE_RIGHT_PADDLE_UP, SDL_SCANCODE_RSHIFT);
	register_usable_mapping(p_out_instance->p_mappings, INPUT_MAPPER_KEY_TYPE_RIGHT_PADDLE_DOWN, SDL_SCANCODE_RCTRL);
	register_usable_mapping(p_out_instance->p_mappings, INPUT_MAPPER_KEY_TYPE_QUIT_APPLICATION, SDL_SCANCODE_ESCAPE);

	/* Build a list of custom key input states */
	p_out_instance->p_states = malloc(sizeof(enum input_mapper_key_state) * INPUT_MAPPER_KEY_TYPE_COUNT);
	if (p_out_instance->p_states == NULL)
	{
		input_mapper_destroy(p_out_instance);
		return PONG_FALSE;
	}

	/* Initialize all custom key states */
	for (int custom_key_type = 0; custom_key_type < INPUT_MAPPER_KEY_TYPE_COUNT; custom_key_type++)
	{
		p_out_instance->p_states[custom_key_type] = INPUT_MAPPER_KEY_STATE_NONE;
	}

	/* Success */
	return PONG_TRUE;
}

void input_mapper_destroy(const struct input_mapper_instance * p_instance)
{
	free(p_instance->p_states);
	free(p_instance->p_mappings);
}

void input_mapper_set_intermediate_state
(
	struct input_mapper_instance * p_instance,
	const uint8_t * p_keyboard_state
)
{
	for (int mapping_index = 0; mapping_index < INPUT_MAPPER_KEY_TYPE_COUNT; mapping_index++)
	{
		/* Ignore un-usable custom keys */
		const struct input_mapper_key_mapping * const p_current_mapping = p_instance->p_mappings + mapping_index;
		if (!p_current_mapping->usable)
			continue;

		/* Udpate the state of the usable custom key */
		const uint8_t custom_key_pressed = p_keyboard_state[p_current_mapping->associated_scancode];
		enum input_mapper_key_state * p_current_custom_key_state = p_instance->p_states + p_current_mapping->custom_key_type;

		if (custom_key_pressed)
    {
      /* Pressed */
      if (*p_current_custom_key_state == INPUT_MAPPER_KEY_STATE_NONE)
      {
        *p_current_custom_key_state = INPUT_MAPPER_KEY_STATE_PRESSED;
      }
      else if (*p_current_custom_key_state == INPUT_MAPPER_KEY_STATE_PRESSED)
      {
        *p_current_custom_key_state = INPUT_MAPPER_KEY_STATE_HELD;
      }
    }
    else
    {
      /* Released */
      if (
      	*p_current_custom_key_state == INPUT_MAPPER_KEY_STATE_PRESSED ||
      	*p_current_custom_key_state == INPUT_MAPPER_KEY_STATE_HELD
      )
      {
        *p_current_custom_key_state = INPUT_MAPPER_KEY_STATE_RELEASED;
      }
      else if (*p_current_custom_key_state == INPUT_MAPPER_KEY_STATE_RELEASED)
      {
				*p_current_custom_key_state = INPUT_MAPPER_KEY_STATE_NONE;
      }
    }
	}
}

pong_bool_te input_mapper_custom_key_state_none
(
	struct input_mapper_instance * p_instance,
	enum input_mapper_key_type custom_key_type
)
{
	return input_mapper_custom_key_in_state(p_instance, custom_key_type, INPUT_MAPPER_KEY_STATE_NONE);
}

pong_bool_te input_mapper_custom_key_state_pressed
(
	struct input_mapper_instance * p_instance,
	enum input_mapper_key_type custom_key_type
)
{
return input_mapper_custom_key_in_state(p_instance, custom_key_type, INPUT_MAPPER_KEY_STATE_PRESSED);
}

pong_bool_te input_mapper_custom_key_state_held
(
	struct input_mapper_instance * p_instance,
	enum input_mapper_key_type custom_key_type
)
{
return input_mapper_custom_key_in_state(p_instance, custom_key_type, INPUT_MAPPER_KEY_STATE_HELD);
}

pong_bool_te input_mapper_custom_key_state_released
(
	struct input_mapper_instance * p_instance,
	enum input_mapper_key_type custom_key_type
)
{
return input_mapper_custom_key_in_state(p_instance, custom_key_type, INPUT_MAPPER_KEY_STATE_RELEASED);
}