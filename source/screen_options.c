/* Includes */
#include <screen_options.h>
#include <input_mapper.h>
#include <vec2f.h>
#include <vec2i.h>
#include <color4ub.h>
#include <audio_player.h>
#include <region2Di.h>
#include <pong_bool.h>
#include <stdlib.h>

/* Defines */
#define MAX_OPTIONS_COUNT (16)
#define MAX_OPTIONS_VALUE_LENGTH (128)
#define MAX_OPTIONS_DESCRIPTION_LENGTH (MAX_OPTIONS_VALUE_LENGTH * 2)

/* Data types */
typedef void (* callback_current_value_tf)(char * p_out_description, size_t description_max_length);
typedef void (* callback_select_sub_option_tf)(int direction);

struct options_menu_item {
	const char * p_name;
	callback_current_value_tf cb_current_value;
	callback_select_sub_option_tf cb_selection_sub_option;
};

struct options_menu {
	struct vec2i base_position;
	int font_height;
	int selected_item_index;
	struct color4ub color_active;
	struct color4ub color_in_active;
	struct options_menu_item items[MAX_OPTIONS_COUNT];
};

/* Private state */
	/* Dependencies */
	static const struct gameplay_dependencies_windowing * p_deps_windowing;
	/* General */
	static struct options_menu menu_options;
	static int options_used = 0;
	/* Fullscreen */
	static pong_bool_te option_fullscreen_enabled;
	/* Display modes */
	static int number_of_display_modes = 0;
	static SDL_DisplayMode * p_available_display_modes;
	static int selected_display_mode_index = 0;
	/* Confirm and cancel */
	static pong_bool_te options_have_changed = PONG_FALSE;

/* Private helper functions */
static void increment_selected_item_index_in_direction(int direction)
{
	if (direction == 0)
		return;

	/* Cap to length of one */
	if (direction > 1)
		direction = 1;
	if (direction < -1)
		direction = -1;

	/* Increment and wrap */
	const int next_selection_index = menu_options.selected_item_index + direction;
	if (next_selection_index >= options_used)
		menu_options.selected_item_index = 0;
	else if (next_selection_index < 0)
		menu_options.selected_item_index = options_used - 1;
	else
		menu_options.selected_item_index = next_selection_index;
}

static void apply_chosen_display_mode(void)
{
	const SDL_DisplayMode * const p_selected_display_mode = p_available_display_modes + selected_display_mode_index;

	/* Exit fullscreen mode */
	p_deps_windowing->hook_window_disable_fullscreen();

	/* Set display mode */

}

/* Item callbacks - Fullscreen */
static void callback_fullscreen_current_value(char * p_out_description, size_t description_max_length)
{
	snprintf(p_out_description, description_max_length, option_fullscreen_enabled ? "On" : "Off");
}

static void callback_fullscreen_select_sub_option(int direction)
{
	if (direction == 0)
		return;

	/* Set window to fullscreen only when windowed */
	if (!p_deps_windowing->hook_window_is_fullscreen())
	{
		option_fullscreen_enabled = p_deps_windowing->hook_window_set_fullscreen();
	}
	else
	{
		/* Switch to desktop display mode */
		p_deps_windowing->hook_window_set_desktop_display_mode();
	}

	/* Opdate option */
	option_fullscreen_enabled = p_deps_windowing->hook_window_is_fullscreen();
}

/* Item callbacks - Display mode */
static void callback_display_mode_current_value(char * p_out_description, size_t description_max_length)
{
	/* Display the current display mode in text form */
	const SDL_DisplayMode * const p_selected_display_mode = p_available_display_modes + selected_display_mode_index;
	snprintf(
		p_out_description,
		description_max_length,
		"%-4d X %-4d @ %d",
		p_selected_display_mode->w,
		p_selected_display_mode->h,
		p_selected_display_mode->refresh_rate
	);
}

static void callback_display_mode_select_sub_option(int direction)
{
	if(direction == 0)
		return;

	/* Options can now be applied */
	options_have_changed = PONG_TRUE;

	/* Adjust the display mode selection index */
	if (direction > 1)
		direction = 1;
	if (direction < 1)
		direction = -1;

	const int next_selected_display_mode_index = selected_display_mode_index + direction;
	if (next_selected_display_mode_index < 0)
		selected_display_mode_index = number_of_display_modes - 1;
	else if (next_selected_display_mode_index >= number_of_display_modes)
		selected_display_mode_index = 0;
	else
		selected_display_mode_index = next_selected_display_mode_index;
}

