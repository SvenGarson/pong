/* Includes */
#include <screen_pong.h>
#include <screen_main_menu.h>
#include <stdio.h>

/* Function definitions */
static void screen_initialize(void)
{
	printf("\nPong init");
}

static struct screen screen_integrate(double dt)
{
	printf("\nPong integrate: %f", dt);

	return screen_main_menu_make();
}

static void screen_render(void)
{
	printf("\nPong render");
}

static void screen_cleanup(void)
{
	printf("\nPong cleanup");
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