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
#include <time.h>
#include <audio_player.h>

/* Defines */
#define WINDOW_MAX_TITLE_LENGTH (64)
#define SCORE_TEXT_MAX_LENGTH (16)

/* Datatypes */
struct ball {
  struct vec2f position;
  float diameter;
  struct vec2f velocity;
};

struct paddle {
  struct vec2f position;
  struct vec2f dimensions;
};

struct edge_collider {
  struct vec2f a;
  struct vec2f b;
  struct vec2f center;
  struct vec2f surface_normal;
  struct paddle * p_associated_paddle;
};

struct range2f {
  float min;
  float max;
};

/* Constants */
const char * WINDOW_TITLE = "Pong";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float PADDLE_PIXELS_PER_SECOND = 500.0f;
const float BALL_SPEED_PIXELS_PER_SECOND = 350.0f;

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

double degrees_to_radians(double degrees)
{
  return degrees * (M_PI / 180.0);
}

struct paddle make_paddle(float center_x, float center_y, float dimension_x, float dimension_y)
{
  struct paddle paddle;

  paddle.position = (struct vec2f){ center_x, center_y };
  paddle.dimensions = (struct vec2f){ dimension_x, dimension_y };

  return paddle;  
};

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

struct vec2f vec2f_make(float x, float y)
{
  return (struct vec2f) { x, y };
}

struct vec2f biased_random_ball_velocity(int horizontal_direction)
{
  /* Chose a random horizontal direction when none specified */
  if (horizontal_direction == 0)
    horizontal_direction = ((rand() % 2) == 0) ? - 1 : 1;

  /* Keep the angle to 45 degrees (or so) from the vertical divider */
  const float random_offset_angle = (float)(rand() % 46);
  const int direction_left = (horizontal_direction <= 0) ? 1 : 0;

  /* Random vertical direction */
  const int random_vertical = rand();
  float chosen_random_angle;
  if ((random_vertical % 2) == 0)
  {
    /* Up */
    chosen_random_angle = direction_left ? (180.0f - random_offset_angle) : random_offset_angle;
  }
  else
  {
    /* Down */
    chosen_random_angle = direction_left ? (180.0f + random_offset_angle) : -random_offset_angle;
  }

  struct vec2f random_direction = vec2f_make(
    cos(degrees_to_radians(chosen_random_angle)),
    sin(degrees_to_radians(chosen_random_angle))
  );

  /* Construct velocity vector */
  return vec2f_scale(vec2f_normalize(random_direction), BALL_SPEED_PIXELS_PER_SECOND);
}

void set_ball_velocity(struct ball * p_ball, struct vec2f new_velocity)
{
  if (!p_ball)
    return;

  p_ball->velocity = new_velocity;
}

float flt_list_min(const float * p_float_list, size_t entries)
{
  const float * p_min = NULL;
  for (int i = 0; i < entries; i++)
  {
    const float * p_value = p_float_list + i;
    if (p_min == NULL || *p_value < *p_min)
    {
      p_min = p_value;
    }
  }

  return *p_min;
}

float flt_list_max(const float * p_float_list, size_t entries)
{
  const float * p_max = NULL;
  for (int i = 0; i < entries; i++)
  {
    const float * p_value = p_float_list + i;
    if (p_max == NULL || *p_value > *p_max)
    {
      p_max = p_value;
    }
  }

  return *p_max;
}

struct range2f project_region_onto_edge(struct region2Df * p_region, struct edge_collider * p_collider)
{
  /* Determine vectors from surface to region edges */
  const struct vec2f collider_to_corner_1 = vec2f_sub((struct vec2f){ p_region->min.x, p_region->min.y }, p_collider->a);
  const struct vec2f collider_to_corner_2 = vec2f_sub((struct vec2f){ p_region->max.x, p_region->min.y }, p_collider->a);
  const struct vec2f collider_to_corner_3 = vec2f_sub((struct vec2f){ p_region->max.x, p_region->max.y }, p_collider->a);
  const struct vec2f collider_to_corner_4 = vec2f_sub((struct vec2f){ p_region->min.x, p_region->max.y }, p_collider->a);

