#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

/* Includes */
#include <pong_bool.h>
#include <vec2i.h>
#include <region2Di.h>
#include <region2Df.h>
#include <texture_loader.h>

/* Defines */
#define TEXT_RENDERER_MAX_GLYPHS_RENDERED (128)
#define ASCII_CODE_RANGE_FLOOR (0)
#define ASCII_CODE_RANGE_CEILING (255)

/* Constants */
const int GLYPH_WIDTH_IN_PIXELS = 5;
const int GLYPH_HEIGHT_IN_PIXELS = 9;

/* Datatypes */
struct text_renderer_glyph_info {
  struct region2Di render_region;
  struct region2Df texcoords_region;
};

struct text_renderer_cache {
  int glyph_infos_generated;
  struct text_renderer_glyph_info glyph_infos[TEXT_RENDERER_MAX_GLYPHS_RENDERED];
};

static struct ascii_glyph_info {
  pong_bool_te is_printable;
  struct region2Df texcoords_region;
};

/* Private state */
static pong_bool_te text_renderer_initialized = PONG_FALSE;
static struct ascii_glyph_info ascii_glyph_info_store[ASCII_CODE_RANGE_CEILING];
static const SDL_Surface * p_glyph_texture = NULL;

/* Private helper functions */
pong_bool_te is_valid_ascii_code(int ascii_code)
{
  return (
    ascii_code >= ASCII_CODE_RANGE_FLOOR &&
    ascii_code <= ASCII_CODE_RANGE_CEILING
  ) ? PONG_TRUE : PONG_FALSE;
}

pong_bool_te is_printable(int ascii_code)
{
  return is_valid_ascii_code(ascii_code) ? ascii_glyph_info_store[ascii_code].is_printable : PONG_FALSE;
}

void register_printable_glyph(int ascii_code, int texture_grid_x, int texture_grid_y)
{
  if (ascii_code < ASCII_CODE_RANGE_FLOOR || ascii_code > ASCII_CODE_RANGE_CEILING)
    fprintf(stderr, "[Text renderer] Attempting to register out of range ascii code '%d'", ascii_code);

  /* Determine the glyph texcoords based on the texture grid region */
  /* TODO-GS: Compute from the actual texture dimensions */
  struct ascii_glyph_info * const p_info = ascii_glyph_info_store + ascii_code;
  const float texture_glyph_span_x = (float)GLYPH_WIDTH_IN_PIXELS / (float)p_glyph_texture->w;
  const float texture_glyph_span_y = (float)GLYPH_HEIGHT_IN_PIXELS / (float)p_glyph_texture->h;

  p_info->texcoords_region.min.x = ((texture_grid_x * (GLYPH_WIDTH_IN_PIXELS + 1)) + 1) / (float)p_glyph_texture->w;
  p_info->texcoords_region.min.y = ((texture_grid_y * (GLYPH_HEIGHT_IN_PIXELS + 1)) + 1) / (float)p_glyph_texture->h;
  p_info->texcoords_region.max.x = p_info->texcoords_region.min.x + texture_glyph_span_x;
  p_info->texcoords_region.max.y = p_info->texcoords_region.min.y + texture_glyph_span_y;

  p_info->is_printable = PONG_TRUE;
}

