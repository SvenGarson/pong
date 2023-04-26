#ifndef SCREEN_H
#define SCREEN_H

/* Screen types */
enum screen_type {
	SCREEN_TYPE_MAIN_MENU,
	SCREEN_TYPE_OPTIONS,
	SCREEN_TYPE_PONG,
	SCREEN_TYPE_NONE
};

/* Screen callback types */
typedef void (* screen_callback_initialize_tf)(void);
typedef struct screen (* screen_callback_integrate_tf)(double dt);
typedef void (* screen_callback_render_tf)(void);
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