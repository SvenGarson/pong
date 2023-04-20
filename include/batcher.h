#ifndef BATCHER_H
#define BATCHER_H

/* Includes */
#include <vec2f.h>
#include <SDL2/SDL_opengl.h>
#include <color4ub.h>

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
void batcher_text
(
  const char * p_text,
  int base_x,
  int base_y,
  int font_height
)
{
  /* TODO-GS: Render text and get info and texure handle from there */
  batcher_texture_handle(1);
  batcher_texture_coords(0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);
  batcher_triangle(base_x, base_y, base_x + 256, base_y, base_x + 256, base_y + 256);

  batcher_texture_coords(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
  batcher_triangle(base_x, base_y, base_x + 256, base_y + 256, base_x, base_y + 256);
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
  /* Render added batches in the order added */
  for (int triangle_index = 0; triangle_index < batched_triangles; triangle_index++)
  {
    const struct batcher_triangle * p_triangle = triangles + triangle_index;

    /* Texture VS non-textured primitive */
    if (p_triangle->texture_handle == 0x00)
    {
      /* No Texture */
      glDisable(GL_BLEND);
      glDisable(GL_TEXTURE_2D);
      glColor4ub(p_triangle->color.red, p_triangle->color.green, p_triangle->color.blue, p_triangle->color.alpha);

      glBegin(GL_TRIANGLES);
        glVertex2f(p_triangle->v0.x, p_triangle->v0.y);
        glVertex2f(p_triangle->v1.x, p_triangle->v1.y);
        glVertex2f(p_triangle->v2.x, p_triangle->v2.y);
      glEnd();
    }
    else
    {
      /* Texture */
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, p_triangle->texture_handle);

      glColor4ub(p_triangle->color.red, p_triangle->color.green, p_triangle->color.blue, p_triangle->color.alpha);
      glBegin(GL_TRIANGLES);
        glTexCoord2f(p_triangle->tcv0.x, p_triangle->tcv0.y);
        glVertex2f(p_triangle->v0.x, p_triangle->v0.y);

        glTexCoord2f(p_triangle->tcv1.x, p_triangle->tcv1.y);
        glVertex2f(p_triangle->v1.x, p_triangle->v1.y);

        glTexCoord2f(p_triangle->tcv2.x, p_triangle->tcv2.y);
        glVertex2f(p_triangle->v2.x, p_triangle->v2.y);
      glEnd();
    }
  }

  /* Clear the buffer */
  batched_triangles = 0;
}

#endif
