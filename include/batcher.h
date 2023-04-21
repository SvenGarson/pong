#ifndef BATCHER_H
#define BATCHER_H

/* Includes */
#include <vec2f.h>
#include <SDL2/SDL_opengl.h>
#include <color4ub.h>
#include <text_renderer.h>
#include <pong_bool.h>

/* Defines */
#define BATCHER_MAX_TRIANGLES (1024)

/* Data types */
struct batcher_triangle {
  /* Vertices */
  struct vec2f v0;
  struct vec2f v1;
  struct vec2f v2;

  /* Texture */
  GLuint texture_handle;
  struct vec2f tcv0;
  struct vec2f tcv1;
  struct vec2f tcv2;

  /* Color */
  struct color4ub color;
};

/* Private batcher state */
struct batcher_triangle triangles[BATCHER_MAX_TRIANGLES];
int batched_triangles = 0;
struct color4ub current_color = { 255, 255, 255, 255 };
GLuint current_texture_handle = 0x00;
struct vec2f current_texcoords_v0 = { 0.0f, 0.0f };
struct vec2f current_texcoords_v1 = { 0.0f, 0.0f };
struct vec2f current_texcoords_v2 = { 0.0f, 0.0f };
GLuint text_glyph_texture_handle = 0x00;

/* Private batcher helpers */
void batcher_color(
  unsigned char red,
  unsigned char green,
  unsigned char blue,
  unsigned char alpha
)
{
  current_color.red = red;
  current_color.green = green;
  current_color.blue = blue;
  current_color.alpha = alpha;
}

void batcher_texture_handle(GLuint texture_handle)
{
  current_texture_handle = texture_handle;
}

void batcher_texture_coords
(
  float tcv0x, float tcv0y,
  float tcv1x, float tcv1y,
  float tcv2x, float tcv2y
)
{
  current_texcoords_v0 = (struct vec2f){ tcv0x, tcv0y };
  current_texcoords_v1 = (struct vec2f){ tcv1x, tcv1y };
  current_texcoords_v2 = (struct vec2f){ tcv2x, tcv2y };
}

void batcher_triangle
(
  float v0x, float v0y,
  float v1x, float v1y,
  float v2x, float v2y
)
{
  /* Cap the number of triangles rendered */
  if (batched_triangles >= BATCHER_MAX_TRIANGLES)
  {
    fprintf(stderr, "\n[Batcher] Max number of batch triangles reached at index %d", BATCHER_MAX_TRIANGLES);
    return;
  }

  /* Specify the new triangle */
  struct batcher_triangle * p_triangle = triangles + batched_triangles++;

  /* Vertices */
  p_triangle->v0 = (struct vec2f){ v0x, v0y };
  p_triangle->v1 = (struct vec2f){ v1x, v1y };
  p_triangle->v2 = (struct vec2f){ v2x, v2y };

  /* Texture */
  p_triangle->texture_handle = current_texture_handle;

  /* Texcoords */
  p_triangle->tcv0 = current_texcoords_v0;
  p_triangle->tcv1 = current_texcoords_v1;
  p_triangle->tcv2 = current_texcoords_v2;

  /* Color */
  p_triangle->color = current_color;
}

/* Batcher function definitions */
pong_bool_te batcher_initialize(void)
{
  /* Initialize text renderer as requirement for batched rendering */
  if (text_renderer_initialize() == PONG_FALSE)
    return PONG_FALSE;

  /* Construct OpenGL texture for font rendering */
  /* Solution: https://stackoverflow.com/questions/25771735/creating-opengl-texture-from-sdl2-surface-strange-pixel-values */
  const SDL_Surface * p_glyph_texture = text_renderer_texture_info();
  glGenTextures(1, &text_glyph_texture_handle);
  glBindTexture(GL_TEXTURE_2D, text_glyph_texture_handle);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    p_glyph_texture->w,
    p_glyph_texture->h,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    p_glyph_texture->pixels
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  /* Success */
  return PONG_TRUE;
}

void batcher_cleanup(void)
{
  text_renderer_text_cleanup();
}

