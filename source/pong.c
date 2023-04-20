#include <stdio.h>
#include <pong_bool.h>
#include <stdint.h>
#include <text_renderer.h>

/* SDL and OpenGL related includes */
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

/* Defines */
#define WINDOW_MAX_TITLE_LENGTH (64)

/* Constants */
const char * WINDOW_TITLE = "Pong";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int main(void)
{
  /* Rendering window and surface contained by that window */
  SDL_Window * p_window = NULL;
  SDL_Surface * p_surface = NULL;
  SDL_GLContext * p_opengl_context = NULL;

  /* Initialize SDL subsystems */
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
  {
    fprintf(stderr, "\n[SDL] Could not initialize subsystems - Error: %s\n", SDL_GetError());
    return -1;
  }

  /* Specify SDL OpenGL window context attributes for window creation */
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  /* Create the SDL rendering window */
  p_window = SDL_CreateWindow(
    WINDOW_TITLE,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
  );

  if (p_window == NULL)
  {
    fprintf(stderr, "\n[SDL] Could not create window - Error: %s\n", SDL_GetError());
    return -1;
  }

  /* Create OpenGL context for created window */
  p_opengl_context = SDL_GL_CreateContext(p_window);
  if (p_opengl_context == NULL)
  {
    fprintf(stderr, "\n[SDL] Could not attribute OpenGL context to window - Error: %s\n", SDL_GetError());
    return -1;
  }

  /* Configure display sync option - TODO-GS: The call is correct, just not working under Ubuntu? */
  const int SDL_SYNC_NONE = 0;
  const int SDL_VSYNC = 1;
  const int SDL_SYNC_ADAPTIVE = -1;
  if (SDL_GL_SetSwapInterval(SDL_VSYNC) < 0)
  {
    fprintf(stderr, "\n[SDL] Selected display sync option not supported - Error: %s\n", SDL_GetError());
  }

  /* Initialize OpenGL state */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  /* TODO-GS: Abstract the whole thing and the OpenGL error checking */
  GLenum gl_error = glGetError();
  if (gl_error != GL_NO_ERROR)
  {
    fprintf(stderr, "\n[OpenGL] OpenGL error: %s\n", gluErrorString(gl_error));
    return -1;
  }

  /* Initialize the text renderer */
  if (text_renderer_initialize() == PONG_FALSE)
  {
    fprintf(stderr, "\n[Text renderer] Could not initialize the text renderer");
    return -1;
  }

  /* Gameloop timing */
  uint64_t fps_counter_last = SDL_GetPerformanceCounter();
  int frames_per_second = 0;

  /* Construct OpenGL texture for font rendering */
  /* Solution: https://stackoverflow.com/questions/25771735/creating-opengl-texture-from-sdl2-surface-strange-pixel-values */
  const SDL_Surface * p_glyph_texture = text_renderer_texture_info();
  GLuint text_renderer_texture_handle;
  glGenTextures(1, &text_renderer_texture_handle);
  glBindTexture(GL_TEXTURE_2D, text_renderer_texture_handle);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    p_glyph_texture->w, p_glyph_texture->h,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    p_glyph_texture->pixels
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  /* Gameloop */
  pong_bool_te window_close_requested = PONG_FALSE;
  while(window_close_requested == PONG_FALSE)
  {
    /* FPS counter */
    frames_per_second++;
    const uint64_t fps_counter_new = SDL_GetPerformanceCounter();
    const uint64_t fps_counter_delta = fps_counter_new - fps_counter_last;
    const double fps_delta_seconds = (double)fps_counter_delta / (double)SDL_GetPerformanceFrequency();
    if (fps_delta_seconds > 1.0)
    {
      /* Update FPS counter in title */
      char fps_window_title[WINDOW_MAX_TITLE_LENGTH];
      snprintf(
        fps_window_title,
        WINDOW_MAX_TITLE_LENGTH,
        "%s - FPS: %d",
        WINDOW_TITLE,
        frames_per_second
      );
      SDL_SetWindowTitle(p_window, fps_window_title);

      /* Reset counter */
      fps_counter_last = fps_counter_new;
      frames_per_second = 0;
    }

    /* Poll events */
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch(event.type)
      {
        case SDL_QUIT:
          window_close_requested = PONG_TRUE;
          break;
        case SDL_KEYDOWN:
          if (event.key.keysym.sym == SDLK_ESCAPE)
            window_close_requested = PONG_TRUE;
          break;
        default:
          break;
      }
    }

    /* Clear scene */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Render scene */
    glColor3ub(255, 255, 255);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glBindTexture(GL_TEXTURE_2D, text_renderer_texture_handle);

    struct text_renderer_cache cache;
    text_renderer_text_info(SDL_GetWindowTitle(p_window), 0, 300, 27, &cache);  

    glColor3ub(255, 0, 0);
    glBegin(GL_QUADS);
    for (int i = 0; i < cache.glyph_infos_generated; i++)
    {
      const struct text_renderer_glyph_info * p_info = cache.glyph_infos + i;
      glTexCoord2f(p_info->texcoords_region.min.x, p_info->texcoords_region.max.y);
      glVertex2f(p_info->render_region.min.x, p_info->render_region.min.y);

      glTexCoord2f(p_info->texcoords_region.max.x, p_info->texcoords_region.max.y);
      glVertex2f(p_info->render_region.max.x, p_info->render_region.min.y);

      glTexCoord2f(p_info->texcoords_region.max.x, p_info->texcoords_region.min.y);
      glVertex2f(p_info->render_region.max.x, p_info->render_region.max.y);

      glTexCoord2f(p_info->texcoords_region.min.x, p_info->texcoords_region.min.y);
      glVertex2f(p_info->render_region.min.x, p_info->render_region.max.y);
    }
    glEnd();

    /* Swap buffers */
    SDL_GL_SwapWindow(p_window);
  }

  /* Cleanup */
  text_renderer_text_cleanup();
  SDL_DestroyWindow(p_window);
  SDL_Quit();

  /* TODO-GS: Return to OS successfully */
  return 0;
}