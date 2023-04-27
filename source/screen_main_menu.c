/* Includes */
#include <screen_main_menu.h>
#include <screen_pong.h>
#include <stdio.h>

/* Function definitions */
static void screen_initialize(void)
{
}

static void screen_integrate
(
	double dt,
	screen_callback_change_request_tf change_request
)
{
}

static void screen_render(void)
{
	batcher_text("Screen - Main menu", 10, 590, 9 * 3);
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