/* Includes */
#include <screen_main_menu.h>
#include <input_mapper.h>

/* Private state */


/* Function definitions */
static void screen_initialize
(
	const struct gameplay_dependencies_windowing * p_windowing
)
{
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
}

static void screen_render
(
	const struct gameplay_dependencies_batcher * p_batcher,
	const struct gameplay_dependencies_windowing * p_windowing
)
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