#include <stdio.h>
#include <pong_bool.h>
#include <stdint.h>

/* SDL and OpenGL related includes */
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <vec2i.h>
#include <batcher.h>

/* Defines */
#define WINDOW_MAX_TITLE_LENGTH (64)

/* Constants */
const char * WINDOW_TITLE = "Pong";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

/* Helper functions */
void log_opengl_error(void)
{
  GLenum gl_error = glGetError();
  if (gl_error == GL_NO_ERROR) return;

  printf("\n\nOpenGL errors:");
  int error_index = 1;
  while (gl_error != GL_NO_ERROR)
  {
    printf("\n\t[%-3d] %s", error_index++, gluErrorString(gl_error));
    gl_error = glGetError();
  }
}

double time_in_seconds(void)
{
  return (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
}

/* Pong entry point */
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

  /* Initialize batch renderer */
  if (batcher_initialize() == PONG_FALSE)
  {
    fprintf(stderr, "\n[Pong] Could not initialize the batch renderer");
    return -1;
  }

  /* Gameloop timing */
  uint64_t fps_counter_last = SDL_GetPerformanceCounter();
  int frames_per_second = 0;

  /* Pong game variables */
  /* Input */
  pong_bool_te paddle_left_up_pressed = PONG_FALSE;
  pong_bool_te paddle_left_down_pressed = PONG_FALSE;

  /* Integration */
  double last_time_in_seconds = time_in_seconds();
  const float PADDLE_PIXELS_PER_SECOND = 500.0f;
  const float BALL_PIXELS_PER_SECOND = 50.0f;

  /* Positioning and sizing */
  const int PADDLE_INSET = 50;
  const struct vec2i PADDLE_DIMENSIONS = { 20, 50 };
  struct vec2f paddle_left_pos = { PADDLE_INSET, WINDOW_HEIGHT / 2 };
  struct vec2f paddle_right_pos = { WINDOW_WIDTH - PADDLE_INSET - PADDLE_DIMENSIONS.x, WINDOW_HEIGHT / 2 };
  const int BALL_SIZE = 15;
  const struct vec2f BALL_SPAWN_POS = { (WINDOW_WIDTH / 2.0f) - (BALL_SIZE / 2.0f), (WINDOW_HEIGHT / 2.0f) - (BALL_SIZE / 2.0f) };
  struct vec2f ball_pos = BALL_SPAWN_POS;
  struct vec2f ball_velocity = { BALL_PIXELS_PER_SECOND, BALL_PIXELS_PER_SECOND };

  /* Scoring */
  int score_left = 0;
  int score_right = 0;

  /* Gameloop */
  pong_bool_te window_close_requested = PONG_FALSE;
  while(window_close_requested == PONG_FALSE)
  {
    /* Integration */
    const double new_time_in_seconds = time_in_seconds();
    const double delta_time_in_seconds = new_time_in_seconds - last_time_in_seconds;
    last_time_in_seconds = new_time_in_seconds;

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

    /* Poll events into required key states */
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch(event.type)
      {
        case SDL_QUIT:
          window_close_requested = PONG_TRUE;
          break;
        case SDL_KEYDOWN:
          /* Process key presses */
          switch(event.key.keysym.sym)
          {
            case SDLK_ESCAPE:
              window_close_requested = PONG_TRUE;
              break;
            case SDLK_LSHIFT:
              paddle_left_up_pressed = PONG_TRUE;
              break;
            case SDLK_LCTRL:
              paddle_left_down_pressed = PONG_TRUE;
              break;
          }
          break;
        case SDL_KEYUP:
          /* Process key releases */
          switch(event.key.keysym.sym)
          {
            case SDLK_LSHIFT:
              paddle_left_up_pressed = PONG_FALSE;
              break;
            case SDLK_LCTRL:
              paddle_left_down_pressed = PONG_FALSE;
              break;
          }
          break;
        default:
          break;
      }
    }

    /* Pong logic */
    /* Controlling paddles */
    if (paddle_left_up_pressed)
    {
      paddle_left_pos.y += PADDLE_PIXELS_PER_SECOND * delta_time_in_seconds;
    }
    if (paddle_left_down_pressed)
    {
      paddle_left_pos.y -= PADDLE_PIXELS_PER_SECOND * delta_time_in_seconds;
    }
    if (paddle_left_pos.y + PADDLE_DIMENSIONS.y >= WINDOW_HEIGHT)
      paddle_left_pos.y = WINDOW_HEIGHT - PADDLE_DIMENSIONS.y;
    if (paddle_left_pos.y <= 0.0f)
      paddle_left_pos.y = 0.0f;

    /* Integrating the ball - TODO-GS: Make this continuous */
    /* Collision detection */
    /*
        - Continuous check against edges (vertical and horizontal) by TOI
        - Set position to surface and deflect + scale the velocity to the rest?
        - Keep integrating until the velocity fully consumed for the integration step

        Q & A
          - How to react to two exact collisions on both axis?
            Handle both if TOI very close together!? Is that every relevant?
    */
    /* Continuous collision detection and deflection */
    const struct region2Df BALL_REGION = { { ball_pos.x, ball_pos.y }, { ball_pos.x + BALL_SIZE, ball_pos.y + BALL_SIZE }};
    /* Vertical playfield bounds */
    const float TOI_PLAYFIELD_TOP = (WINDOW_HEIGHT - BALL_REGION.max.y) / ball_velocity.y;
    
    /* Velocity */
    ball_pos.x += ball_velocity.x * delta_time_in_seconds;
    ball_pos.y += ball_velocity.y * delta_time_in_seconds;
    /* Reset and scoring */
    if ((ball_pos.x + BALL_SIZE) < 0.0f)
    {
      /* Right player scores - Left player turn */
      score_right++;
      ball_pos = BALL_SPAWN_POS;
      ball_velocity = (struct vec2f){ BALL_PIXELS_PER_SECOND, BALL_PIXELS_PER_SECOND };
    }
    if (ball_pos.x >= WINDOW_WIDTH)
    {
      /* Left player scores - Right player turn */
      score_left++;
      ball_pos = BALL_SPAWN_POS;
      ball_velocity = (struct vec2f){ -BALL_PIXELS_PER_SECOND, BALL_PIXELS_PER_SECOND };
    }

    /* Batch scene data */
    /* Left paddle */
    batcher_color(255, 255, 255, 255);
    batcher_quadf(paddle_left_pos.x, paddle_left_pos.y, paddle_left_pos.x + PADDLE_DIMENSIONS.x, paddle_left_pos.y + PADDLE_DIMENSIONS.y);

    /* Right paddle */
    batcher_color(255, 255, 255, 255);
    batcher_quadf(paddle_right_pos.x, paddle_right_pos.y, paddle_right_pos.x + PADDLE_DIMENSIONS.x, paddle_right_pos.y + PADDLE_DIMENSIONS.y);

    /* Ball */
    batcher_color(255, 255, 255, 255);
    batcher_quadf(BALL_REGION.min.x, BALL_REGION.min.y, BALL_REGION.max.x, BALL_REGION.max.y);

    /* Left score */
    batcher_color(255, 255, 255, 255);
    char score_text[16];
    snprintf(score_text, 16, "%2d", score_left);
    batcher_text(score_text, WINDOW_WIDTH * 0.25, WINDOW_HEIGHT * 0.95, 9 * 8);
    snprintf(score_text, 16, "%2d", score_right);
    batcher_text(score_text, WINDOW_WIDTH * 0.55, WINDOW_HEIGHT * 0.95, 9 * 8);

    /* Clear scene */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Render batches */
    batcher_render();

    /* Check OpenGL errors */
    log_opengl_error();

    /* Swap buffers */
    SDL_GL_SwapWindow(p_window);
  }

  /* Cleanup */
  batcher_cleanup();
  SDL_DestroyWindow(p_window);
  SDL_Quit();

  /* TODO-GS: Return to OS successfully */
  return 0;
}