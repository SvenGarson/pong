/* Includes */
#include <screen_main_menu.h>
#include <input_mapper.h>
#include <vec2f.h>
#include <color4ub.h>
#include <audio_player.h>
#include <region2Di.h>

/* Datatypes */
enum menu_item_type {
	MENU_ITEM_TYPE_PLAY,
	MENU_ITEM_TYPE_OPTIONS,
	MENU_ITEM_TYPE_EXIT_TO_DESKTOP
};

struct menu_item {
	const char * p_text;
	enum menu_item_type type;
};

struct menu {
	int selection_index;
	struct vec2f base_position;
	int font_height;
	struct color4ub color_foreground;
	struct color4ub color_background;
	enum audio_player_sfx_type sfx_type;
	const struct menu_item * p_items;
};

/* Constants */
const char * GAME_TITLE = "Pong";
const char * WRITTEN_BY = "Written by: Sven Garson";

/* Private state */
static const struct menu_item main_menu_items[] = {
	{ "Play", MENU_ITEM_TYPE_PLAY },
	{ "Options", MENU_ITEM_TYPE_OPTIONS },
	{ "Exit to desktop", MENU_ITEM_TYPE_EXIT_TO_DESKTOP }
};
static struct menu main_menu;

/* Private helper functions */
static int number_of_menu_items(void)
{
	return (int)(sizeof(main_menu_items) / sizeof(main_menu_items[0]));
}

static void set_selection_index(int direction)
{
	const int selection_increment = (direction >= 0) ? 1 : -1;
	const int next_selection_index = main_menu.selection_index + selection_increment;

	/* Apply and cap */
	const int menu_item_count = number_of_menu_items();
	if (next_selection_index < 0)
		main_menu.selection_index = menu_item_count - 1;
	else if (next_selection_index >= menu_item_count)
		main_menu.selection_index = 0;
	else
		main_menu.selection_index = next_selection_index;
}

/* Function definitions */
static void screen_initialize
(
	const struct gameplay_dependencies_windowing * p_windowing
)
{
	/* Build the menu */
	main_menu.selection_index = 0;
	main_menu.base_position = (struct vec2f){ 20, 100 };
	main_menu.font_height = 9 * 3;
	main_menu.color_foreground = (struct color4ub){ 255, 255, 255, 255 };
	main_menu.color_background = (struct color4ub){ 100, 150, 100, 255 };
	main_menu.sfx_type = AUDIO_PLAYER_SFX_TYPE_PADDLE_HIT;
	main_menu.p_items = main_menu_items;
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
	/* Menu item selection */
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_UP))
	{
		p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_MENU_SELECT);
		set_selection_index(-1);
	}
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_DOWN))
	{
		p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_MENU_SELECT);
		set_selection_index(1);
	}

	/* Trigger selection */
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_SELECT))
	{
		/* Sound */
		p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_MENU_CHOOSE);

		/* Action */
		const enum menu_item_type selected_item_type = main_menu.p_items[main_menu.selection_index].type;
		switch(selected_item_type)
		{
			case MENU_ITEM_TYPE_PLAY:
				change_request(SCREEN_TYPE_PONG);
				break;
			case MENU_ITEM_TYPE_OPTIONS:
				change_request(SCREEN_TYPE_OPTIONS);
				break;
			case MENU_ITEM_TYPE_EXIT_TO_DESKTOP:
				p_windowing->hook_close_window();
				break;
		}
	}
}

static void screen_render
(
	const struct gameplay_dependencies_batcher * p_batcher,
	const struct gameplay_dependencies_windowing * p_windowing
)
{
	/* Render menu items */
	struct vec2f menu_item_cursor = main_menu.base_position;
	for (int item_index = 0; item_index < sizeof(main_menu_items) / sizeof(struct menu_item); item_index++)
	{
		const struct menu_item * const p_menu_item = main_menu.p_items + item_index;

		/* Render active item background region when selected */
		struct region2Di background_region;
		if (
			main_menu.selection_index == item_index &&
			p_batcher->text_region(p_menu_item->p_text, menu_item_cursor.x, menu_item_cursor.y, main_menu.font_height, &background_region)
		)
		{

			p_batcher->color(main_menu.color_background.red, main_menu.color_background.green, main_menu.color_background.blue, main_menu.color_background.alpha);
			p_batcher->quadf(background_region.min.x, background_region.min.y, background_region.max.x, background_region.max.y);
		}

		/* Render item text */
		p_batcher->color(main_menu.color_foreground.red, main_menu.color_foreground.green, main_menu.color_foreground.blue, main_menu.color_foreground.alpha);
		p_batcher->text(p_menu_item->p_text, menu_item_cursor.x, menu_item_cursor.y, main_menu.font_height);

		/* Move the cursor to the next menu item in line */
		menu_item_cursor.y -= main_menu.font_height;
	}

	/* Information */
	p_batcher->color(255, 255, 255, 255);
	p_batcher->text(GAME_TITLE, p_windowing->window_width * 0.35f, p_windowing->window_height * 0.7f, 90);
	p_batcher->color(150, 150, 150, 255);
	p_batcher->text(WRITTEN_BY, 10, p_windowing->window_height - 10, 9);
}

static void screen_cleanup(void)
{
}

struct screen screen_main_menu_make(void)
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