/* Functions */
pong_bool_te text_renderer_initialize(void)
{
  /* Initialize only on the first invocation */
  if (text_renderer_initialized == PONG_TRUE)
    return PONG_TRUE;

  /* Make sure the texture loader is initialized */
  if (texture_loader_initialize() == PONG_FALSE)
    return PONG_FALSE;

  /* Load the glyph texture for all text rendering */
  p_glyph_texture = texture_loader_load_texture("font_5x9_glyph_texture.png");
  if (p_glyph_texture == NULL)
  {
    fprintf(stderr, "\n[Pong] Could not load the required glyph texture for font rendering");
    return PONG_FALSE;
  }

  /* Initialize all glyphs to non-printable */
  for (int ascii_code = ASCII_CODE_RANGE_FLOOR; ascii_code < ASCII_CODE_RANGE_CEILING; ascii_code++)
  {
    ascii_glyph_info_store[ascii_code].is_printable = PONG_FALSE;
  }

  /* Register printable ascii glyphs */
  register_printable_glyph('A', 0, 0);
  register_printable_glyph('B', 1, 0);
  register_printable_glyph('C', 2, 0);
  register_printable_glyph('D', 3, 0);
  register_printable_glyph('E', 4, 0);
  register_printable_glyph('F', 5, 0);
  register_printable_glyph('G', 6, 0);
  register_printable_glyph('H', 7, 0);
  register_printable_glyph('I', 8, 0);
  register_printable_glyph('J', 9, 0);
  register_printable_glyph('K', 10, 0);
  register_printable_glyph('L', 11, 0);
  register_printable_glyph('M', 12, 0);
  register_printable_glyph('N', 13, 0);
  register_printable_glyph('O', 14, 0);
  register_printable_glyph('P', 15, 0);
  register_printable_glyph('Q', 16, 0);
  register_printable_glyph('R', 17, 0);
  register_printable_glyph('S', 18, 0);
  register_printable_glyph('T', 19, 0);
  register_printable_glyph('U', 20, 0);
  register_printable_glyph('V', 0, 1);
  register_printable_glyph('W', 1, 1);
  register_printable_glyph('X', 2, 1);
  register_printable_glyph('Y', 3, 1);
  register_printable_glyph('Z', 4, 1);

  register_printable_glyph('a', 5, 1);
  register_printable_glyph('b', 6, 1);
  register_printable_glyph('c', 7, 1);
  register_printable_glyph('d', 8, 1);
  register_printable_glyph('e', 9, 1);
  register_printable_glyph('f', 10, 1);
  register_printable_glyph('g', 11, 1);
  register_printable_glyph('h', 12, 1);
  register_printable_glyph('i', 13, 1);
  register_printable_glyph('j', 14, 1);
  register_printable_glyph('k', 15, 1);
  register_printable_glyph('l', 16, 1);
  register_printable_glyph('m', 17, 1);
  register_printable_glyph('n', 18, 1);
  register_printable_glyph('o', 19, 1);
  register_printable_glyph('p', 20, 1);
  register_printable_glyph('q', 0, 2);
  register_printable_glyph('r', 1, 2);
  register_printable_glyph('s', 2, 2);
  register_printable_glyph('t', 3, 2);
  register_printable_glyph('u', 4, 2);
  register_printable_glyph('v', 5, 2);
  register_printable_glyph('w', 6, 2);
  register_printable_glyph('x', 7, 2);
  register_printable_glyph('y', 8, 2);
  register_printable_glyph('z', 9, 2);

  register_printable_glyph('0', 10, 2);
  register_printable_glyph('1', 11, 2);
  register_printable_glyph('2', 12, 2);
  register_printable_glyph('3', 13, 2);
  register_printable_glyph('4', 14, 2);
  register_printable_glyph('5', 15, 2);
  register_printable_glyph('6', 16, 2);
  register_printable_glyph('7', 17, 2);
  register_printable_glyph('8', 18, 2);
  register_printable_glyph('9', 19, 2);

  register_printable_glyph('!', 0, 3);
  register_printable_glyph('"', 1, 3);
  register_printable_glyph('#', 2, 3);
  register_printable_glyph('$', 3, 3);
  register_printable_glyph('%', 4, 3);
  register_printable_glyph('&', 5, 3);
  register_printable_glyph('\'', 6, 3);
  register_printable_glyph('(', 7, 3);
  register_printable_glyph(')', 8, 3);
  register_printable_glyph('*', 9, 3);
  register_printable_glyph('+', 10, 3);
  register_printable_glyph(',', 11, 3);
  register_printable_glyph('-', 12, 3);
  register_printable_glyph('.', 13, 3);
  register_printable_glyph('/', 14, 3);
  register_printable_glyph(':', 15, 3);
  register_printable_glyph(';', 16, 3);
  register_printable_glyph('<', 17, 3);
  register_printable_glyph('=', 18, 3);
  register_printable_glyph('>', 19, 3);
  register_printable_glyph('?', 20, 3);

  register_printable_glyph('@', 0, 4);
  register_printable_glyph('[', 1, 4);
  register_printable_glyph('\\', 2, 4);
  register_printable_glyph(']', 3, 4);
  register_printable_glyph('^', 4, 4);
  register_printable_glyph('_', 5, 4);
  register_printable_glyph('`', 6, 4);
  register_printable_glyph('{', 7, 4);
  register_printable_glyph('|', 8, 4);
  register_printable_glyph('}', 9, 4);
  register_printable_glyph('~', 10, 4);

  /* Initialization done */
  text_renderer_initialized = PONG_TRUE;
  return PONG_TRUE;
}

