#ifndef SCREEN_H
#define SCREEN_H

/* Includes */
#include <gameplay_dependencies.h>

/* Screen types */
enum screen_type {
	SCREEN_TYPE_MAIN_MENU,
	SCREEN_TYPE_OPTIONS,
	SCREEN_TYPE_PONG,
	SCREEN_TYPE_QUIT, /* To signal quitting the screen state machine */
	SCREEN_TYPE_COUNT
};

/* Screen callback types */
typedef void (* screen_callback_change_request_tf)(enum screen_type requested_screen_type);
typedef void (* screen_callback_initialize_tf)(void);
typedef void (* screen_callback_integrate_tf)
(
	double dt,
	const struct gameplay_dependencies_input * p_input,
  const struct gameplay_dependencies_batcher * p_batcher,
  const struct gameplay_dependencies_audio * p_audio,
	screen_callback_change_request_tf change_request
);
typedef void (* screen_callback_render_tf)(const struct gameplay_dependencies_batcher * p_batcher);
typedef void (* screen_callback_cleanup_tf)(void);

/* Datatypes */
struct screen {
	enum screen_type type;
	screen_callback_initialize_tf p_initialize;
	screen_callback_integrate_tf p_integrate;
	screen_callback_render_tf p_render;
	screen_callback_cleanup_tf p_cleanup;
};

/* Function prototypes */
struct screen screen_make
(
	enum screen_type type,
	screen_callback_initialize_tf p_initialize,
	screen_callback_integrate_tf p_integrate,
	screen_callback_render_tf p_render,
	screen_callback_cleanup_tf p_cleanup
);

#endif