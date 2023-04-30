/* Includes */
#include <screen_options.h>
#include <input_mapper.h>
#include <vec2f.h>
#include <color4ub.h>
#include <audio_player.h>
#include <region2Di.h>

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
	/* Back to main menu */
	if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_RETURN))
	{
		p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_MENU_RETURN);
		change_request(SCREEN_TYPE_MAIN_MENU);
	}
}

static void screen_render
(
	const struct gameplay_dependencies_batcher * p_batcher,
	const struct gameplay_dependencies_windowing * p_windowing
)
{
}

static void screen_cleanup(void)
{
}

struct screen screen_options_make(void)
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