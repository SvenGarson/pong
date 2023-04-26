/* Includes */
#include <screen_main_menu.h>
#include <screen_pong.h>
#include <stdio.h>

/* Function definitions */
static void screen_initialize(void)
{
	printf("\nMain init");
}

static struct screen screen_integrate(double dt)
{
	printf("\nMain integrate: %f", dt);

	return screen_pong_make();
}

static void screen_render(void)
{
	printf("\nMain render");
}

static void screen_cleanup(void)
{
	printf("\nMain cleanup");
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