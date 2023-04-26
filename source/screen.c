/* Includes */
#include <screen.h>

/* Function definitions */
struct screen screen_make
(
	enum screen_type type,
	screen_callback_initialize_tf p_initialize,
	screen_callback_integrate_tf p_integrate,
	screen_callback_render_tf p_render,
	screen_callback_cleanup_tf p_cleanup
)
{
	struct screen new_screen;

	new_screen.type = type;
	new_screen.p_initialize = p_initialize;
	new_screen.p_integrate = p_integrate;
	new_screen.p_render = p_render;
	new_screen.p_cleanup = p_cleanup;

	return new_screen;
}