const SDL_Surface * text_renderer_texture_info(void)
{
  return p_glyph_texture;
}

void text_renderer_text_info(
  const char * p_text,
  int base_x,
  int base_y,
  int desired_font_height,
  struct text_renderer_cache * p_cache
)
{
  if (p_cache == NULL)
    return;

  /* Reset the cache */
  p_cache->glyph_infos_generated = 0;

  /* Do nothing when no text given */
  if (p_text == NULL)
    return;

  /* Determine font-size multiplier to keep glyphs rendered in the original ratio */
  if (desired_font_height < GLYPH_HEIGHT_IN_PIXELS)
    desired_font_height = GLYPH_HEIGHT_IN_PIXELS;
  const int FONT_SCALE = desired_font_height / GLYPH_HEIGHT_IN_PIXELS;

  /* Generate text glyph info */
  struct vec2i glyph_cursor = { base_x, base_y };
  const char * p_text_char = NULL;
  for (p_text_char = p_text; *p_text_char; p_text_char++)
  {
    /* TODO-GS: Stop when cache is full */
    if (p_cache->glyph_infos_generated >= TEXT_RENDERER_MAX_GLYPHS_RENDERED)
    {
      printf("\nCache full at index: %d", p_cache->glyph_infos_generated);
      return;
    }

    /* Discern between printing and control characters */
    const int SPACES_PER_TAB = 2;
    const int SCALED_GLYPH_WIDTH = GLYPH_WIDTH_IN_PIXELS * FONT_SCALE;
    const int SCALED_GLYPH_HEIGHT = GLYPH_HEIGHT_IN_PIXELS * FONT_SCALE;
    const char text_char = *p_text_char;
    if (text_char == '\n')
    {
      /* Next line */
      glyph_cursor.y -= SCALED_GLYPH_HEIGHT + FONT_SCALE;
      glyph_cursor.x = base_x;
    }
    else if (text_char == ' ')
    {
      /* Skip char */
      glyph_cursor.x += SCALED_GLYPH_WIDTH + FONT_SCALE;
    }
    else if (is_printable(text_char))
    {
      /* Generate textured quad rendering info */
      struct text_renderer_glyph_info * const p_info = p_cache->glyph_infos + p_cache->glyph_infos_generated++;

      /* Rendering region */
      p_info->render_region.min.x = glyph_cursor.x;
      p_info->render_region.min.y = glyph_cursor.y - SCALED_GLYPH_HEIGHT;
      p_info->render_region.max.x = glyph_cursor.x + SCALED_GLYPH_WIDTH;
      p_info->render_region.max.y = glyph_cursor.y;

      /* Texcoords region */
      p_info->texcoords_region = ascii_glyph_info_store[text_char].texcoords_region;

      /* To next character */
      glyph_cursor.x += SCALED_GLYPH_WIDTH + FONT_SCALE;
    }
    else {
      /* Non-printable and non-control character */
    }
  }
}

void text_renderer_text_cleanup(void)
{
  texture_loader_destroy_texture(p_glyph_texture);
  texture_loader_cleanup();
  text_renderer_initialized = PONG_FALSE;
}

#endif