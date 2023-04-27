#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

/* Includes */
#include <pong_bool.h>
#include <SDL2/SDL.h>

/* Function prototypes */
pong_bool_te texture_loader_initialize(void);
SDL_Surface * texture_loader_load_texture
(
  const char * p_texture_name_with_extension
);
void texture_loader_destroy_texture(SDL_Surface * p_texture);
void texture_loader_cleanup(void);

#endif