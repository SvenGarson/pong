#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

/* Defines */
#define AUDIO_PLAYER_MAX_SOUND_EFFECTS (3)
#define MAX_AUDIO_PATH_LENGTH (512)

/* Includes */
#include <pong_bool.h>
#include <SDL2/SDL_mixer.h>

/* Private audio player state */
int sound_effects_count = 0;
Mix_Chunk * sound_effects[AUDIO_PLAYER_MAX_SOUND_EFFECTS];

/* Interface function definitions */
pong_bool_te audio_player_initialize(void)
{
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		fprintf(stderr, "\n[SDL2 Mixer] Could not initialize SDL2 mixel - Error: %s", Mix_GetError());
		return PONG_FALSE;
	}

	return PONG_TRUE;
}

int audio_player_register_sound_effect(const char * p_sound_effect_filename)
{
	if (sound_effects_count >= AUDIO_PLAYER_MAX_SOUND_EFFECTS)
	{
		fprintf(stderr, "\n[Audio player] Reached sound effect capacity at index : %d", sound_effects_count);
		return -1;
	}

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
  Mix_Chunk * p_new_sound_effect = Mix_LoadWAV(absolute_sound_effect_path);
  if (p_new_sound_effect == NULL)
  {
  	fprintf(
  		stderr, 
  		"\n[Audio player] Could not resolve sound effect: %s in directory: %s",
  		p_sound_effect_filename,
  		absolute_sound_effect_path
  	);
  	return -1;
  }

  /* Register the sound effect and return a handle to it */
  const int new_sound_effect_id = sound_effects_count++;
  sound_effects[new_sound_effect_id] = p_new_sound_effect;
  return new_sound_effect_id;
}

int audio_player_play_sound_effect(int sound_effect_id)
{
	if (sound_effect_id == -1)
	{
		fprintf(stderr, "\n[Audio player] Cannot play sound effect from invalid sound effect id: %d", sound_effect_id);
	}

	if (Mix_PlayChannel(-1, sound_effects[sound_effect_id], 0) < 0)
	{
		fprintf(
			stderr,
			"\n[Audio player] Could not player sound effect with id: %d - Error: %s",
			sound_effect_id,
			Mix_GetError()
		);
		return 0;
	}

	return 1;
}

void audio_player_cleanup(void)
{
	/* Cleanup loaded sound effect and music resources */
	for (int sfx_index = 0; sfx_index < sound_effects_count; sfx_index++)
	{
		free(sound_effects[sfx_index]);
		sound_effects[sfx_index] = NULL;
	}
	sound_effects_count = 0;

	/* Cleanup subsystem */
	Mix_Quit();
}

#endif