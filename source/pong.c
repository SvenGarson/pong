#include <stdio.h>
#include <pong_bool.h>

/* SDL and OpenGL related includes */
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

/* Constants */
const char * P_WINDOW_TITLE = "Pong";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int main(void)
{
  /* Rendering window and surface contained by that window */
  SDL_Window * p_window = NULL;
  SDL_Surface * p_surface = NULL;
  SDL_GLContext * p_opengl_context = NULL;

  /* Initialize SDL subsystems */
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf(stderr, "\n[SDL] Could not initialize subsystems - Error: %s\n", SDL_GetError());
    return -1;
  }

  /* Specify SDL OpenGL window context attributes for window creation */
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  /* Create the SDL rendering window */
  p_window = SDL_CreateWindow(
    P_WINDOW_TITLE,
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

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  /* TODO-GS: Abstract the whole thing and the OpenGL error checking */
  GLenum gl_error = glGetError();
  if (gl_error != GL_NO_ERROR)
  {
    fprintf(stderr, "\n[OpenGL] OpenGL error: %s\n", gluErrorString(gl_error));
    return -1;
  }

  /* Gameloop */
  pong_bool_te window_close_requested = PONG_FALSE;
  while(window_close_requested == PONG_FALSE)
  {
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

    /* Render scene */
    glBegin(GL_TRIANGLES);
      glColor3ub(255, 0, 0);
      glVertex2f(-0.8f, -0.8f);

      glColor3ub(0, 255, 0);
      glVertex2f( 0.8f, -0.8f);

      glColor3ub(0, 0, 255);
      glVertex2f( 0.8f,  0.8f);
    glEnd();

    /* Swap buffers */
    SDL_GL_SwapWindow(p_window);
  }

  /* Cleanup resources and quit SDL subsystems before returning to OS */
  SDL_DestroyWindow(p_window);
  SDL_Quit();

  /* TODO-GS: Return to OS successfully */
  return 0;
}