void batcher_text
(
  const char * p_text,
  int base_x,
  int base_y,
  int font_height
)
{
  /* Determine text rendering information to batch */
  static struct text_renderer_cache text_info;
  text_renderer_text_info(p_text, base_x, base_y, font_height, &text_info);

  batcher_texture_handle(1); /* TODO-GS: Get actual texture */
  for (int glyph_index = 0; glyph_index < text_info.glyph_infos_generated; glyph_index++)
  {
    const struct text_renderer_glyph_info * p_info = text_info.glyph_infos + glyph_index;

    /* Lower-right glyph triangle */
    batcher_texture_coords(
      p_info->texcoords_region.min.x,
      p_info->texcoords_region.max.y,
      p_info->texcoords_region.max.x,
      p_info->texcoords_region.max.y,
      p_info->texcoords_region.max.x,
      p_info->texcoords_region.min.y
    );
    batcher_triangle(
      p_info->render_region.min.x,
      p_info->render_region.min.y,
      p_info->render_region.max.x,
      p_info->render_region.min.y,
      p_info->render_region.max.x,
      p_info->render_region.max.y
    );

    /* Upper-left glyph triangle */
    batcher_texture_coords(
      p_info->texcoords_region.min.x,
      p_info->texcoords_region.max.y,
      p_info->texcoords_region.max.x,
      p_info->texcoords_region.min.y,
      p_info->texcoords_region.min.x,
      p_info->texcoords_region.min.y
    );
    batcher_triangle(
      p_info->render_region.min.x,
      p_info->render_region.min.y,
      p_info->render_region.max.x,
      p_info->render_region.max.y,
      p_info->render_region.min.x,
      p_info->render_region.max.y
    );
  }
}

void batcher_quadf
(
  float min_x,
  float min_y,
  float max_x,
  float max_y
)
{
  batcher_texture_handle(0);
  batcher_triangle(min_x, min_y, max_x, min_y, max_x, max_y);
  batcher_triangle(min_x, min_y, max_x, max_y, min_x, max_y);
}

void batcher_render(void)
{
  /* Batcher OpenGL settings */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

  /* Batch configuration changes between textured and non-texured primitives */
  GLuint last_texture_handle = 0x00;

  /* Render added batches in the order added */
  for (int triangle_index = 0; triangle_index < batched_triangles; triangle_index++)
  {
    const struct batcher_triangle * p_triangle = triangles + triangle_index;
    pong_bool_te is_textured = p_triangle->texture_handle ? PONG_TRUE : PONG_FALSE;

    /* End and begin batches on texture unit changes */
    if (triangle_index == 0)
    {
      /* First batch configured to the first triangle*/
      if (is_textured)
      {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, p_triangle->texture_handle);
      }
      else
      {
        glDisable(GL_TEXTURE_2D);
      }

      /* Begin the first batch */
      last_texture_handle = p_triangle->texture_handle;
      glColor4ub(p_triangle->color.red, p_triangle->color.green, p_triangle->color.blue, p_triangle->color.alpha);
      glBegin(GL_TRIANGLES);
    }
    else
    {
      /* Successive batch when texture unit has changed */
      const GLuint new_texture_handle = p_triangle->texture_handle;
      if (new_texture_handle != last_texture_handle)
      {
        /* Close current batch */
        glEnd();

        /* Configure the next batch */
        if (is_textured)
        {
          glEnable(GL_TEXTURE_2D);
          glBindTexture(GL_TEXTURE_2D, p_triangle->texture_handle);
        }
        else
        {
          glDisable(GL_TEXTURE_2D);
        }


        /* Begin the next batch */
        last_texture_handle = p_triangle->texture_handle;
        glColor4ub(p_triangle->color.red, p_triangle->color.green, p_triangle->color.blue, p_triangle->color.alpha);
        glBegin(GL_TRIANGLES);
      }
    }

    /* Render data with batch setting */
    glTexCoord2f(p_triangle->tcv0.x, p_triangle->tcv0.y);
    glVertex2f(p_triangle->v0.x, p_triangle->v0.y);

    glTexCoord2f(p_triangle->tcv1.x, p_triangle->tcv1.y);
    glVertex2f(p_triangle->v1.x, p_triangle->v1.y);

    glTexCoord2f(p_triangle->tcv2.x, p_triangle->tcv2.y);
    glVertex2f(p_triangle->v2.x, p_triangle->v2.y);
  }

  /* Clase last batch */
  glEnd();

  /* Clear the buffer */
  batched_triangles = 0;
}

#endif
