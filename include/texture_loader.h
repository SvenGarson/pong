#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

/* Includes */
#include <pong_bool.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>

/* Defines */
#define MAX_PATH_LENGTH (512)

/* Function definitions - TODO-GS: Split into headers and source files */
pong_bool_te texture_loader_initialize(void)
{
  /* TODO-GS: Make sure the loader can be initialized but once */
  const int required_image_flags = IMG_INIT_PNG;
  if (!(IMG_Init(required_image_flags) & required_image_flags))
  {
    fprintf(stderr, "\n[Texture loader] %s\n", IMG_GetError());
    return PONG_FALSE;
  }

  return PONG_TRUE;
}

SDL_Surface * texture_loader_load_texture
(
  const char * p_texture_name_with_extension
)
{
  if (p_texture_name_with_extension == NULL)
    return NULL;

  /* Determine the absolute path to the texture */
  static char absolute_texture_path[MAX_PATH_LENGTH];
  snprintf(
    absolute_texture_path,
    MAX_PATH_LENGTH,
    "%s%s/%s",
    SDL_GetBasePath(),
    "../resources/images",
    p_texture_name_with_extension
  );

  /* Attempt to load the texture as surface */
  SDL_Surface * p_loaded_texture = IMG_Load(absolute_texture_path);
  if (p_loaded_texture == NULL)
  {
    fprintf(stderr, "\n[SDL_Image] Could not load image '%s' - Error: %s\n", absolute_texture_path, IMG_GetError());
  }

  /* Success */
  return p_loaded_texture;
}

void texture_loader_destroy_texture(SDL_Surface * p_texture)
{
  SDL_FreeSurface(p_texture);
}

void texture_loader_cleanup(void)
{
  IMG_Quit();
}

#endif