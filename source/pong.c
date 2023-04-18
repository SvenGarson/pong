#include <stdio.h>
#include <SDL2/SDL.h>

/* Constants */
const char * P_WINDOW_TITLE = "Pong";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int main(void)
{
  /* Rendering window and surface contained by that window */
  SDL_Window * p_window = NULL;
  SDL_Surface * p_surface = NULL;

  /* Initialize SDL subsystems */
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    fprintf(stderr, "\n[SDL] Could not initialize subsystems - Error: %s", SDL_GetError());
    return -1;
  }

  /* Create the SDL rendering window */
  p_window = SDL_CreateWindow(
    P_WINDOW_TITLE,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    SDL_WINDOW_SHOWN
  );

  /* TODO-GS: Return to OS successfully */
  return 0;
}