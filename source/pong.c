#include <stdio.h>
#include <pong_bool.h>
#include <stdint.h>

/* SDL and OpenGL related includes */
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <vec2i.h>
#include <vec2f.h>
#include <batcher.h>
#include <region2Df.h>
#include <math.h>

/* Defines */
#define WINDOW_MAX_TITLE_LENGTH (64)

/* Datatypes */
struct edge_collider {
  struct vec2f a;
  struct vec2f b;
  struct vec2f center;
  struct vec2f surface_normal;
};

struct ball {
  struct vec2f position;
  float diameter;
  struct vec2f velocity;
};

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

struct ball make_ball(float center_x, float center_y, float diameter, float velocity_x, float velocity_y)
{
  struct ball ball;

  ball.position = (struct vec2f){ center_x, center_y };
  ball.diameter = diameter;
  ball.velocity = (struct vec2f){ velocity_x, velocity_y };

  return ball;
}

struct vec2f vec2f_sub(struct vec2f left, struct vec2f right)
{
  return (struct vec2f){
    left.x - right.x,
    left.y - right.y
  };
}

struct vec2f vec2f_scale(struct vec2f v, float scale)
{
  return (struct vec2f){ v.x * scale, v.y * scale };
}

float vec2f_dot(struct vec2f a, struct vec2f b)
{
  return (a.x * b.x) + (a.y * b.y);
}

float vec2f_length(struct vec2f v)
{
  return sqrt((v.x * v.x) + (v.y * v.y));
}

struct vec2f vec2f_normalize(struct vec2f v)
{
  const float vlen = vec2f_length(v);
  if (vlen != 0.0f)
    return (struct vec2f){ v.x / vlen, v.y / vlen};
  else
    return v;
}

struct vec2f edge_tangent(struct vec2f a, struct vec2f b)
{
  return vec2f_normalize(vec2f_sub(b, a));
}

struct vec2f left_normal(struct vec2f tangent)
{
  const struct vec2f tangent_norm = vec2f_normalize(tangent);
  return (struct vec2f){ -tangent_norm.y, tangent_norm.x };
}

