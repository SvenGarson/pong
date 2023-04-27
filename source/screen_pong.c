/* Includes */
#include <screen_pong.h>
#include <screen_main_menu.h>
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
}

static void screen_cleanup(void)
{
}

struct screen screen_pong_make(void)
{
	return screen_make
	(
		SCREEN_TYPE_PONG,
		screen_initialize,
		screen_integrate,
		screen_render,
		screen_cleanup
	);
}