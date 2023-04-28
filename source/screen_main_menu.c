/* Includes */
#include <screen_main_menu.h>
#include <input_mapper.h>

/* Private state */


/* Function definitions */
static void screen_initialize(void)
{
}

static void screen_integrate
(
	double dt,
	const struct gameplay_dependencies_input * p_input,
  const struct gameplay_dependencies_batcher * p_batcher,
  const struct gameplay_dependencies_audio * p_audio,
	screen_callback_change_request_tf change_request
)
{
	/* Change screens */
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_LEFT_PADDLE_DOWN))
		change_request(SCREEN_TYPE_PONG);
}

static void screen_render(const struct gameplay_dependencies_batcher * p_batcher)
{
	p_batcher->color(255, 0, 0, 255);
	p_batcher->text("Screen - Main menu", 10, 590, 9 * 3);
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