struct edge_collider make_edge_collider
(
  float ax,
  float ay,
  float bx,
  float by
)
{
  struct edge_collider collider;

  collider.a = (struct vec2f){ ax, ay };
  collider.b = (struct vec2f){ bx, by };
  collider.center = (struct vec2f){
    collider.a.x + 0.5f * (collider.b.x - collider.a.x),
    collider.a.y + 0.5f * (collider.b.y - collider.a.y)
  };
  collider.surface_normal = left_normal(
    edge_tangent(collider.a, collider.b)
  );

  return collider;
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

  /* Ball */
  const float BALL_SPEED_PIXELS_PER_SECOND = 100.0f;
  const struct vec2f PLAYFIELD_CENTER = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
  struct ball ball = make_ball(
    PLAYFIELD_CENTER.x,
    PLAYFIELD_CENTER.y,
    30.0f,
    BALL_SPEED_PIXELS_PER_SECOND,
    BALL_SPEED_PIXELS_PER_SECOND
  );

  /* Collision edges */
  const int PADDLE_HIT_INSET = 50;
  const struct edge_collider colliders[] = {
    make_edge_collider(WINDOW_WIDTH - PADDLE_HIT_INSET, WINDOW_HEIGHT, PADDLE_HIT_INSET, WINDOW_HEIGHT)
  };
  const int collider_count = sizeof(colliders) / sizeof(struct edge_collider);

  /* Gameloop */
  pong_bool_te window_close_requested = PONG_FALSE;
  while(window_close_requested == PONG_FALSE)
  {
    /* Integration */
    const double new_time_in_seconds = time_in_seconds();
    const double dts = new_time_in_seconds - last_time_in_seconds;
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

    /* Integrate scene objects */
      /* Compute ball bounding box */
      const struct region2Df region_ball = {
        { ball.position.x - ball.diameter * 0.5f, ball.position.y - ball.diameter * 0.5f },
        { ball.position.x + ball.diameter * 0.5f, ball.position.y + ball.diameter * 0.5f }
      };

      /* Compute list of ball corner positions */
      struct vec2f ball_corners[] = {
        { region_ball.min.x, region_ball.min.y },
        { region_ball.max.x, region_ball.min.y },
        { region_ball.max.x, region_ball.max.y },
        { region_ball.min.x, region_ball.max.y }
      };

      /* Ball */
      /*
          - for all colliders
            - get shortest time + collider info
          - to earliest surface + deflect velocity
      */
      const struct edge_collider * p_earlies_collider = NULL;
      for (int collider_index = 0; collider_index < collider_count; collider_index++)
      {
        const struct edge_collider * const p_collider = colliders + collider_index;

        /* Get ball point closes to current collider surface */
        const struct vec2f * p_closest_corner = NULL;
        float closest_corner_distance = 1000000.0f;
        for (int i_ball_corner = 0; i_ball_corner < sizeof(ball_corners) / sizeof(ball_corners[0]); i_ball_corner++)
        {
          const struct vec2f * const p_corner = ball_corners + i_ball_corner;
          const struct vec2f edge_to_corner = vec2f_sub(*p_corner, p_collider->a);
          const float corner_surface_distance = vec2f_dot(edge_to_corner, p_collider->surface_normal);

          if (corner_surface_distance < closest_corner_distance)
          {
            closest_corner_distance = corner_surface_distance;
            p_closest_corner = p_corner;
          }
        }

        /* Ignore current collider the ball has already sunk under the collider - This should never happen */
        if (closest_corner_distance < 0.0f || p_closest_corner == NULL)
          continue;

        /* Scale the ball velocity to the time-step - TODO-GS: Maybe just store the ball direction and set the scaled velocity once */
        struct vec2f scaled_ball_velocity = vec2f_scale(ball.velocity, dts);

        /* Determine time of impact by projecting the ball velocity - Ignore when ball moving away from surface*/
        const float projected_velocity = vec2f_dot(scaled_ball_velocity, p_collider->surface_normal);
        if (projected_velocity >= 0.0f)
          continue;

        /* Determine time of impact */
        const float impact_time = closest_corner_distance / -projected_velocity;
        if (impact_time >= 0.0f && impact_time <= 1.0f)
        {
          printf("\nHit!");
        }
      }

      /* Integrate ball */
      ball.position.x += BALL_PIXELS_PER_SECOND * dts;
      ball.position.y += BALL_PIXELS_PER_SECOND * dts;

    /* Clear scene */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Render scene */
    batcher_quadf(
      ball.position.x - ball.diameter * 0.5f,
      ball.position.y - ball.diameter * 0.5f,
      ball.position.x + ball.diameter * 0.5f,
      ball.position.y + ball.diameter * 0.5f
    );

    /* Render batches */
    batcher_render();

    /* TODO-GS: Remove debug rendering */
    for (int collider_index = 0; collider_index < collider_count; collider_index++)
    {
      const struct edge_collider * const p_collider = colliders + collider_index;
      /* Tangent and surface normal */
      glColor3ub(150, 150, 150);
      glLineWidth(2.0f);
      glBegin(GL_LINES);
        /* Edge surface */
        glVertex2f(p_collider->a.x, p_collider->a.y);
        glVertex2f(p_collider->b.x, p_collider->b.y);

        /* Edge surface normal */
        const int NORMAL_LENGTH = 40.0f;
        glVertex2f(p_collider->center.x, p_collider->center.y);
        glVertex2f(
          p_collider->center.x + p_collider->surface_normal.x * NORMAL_LENGTH,
          p_collider->center.y + p_collider->surface_normal.y * NORMAL_LENGTH
        );
      glEnd();

      /* Edge points and center */
      glColor3ub(255, 0, 0);
      glPointSize(5.0f);
      glBegin(GL_POINTS);
        glVertex2f(p_collider->center.x, p_collider->center.y);
        glVertex2f(p_collider->a.x, p_collider->a.y);
        glVertex2f(p_collider->b.x, p_collider->b.y);
      glEnd();

      /* Ball corner points */
      glColor3ub(0, 255, 0);
      glBegin(GL_POINTS);
        for (int corner_index = 0; corner_index < sizeof(ball_corners) / sizeof(ball_corners[0]); corner_index++)
        {
          glVertex2f(ball_corners[corner_index].x, ball_corners[corner_index].y);
        }
      glEnd();
    }

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