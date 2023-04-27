#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

/* Includes */
#include <pong_bool.h>
#include <vec2i.h>
#include <region2Di.h>
#include <region2Df.h>
#include <SDL2/SDL.h>

/* Defines */
#define TEXT_RENDERER_MAX_GLYPHS_RENDERED (128)

/* Datatypes */
struct text_renderer_glyph_info {
  struct region2Di render_region;
  struct region2Df texcoords_region;
};

struct text_renderer_cache {
  int glyph_infos_generated;
  struct text_renderer_glyph_info glyph_infos[TEXT_RENDERER_MAX_GLYPHS_RENDERED];
};

/* Function prototypes*/
pong_bool_te text_renderer_initialize(void);
const SDL_Surface * text_renderer_texture_info(void);
void text_renderer_text_info(
  const char * p_text,
  int base_x,
  int base_y,
  int desired_font_height,
  struct text_renderer_cache * p_cache
);
void text_renderer_text_cleanup(void);

#endif