/* Function definitions */
static void screen_initialize
(
	const struct gameplay_dependencies_windowing * p_windowing
)
{
	/* Dependencies */
	p_deps_windowing = p_windowing;

	/* Reset things */
	options_used = 0;
	options_have_changed = PONG_FALSE;
	selected_display_mode_index = 0;

	/* Initialize options from previously configured things */
	option_fullscreen_enabled = p_windowing->hook_window_is_fullscreen();

	/* Setup menu items */
	struct options_menu_item item_fullscreen;
	item_fullscreen.p_name = "Fullscreen";
	item_fullscreen.cb_current_value = callback_fullscreen_current_value;
	item_fullscreen.cb_selection_sub_option = callback_fullscreen_select_sub_option;

	struct options_menu_item item_display_mode;
	item_display_mode.p_name = "Display mode";
	item_display_mode.cb_current_value = callback_display_mode_current_value;
	item_display_mode.cb_selection_sub_option = callback_display_mode_select_sub_option;

	/* Setup the menu */
	menu_options.base_position = (struct vec2i){ p_windowing->window_width * 0.2f, p_windowing->window_height * 0.7f };
	menu_options.font_height = 9 * 2;
	menu_options.selected_item_index = 0;
	menu_options.color_active = (struct color4ub){ 255, 255, 255, 255 };
	menu_options.color_in_active = (struct color4ub){ 125, 125, 125, 255 };

	/* Add items to the menu */
	menu_options.items[options_used++] = item_fullscreen;
	menu_options.items[options_used++] = item_display_mode;

	/* Load all display modes and determine the current one */
	number_of_display_modes = p_windowing->hook_window_number_of_display_modes();
	p_available_display_modes = malloc(sizeof(SDL_DisplayMode) * number_of_display_modes);
	if (p_available_display_modes == NULL)
	{
		fprintf(stderr, "\n[Screen options] Could not allocated memory for available display modes");
	}

	for (int display_mode_index = 0; display_mode_index < number_of_display_modes; display_mode_index++)
	{
		if (SDL_GetDisplayMode(0, display_mode_index, p_available_display_modes + display_mode_index) != 0)
		{
			fprintf(
				stderr,
				"\n[Screen options] Could not access display mode at index %d - Error: %s",
				display_mode_index,
				SDL_GetError()
			);
			continue;
		}
	}
}

static void screen_integrate
(
	double dt,
	const struct gameplay_dependencies_input * p_input,
  const struct gameplay_dependencies_batcher * p_batcher,
  const struct gameplay_dependencies_audio * p_audio,
  const struct gameplay_dependencies_windowing * p_windowing,
	screen_callback_change_request_tf change_request
)
{
	/* Back to main menu - Use a menu entry point for this */
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_RETURN))
	{
		p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_MENU_RETURN);
		change_request(SCREEN_TYPE_MAIN_MENU);
	}

	/* Select option */
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_UP))
	{
		p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_MENU_SELECT);
		increment_selected_item_index_in_direction(1);
	}
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_DOWN))
	{
		p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_MENU_SELECT);
		increment_selected_item_index_in_direction(-1);
	}

	/* Select sub-option */
	const struct options_menu_item * const p_active_option = menu_options.items + menu_options.selected_item_index;
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_LEFT))
	{
		p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_PADDLE_HIT);
		p_active_option->cb_selection_sub_option(-1);
	}
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_RIGHT))
	{
		p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_PADDLE_HIT);
		p_active_option->cb_selection_sub_option(1);
	}

	/* Apply options */
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_SELECT) && options_have_changed)
	{
		/* Apply the selected display mode */
		const SDL_DisplayMode * const p_selected_display_mode = p_available_display_modes + selected_display_mode_index;
		p_deps_windowing->hook_window_set_display_mode(p_selected_display_mode);

		/* Options applied */
		p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_MENU_CHOOSE);
		options_have_changed = PONG_FALSE;
	}
}

static void screen_render
(
	const struct gameplay_dependencies_batcher * p_batcher,
	const struct gameplay_dependencies_windowing * p_windowing
)
{
	/* Render the options menu with currently selected values */
	struct vec2i menu_item_cursor = menu_options.base_position;
	static char option_description_buffer[MAX_OPTIONS_DESCRIPTION_LENGTH];
	static char option_value_buffer[MAX_OPTIONS_VALUE_LENGTH];
	for (int option_index = 0; option_index < options_used; option_index++)
	{
		const struct options_menu_item * const p_item = menu_options.items + option_index;

		/* Highlight the selected option */
		if (option_index == menu_options.selected_item_index)
		{
			p_batcher->color(menu_options.color_active.red, menu_options.color_active.green, menu_options.color_active.blue, menu_options.color_active.alpha);
		}
		else
		{
			p_batcher->color(menu_options.color_in_active.red, menu_options.color_in_active.green, menu_options.color_in_active.blue, menu_options.color_in_active.alpha);
		}

		/* Render the option with the currently selected value as string */
		p_item->cb_current_value(option_value_buffer, MAX_OPTIONS_VALUE_LENGTH);
		snprintf(option_description_buffer, MAX_OPTIONS_DESCRIPTION_LENGTH, "%-16s: < %s >", p_item->p_name, option_value_buffer);
		p_batcher->text(option_description_buffer, menu_item_cursor.x, menu_item_cursor.y - (option_index * menu_options.font_height), menu_options.font_height);
	}

	/* Confirm or discard options */
	if (options_have_changed)
	{
		/* Confirm settings */
		p_batcher->color(150, 150, 255, 255);
		p_batcher->text("Enter to apply selected options", 10, 10 + 9 * 3, 9 * 3);
	}

	p_batcher->color(150, 150, 150, 255);
	p_batcher->text("Escape to return to main menu", 10, p_windowing->window_height - 10, 9 * 3);
}

static void screen_cleanup(void)
{
	free(p_available_display_modes);
}

struct screen screen_options_make(void)
{
	return screen_make
	(
		SCREEN_TYPE_MAIN_MENU,
		screen_initialize,
		screen_integrate,
		screen_render,
		screen_cleanup
	);
}