  /* Project edges onto collider */
  const struct vec2f collider_tangent = vec2f_normalize(vec2f_sub(p_collider->b, p_collider->a));
  const float projections[] = {
    vec2f_dot(collider_to_corner_1, collider_tangent),
    vec2f_dot(collider_to_corner_2, collider_tangent),
    vec2f_dot(collider_to_corner_3, collider_tangent),
    vec2f_dot(collider_to_corner_4, collider_tangent)
  };

  /* Return the projected min/max range */
  struct range2f projected_range;

  const size_t projection_count = sizeof(projections) / sizeof(projections[0]);
  return (struct range2f){
    flt_list_min(projections, projection_count),
    flt_list_max(projections, projection_count)
  };
}

struct edge_collider make_edge_collider
(
  float ax,
  float ay,
  float bx,
  float by,
  struct paddle * p_paddle
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
  collider.p_associated_paddle = p_paddle;

  return collider;
}

double time_in_seconds(void)
{
  return (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
}

struct region2Df region_for_paddles(struct paddle * p_paddle)
{
  struct region2Df region_paddle;

  region_paddle.min = (struct vec2f){
    p_paddle->position.x - p_paddle->dimensions.x * 0.5f,
    p_paddle->position.y - p_paddle->dimensions.y * 0.5f
  };

  region_paddle.max = (struct vec2f){
    p_paddle->position.x + p_paddle->dimensions.x * 0.5f,
    p_paddle->position.y + p_paddle->dimensions.y * 0.5f
  };

  return region_paddle;
}

struct region2Df region_for_ball(struct ball * p_ball)
{
  struct region2Df region_ball;

  region_ball.min = (struct vec2f){
    p_ball->position.x - p_ball->diameter * 0.5f,
    p_ball->position.y - p_ball->diameter * 0.5f
  };

  region_ball.max = (struct vec2f){
    p_ball->position.x + p_ball->diameter * 0.5f,
    p_ball->position.y + p_ball->diameter * 0.5f
  };

  return region_ball;
}

/* Pong entry point */
int main(void)
{
  /* Rendering window and surface contained by that window */
  SDL_Window * p_window = NULL;
  SDL_Surface * p_surface = NULL;
  SDL_GLContext * p_opengl_context = NULL;

  /* Initialize SDL subsystems */
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
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

  /* Initialize audio player */
  if (audio_player_initialize() == PONG_FALSE)
  {
    fprintf(stderr, "\n[Pong] Could not initialize the audio player");
    return -1;
  }

  /* Register music and sound effects */
  int sfx_pickup = audio_player_register_sound_effect("pickup.wav");
  int r = audio_player_play_sound_effect(sfx_pickup);
  printf("\nPlay success: %d", r);

  /* Set random time seed */
  srand(time(NULL));

  /* Gameloop timing */
  uint64_t fps_counter_last = SDL_GetPerformanceCounter();
  int frames_per_second = 0;

  /* Pong game variables */
  /* Scoring */
  int score_paddle_left = 0;
  int score_paddle_right = 0;

  /* Input */
  pong_bool_te paddle_left_up_pressed = PONG_FALSE;
  pong_bool_te paddle_left_down_pressed = PONG_FALSE;
  pong_bool_te paddle_right_up_pressed = PONG_FALSE;
  pong_bool_te paddle_right_down_pressed = PONG_FALSE;

  /* Integration */
  double last_time_in_seconds = time_in_seconds();

  /* Ball */
  const struct vec2f PLAYFIELD_CENTER = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
  struct ball ball = make_ball(
    PLAYFIELD_CENTER.x,
    PLAYFIELD_CENTER.y,
    30.0f,
    BALL_SPEED_PIXELS_PER_SECOND,
    BALL_SPEED_PIXELS_PER_SECOND
  );

  /* Randomize ball direction for the first spawn */
  set_ball_velocity(&ball, biased_random_ball_velocity(0));

  /* Paddles */
  const struct vec2f PADDLE_DIMENSIONS = { 5, WINDOW_HEIGHT * 0.2f };
  const int PADDLE_HIT_INSET = 50;
  struct paddle paddle_left = make_paddle(
    PADDLE_HIT_INSET - (PADDLE_DIMENSIONS.x * 0.5f),
    WINDOW_HEIGHT * 0.5f,
    PADDLE_DIMENSIONS.x,
    PADDLE_DIMENSIONS.y
  );

  struct paddle paddle_right = make_paddle(
    WINDOW_WIDTH - PADDLE_HIT_INSET + (PADDLE_DIMENSIONS.x * 0.5f),
    WINDOW_HEIGHT * 0.5f,
    PADDLE_DIMENSIONS.x,
    PADDLE_DIMENSIONS.y
  );

  /* Playfield collision edges */
  const struct edge_collider colliders[] = {
    /* Bottom collider */ make_edge_collider(PADDLE_HIT_INSET, 0, WINDOW_WIDTH - PADDLE_HIT_INSET, 0, NULL),
    /* Right collider  */ make_edge_collider(WINDOW_WIDTH - PADDLE_HIT_INSET, 0, WINDOW_WIDTH - PADDLE_HIT_INSET, WINDOW_HEIGHT, &paddle_right),
    /* Top collider    */ make_edge_collider(WINDOW_WIDTH - PADDLE_HIT_INSET, WINDOW_HEIGHT, PADDLE_HIT_INSET, WINDOW_HEIGHT, NULL),
    /* Left collider   */ make_edge_collider(PADDLE_HIT_INSET, WINDOW_HEIGHT, PADDLE_HIT_INSET, 0, &paddle_left)
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
            case SDLK_RSHIFT:
              paddle_right_up_pressed = PONG_TRUE;
              break;
            case SDLK_RCTRL:
              paddle_right_down_pressed = PONG_TRUE;
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
            case SDLK_RSHIFT:
              paddle_right_up_pressed = PONG_FALSE;
              break;
            case SDLK_RCTRL:
              paddle_right_down_pressed = PONG_FALSE;
              break;
          }
          break;
        default:
          break;
      }
    }

    /* Integrate scene objects */
      /* Paddles */
      if (paddle_left_up_pressed) paddle_left.position.y += PADDLE_PIXELS_PER_SECOND * dts;
      if (paddle_left_down_pressed) paddle_left.position.y -= PADDLE_PIXELS_PER_SECOND * dts;

      if (paddle_right_up_pressed) paddle_right.position.y += PADDLE_PIXELS_PER_SECOND * dts;
      if (paddle_right_down_pressed) paddle_right.position.y -= PADDLE_PIXELS_PER_SECOND * dts;

      /* Cap vertical paddle movement */
      if (paddle_left.position.y + paddle_left.dimensions.y * 0.5f >= WINDOW_HEIGHT) 
        paddle_left.position.y = WINDOW_HEIGHT - (paddle_left.dimensions.y * 0.5f);
      if (paddle_left.position.y - paddle_left.dimensions.y * 0.5f <= 0.0f) 
        paddle_left.position.y = paddle_left.dimensions.y * 0.5f;

      if (paddle_right.position.y + paddle_right.dimensions.y * 0.5f >= WINDOW_HEIGHT) 
        paddle_right.position.y = WINDOW_HEIGHT - (paddle_right.dimensions.y * 0.5f);
      if (paddle_right.position.y - paddle_right.dimensions.y * 0.5f <= 0.0f) 
        paddle_right.position.y = paddle_right.dimensions.y * 0.5f;

      const struct region2Df region_paddle_left = {
        { paddle_left.position.x - paddle_left.dimensions.x * 0.5f, paddle_left.position.y - paddle_left.dimensions.y * 0.5f },
        { paddle_left.position.x + paddle_left.dimensions.x * 0.5f, paddle_left.position.y + paddle_left.dimensions.y * 0.5f }
      };

      const struct region2Df region_paddle_right = {
        { paddle_right.position.x - paddle_right.dimensions.x * 0.5f, paddle_right.position.y - paddle_right.dimensions.y * 0.5f },
        { paddle_right.position.x + paddle_right.dimensions.x * 0.5f, paddle_right.position.y + paddle_right.dimensions.y * 0.5f }
      };

      /* Compute list of ball corner positions */
      const struct region2Df region_ball = {
        { ball.position.x - ball.diameter * 0.5f, ball.position.y - ball.diameter * 0.5f },
        { ball.position.x + ball.diameter * 0.5f, ball.position.y + ball.diameter * 0.5f }
      };

      struct vec2f ball_corners[] = {
        { region_ball.min.x, region_ball.min.y },
        { region_ball.max.x, region_ball.min.y },
        { region_ball.max.x, region_ball.max.y },
        { region_ball.min.x, region_ball.max.y }
      };

      /* Integrate the ball in the scene */
      while (1)
      {
        const struct edge_collider * p_earliest_collider = NULL;
        float earliest_impact_time;
        for (int collider_index = 0; collider_index < collider_count; collider_index++)
        {
          const struct edge_collider * const p_collider = colliders + collider_index;

          /* Get ball point closes to current collider surface */
          const struct vec2f * p_closest_corner = NULL;
          float closest_corner_distance = 10000000.0f;
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
          if (impact_time < 0.0f || impact_time > 1.0f)
            continue;

          /* Keep track of earliest collision */
          if (p_earliest_collider == NULL || impact_time < earliest_impact_time)
          {
            p_earliest_collider = p_collider;
            earliest_impact_time = impact_time;
          }
        }

        /* Check collision on frame */
        if (p_earliest_collider == NULL)
        {
          /* No collision - Fully integrate the ball velocity */
          ball.position.x += ball.velocity.x * dts;
          ball.position.y += ball.velocity.y * dts;

          /* Reset velocity magnitude */
          ball.velocity = vec2f_normalize(ball.velocity);
          ball.velocity = vec2f_scale(ball.velocity, BALL_SPEED_PIXELS_PER_SECOND);

          /* Done integrating within this frame */
          break;
        }
        else
        {
          /* Collision - Move the ball to the impact surface */
          struct vec2f scaled_ball_velocity = vec2f_scale(ball.velocity, dts);
          ball.position.x += scaled_ball_velocity.x * earliest_impact_time;
          ball.position.y += scaled_ball_velocity.y * earliest_impact_time;

          /* Check if the associated paddle is hit on the surface, if any */
          pong_bool_te paddle_missed_ball = PONG_FALSE;
          if (p_earliest_collider->p_associated_paddle != NULL)
          {
            /* Project paddle extends onto the edge */
            struct region2Df region_paddle = region_for_paddles(p_earliest_collider->p_associated_paddle);
            struct range2f paddle_surface_range = project_region_onto_edge(&region_paddle, p_earliest_collider);

            struct region2Df region_ball = region_for_ball(&ball);
            struct range2f ball_surface_range = project_region_onto_edge(&region_ball, p_earliest_collider);

            /* Deflect ball only if both extends overlap */
            paddle_missed_ball = (
              ball_surface_range.max < paddle_surface_range.min ||
              ball_surface_range.min > paddle_surface_range.max
            ) ? PONG_TRUE : PONG_FALSE;
          }

          /* Deflect the velocity when the paddle was hit or the collider has no paddle associated */
          if (!paddle_missed_ball)
          {
            /* TODO-GS: Compute direction based on where the ball hit the paddle */

            ball.velocity.x = ball.velocity.x + (2.0f * fabs(ball.velocity.x) * p_earliest_collider->surface_normal.x);
            ball.velocity.y = ball.velocity.y + (2.0f * fabs(ball.velocity.y) * p_earliest_collider->surface_normal.y);
          }

          /* Scale the deflected velocity to the time left in the frame */
          const float integration_time_left = 1.0f - earliest_impact_time;
          ball.velocity = vec2f_scale(ball.velocity, integration_time_left);
        }
      }

    /* Ball respawning and scoring - TODO-GS: Random spawn direction and vertical position */
    const struct region2Df region_ball_integrated = region_for_ball(&ball);
    if (region_ball_integrated.min.x > WINDOW_WIDTH)
    {
      /* Left paddle scored - Right player is up next */
      score_paddle_left++;
      set_ball_velocity(&ball, biased_random_ball_velocity(-1));
      ball.position = PLAYFIELD_CENTER;
    }
    if (region_ball_integrated.max.x < 0)
    {
      /* Right paddle scored */
      score_paddle_right++;
      set_ball_velocity(&ball, biased_random_ball_velocity(1));
      ball.position = PLAYFIELD_CENTER;
    }

    /* Clear scene */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Render scene */
      /* Playfield divider */
      const int FIELD_DIV_LENGTH = WINDOW_HEIGHT / 20;
      const int FIELD_DIV_COUNT = 12;
      const int FIELD_DIV_SPACE_COUNT = FIELD_DIV_COUNT - 1;
      const int FIELD_DIV_SPACE_LENGTH = (WINDOW_HEIGHT - (FIELD_DIV_COUNT * FIELD_DIV_LENGTH)) / FIELD_DIV_SPACE_COUNT;
      const int FIELD_DIV_THICKNESS = 4;
      const struct range2f FIELD_RANGE_HORI = {
        (WINDOW_WIDTH * 0.5f) - (FIELD_DIV_THICKNESS * 0.5f),
        (WINDOW_WIDTH * 0.5f) + (FIELD_DIV_THICKNESS * 0.5f)
      };
      for(int div_index = 0; div_index < FIELD_DIV_COUNT; div_index++)
      {
        const float DIV_BASE_HEIGHT = div_index * (FIELD_DIV_LENGTH + FIELD_DIV_SPACE_LENGTH);
        batcher_color(255, 255, 255, 50);
        batcher_quadf(
          FIELD_RANGE_HORI.min, DIV_BASE_HEIGHT,
          FIELD_RANGE_HORI.max, DIV_BASE_HEIGHT + FIELD_DIV_LENGTH
        );  
      }

      /* Paddles */
      batcher_color(255, 0, 0, 255);
      batcher_quadf(
        region_paddle_left.min.x, region_paddle_left.min.y,
        region_paddle_left.max.x, region_paddle_left.max.y
      );
      batcher_color(0, 255, 0, 255);
      batcher_quadf(
        region_paddle_right.min.x, region_paddle_right.min.y,
        region_paddle_right.max.x, region_paddle_right.max.y
      );

      /* Ball */
      batcher_color(255, 255, 255, 255);
      batcher_quadf(
        ball.position.x - ball.diameter * 0.5f,
        ball.position.y - ball.diameter * 0.5f,
        ball.position.x + ball.diameter * 0.5f,
        ball.position.y + ball.diameter * 0.5f
      );

      /* Scores */
      batcher_color(50, 150, 250, 255);
      const char score_text[SCORE_TEXT_MAX_LENGTH];
      snprintf(score_text, SCORE_TEXT_MAX_LENGTH, "%d", score_paddle_left);
      batcher_text(score_text, WINDOW_WIDTH * 0.2f, WINDOW_HEIGHT - 50, 9 * 5);
      snprintf(score_text, SCORE_TEXT_MAX_LENGTH, "%d", score_paddle_right);
      batcher_text(score_text, WINDOW_WIDTH * 0.75f, WINDOW_HEIGHT - 50, 9 * 5);

    /* Render batches */
    batcher_render();

    /* TODO-GS: Remove debug rendering */
    for (int collider_index = 0; collider_index < collider_count; collider_index++)
    {
      break;
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
  audio_player_cleanup();
  SDL_DestroyWindow(p_window);
  SDL_Quit();

  /* TODO-GS: Return to OS successfully */
  return 0;
}