/* Includes */
#include <audio_player.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>

/* Defines */
#define MAX_AUDIO_PATH_LENGTH (1024)

/* Private audio player state */
Mix_Chunk ** p_sound_effects;

/* Private helper functions */
static void audio_player_register_sound_effect
(
	enum audio_player_sfx_type sfx_type,
	const char * p_sound_effect_filename
)
{
	/* Get the absolute path for the sound effect file */
  static char absolute_sound_effect_path[MAX_AUDIO_PATH_LENGTH];
  snprintf(
    absolute_sound_effect_path,
    MAX_AUDIO_PATH_LENGTH,
    "%s%s/%s",
    SDL_GetBasePath(),
    "../resources/audio/effects",
    p_sound_effect_filename
  );

  /* Attempt to load the sound effect */
  p_sound_effects[sfx_type] = Mix_LoadWAV(absolute_sound_effect_path);
  if (p_sound_effects[sfx_type] == NULL)
  {
  	fprintf(
  		stderr, 
  		"\n[Audio player] Could not register sound effect: %s in directory: %s",
  		p_sound_effect_filename,
  		absolute_sound_effect_path
  	);
  }
}

/* Function definitions */
pong_bool_te audio_player_initialize(void)
{
	/* Initialize audio library */
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		fprintf(stderr, "\n[Audio player] Could not initialize SDL2 mixer - Error: %s", Mix_GetError());
		return PONG_FALSE;
	}

	/* Allocate a list for registering sounds */
	p_sound_effects = malloc(sizeof(Mix_Chunk *) * AUDIO_PLAYER_SFX_TYPE_COUNT);
	if (p_sound_effects == NULL)
	{
		fprintf(stderr, "\n[Audio player] Could not allocate storage for sound effect pointers");
		return PONG_FALSE;
	}

	/* Initialize sound effects pointers so we know which ones are registered at runtime */
	for (int sfx_index = 0; sfx_index < AUDIO_PLAYER_SFX_TYPE_COUNT; sfx_index++)
	{
		p_sound_effects[sfx_index] = NULL;
	}

	/* Register music and sound effects */
  audio_player_register_sound_effect(AUDIO_PLAYER_SFX_TYPE_PADDLE_HIT, "paddle_hit.wav");
  audio_player_register_sound_effect(AUDIO_PLAYER_SFX_TYPE_SCORE, "score.wav");

	/* Success */
	return PONG_TRUE;
}

pong_bool_te audio_player_play_sound_effect(enum audio_player_sfx_type sfx_type)
{
	if (sfx_type < 0 || sfx_type >= AUDIO_PLAYER_SFX_TYPE_COUNT)
		return PONG_FALSE;

	if (Mix_PlayChannel(-1, p_sound_effects[sfx_type], 0) < 0)
	{
		fprintf(
			stderr,
			"\n[Audio player] Could not player sound effect with id: %d - Error: %s",
			sfx_type,
			Mix_GetError()
		);
		return PONG_FALSE;
	}

	return PONG_TRUE;
}

void audio_player_cleanup(void)
{
	/* Cleanup loaded sound effect and music resources */
	for (int sfx_index = 0; sfx_index < AUDIO_PLAYER_SFX_TYPE_COUNT; sfx_index++)
	{
		free(p_sound_effects[sfx_index]);
		p_sound_effects[sfx_index] = NULL;
	}
	free(p_sound_effects);

	/* Cleanup subsystem */
	Mix_Quit();
}
