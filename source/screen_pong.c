/* Includes */
#include <screen_pong.h>
#include <range2f.h>
#include <vec2f.h>
#include <region2Df.h>
#include <input_mapper.h>
#include <audio_player.h>

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

/* Defines */
#define SCORE_TEXT_MAX_LENGTH (16)

/* Constants */
static const float PADDLE_PIXELS_PER_SECOND = 500.0f;
static const float BALL_SPEED_PIXELS_PER_SECOND = 350.0f;
const struct vec2f PADDLE_DIMENSIONS = { 5.0f, 80.0f };
const int PADDLE_HIT_INSET = 50;

/* Private helper function prototypes */
struct ball make_ball(float center_x, float center_y, float diameter, float velocity_x, float velocity_y);
struct paddle make_paddle(float center_x, float center_y, float dimension_x, float dimension_y);
double degrees_to_radians(double degrees);
struct vec2f edge_tangent(struct vec2f a, struct vec2f b);
struct vec2f left_normal(struct vec2f tangent);
struct vec2f biased_random_ball_velocity(int horizontal_direction);
void set_ball_velocity(struct ball * p_ball, struct vec2f new_velocity);
float flt_list_min(const float * p_float_list, size_t entries);
float flt_list_max(const float * p_float_list, size_t entries);
struct range2f project_region_onto_edge(const struct region2Df * p_region, const struct edge_collider * p_collider);
struct edge_collider make_edge_collider(float ax, float ay, float bx, float by, struct paddle * p_paddle);
struct region2Df region_for_paddles(struct paddle * p_paddle);
struct region2Df region_for_ball(struct ball * p_ball);

/* Private state */
int score_paddle_left = 0;
int score_paddle_right = 0;
struct ball ball;
struct paddle paddle_left;
struct paddle paddle_right;
struct edge_collider colliders[4];
int collider_count;

/* Function definitions */
static void screen_initialize
(
	const struct gameplay_dependencies_windowing * p_windowing
)
{
  /* Reset scores */
  score_paddle_right = score_paddle_left = 0;

  /* Make the ball */
  ball = make_ball(
    p_windowing->window_width / 2.0f,
    p_windowing->window_height / 2.0f,
    30.0f,
    BALL_SPEED_PIXELS_PER_SECOND,
    BALL_SPEED_PIXELS_PER_SECOND
  );

  /* Randomize ball direction for the first spawn */
  set_ball_velocity(&ball, biased_random_ball_velocity(0));

  /* Paddles */
  paddle_left = make_paddle(
    PADDLE_HIT_INSET - (PADDLE_DIMENSIONS.x * 0.5f),
    p_windowing->window_height * 0.5f,
    PADDLE_DIMENSIONS.x,
    PADDLE_DIMENSIONS.y
  );

  paddle_right = make_paddle(
    p_windowing->window_width - PADDLE_HIT_INSET + (PADDLE_DIMENSIONS.x * 0.5f),
    p_windowing->window_height * 0.5f,
    PADDLE_DIMENSIONS.x,
    PADDLE_DIMENSIONS.y
  );

  /* Playfield collision edges */
  colliders[0] = make_edge_collider(PADDLE_HIT_INSET, 0, p_windowing->window_width - PADDLE_HIT_INSET, 0, NULL);
  colliders[1] = make_edge_collider(p_windowing->window_width - PADDLE_HIT_INSET, 0, p_windowing->window_width - PADDLE_HIT_INSET, p_windowing->window_height, &paddle_right);
  colliders[2] = make_edge_collider(p_windowing->window_width - PADDLE_HIT_INSET, p_windowing->window_height, PADDLE_HIT_INSET, p_windowing->window_height, NULL);
  colliders[3] = make_edge_collider(PADDLE_HIT_INSET, p_windowing->window_height, PADDLE_HIT_INSET, 0, &paddle_left);
  collider_count = sizeof(colliders) / sizeof(struct edge_collider);
}

static void screen_integrate
(
	double dt,
	const struct gameplay_dependencies_input * p_input,
  const struct gameplay_dependencies_batcher * p_batcher,
  const struct gameplay_dependencies_audio * p_audio,
  const struct gameplay_dependencies_windowing * p_windowing,
	screen_callback_change_request_tf change_request
)
{
  /* Back to main menu */
  if (p_input->key_pressed(INPUT_MAPPER_KEY_TYPE_MENU_RETURN))
  {
    p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_MENU_RETURN);
    change_request(SCREEN_TYPE_MAIN_MENU);
  }

  /* Left paddles */
  if (p_input->key_held(INPUT_MAPPER_KEY_TYPE_LEFT_PADDLE_UP)) paddle_left.position.y += PADDLE_PIXELS_PER_SECOND * dt;
  if (p_input->key_held(INPUT_MAPPER_KEY_TYPE_LEFT_PADDLE_DOWN)) paddle_left.position.y -= PADDLE_PIXELS_PER_SECOND * dt;

  /* Right paddles */
  if (p_input->key_held(INPUT_MAPPER_KEY_TYPE_RIGHT_PADDLE_UP)) paddle_right.position.y += PADDLE_PIXELS_PER_SECOND * dt;
  if (p_input->key_held(INPUT_MAPPER_KEY_TYPE_RIGHT_PADDLE_DOWN)) paddle_right.position.y -= PADDLE_PIXELS_PER_SECOND * dt;

  /* Cap vertical paddle movement */
  if (paddle_left.position.y + paddle_left.dimensions.y * 0.5f >= p_windowing->window_height) 
    paddle_left.position.y = p_windowing->window_height - (paddle_left.dimensions.y * 0.5f);
  if (paddle_left.position.y - paddle_left.dimensions.y * 0.5f <= 0.0f) 
    paddle_left.position.y = paddle_left.dimensions.y * 0.5f;

  if (paddle_right.position.y + paddle_right.dimensions.y * 0.5f >= p_windowing->window_height) 
    paddle_right.position.y = p_windowing->window_height - (paddle_right.dimensions.y * 0.5f);
  if (paddle_right.position.y - paddle_right.dimensions.y * 0.5f <= 0.0f) 
    paddle_right.position.y = paddle_right.dimensions.y * 0.5f;

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

      /* Scale the ball velocity to the time-step */
      struct vec2f scaled_ball_velocity = vec2f_scale(ball.velocity, dt);

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
      ball.position.x += ball.velocity.x * dt;
      ball.position.y += ball.velocity.y * dt;

      /* Reset velocity magnitude so the ball keeps a constant speed after integration */
      ball.velocity = vec2f_normalize(ball.velocity);
      ball.velocity = vec2f_scale(ball.velocity, BALL_SPEED_PIXELS_PER_SECOND);

      /* Done integrating - Up to the next frame */
      break;
    }
    else
    {
      /* Surface hit sound */
      p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_PADDLE_HIT);

      /* Collision - Move the ball to the impact surface */
      struct vec2f scaled_ball_velocity = vec2f_scale(ball.velocity, dt);
      ball.position.x += scaled_ball_velocity.x * earliest_impact_time;
      ball.position.y += scaled_ball_velocity.y * earliest_impact_time;

      /* React to paddle and egde collider collisions differently */
      if (p_earliest_collider->p_associated_paddle != NULL)
      {
        /* Potential paddle collision detection */
        struct region2Df region_paddle = region_for_paddles(p_earliest_collider->p_associated_paddle);
        struct range2f paddle_surface_range = project_region_onto_edge(&region_paddle, p_earliest_collider);

        struct region2Df region_ball = region_for_ball(&ball);
        struct range2f ball_surface_range = project_region_onto_edge(&region_ball, p_earliest_collider);

        const pong_bool_te ball_hit_paddle = !(
          ball_surface_range.max < paddle_surface_range.min ||
          ball_surface_range.min > paddle_surface_range.max
        ) ? PONG_TRUE : PONG_FALSE;

        if (ball_hit_paddle)
        {
          /* Deflect ball in a way that the player can influence the trajectory - For not perfect deflection */
          ball.velocity.x = ball.velocity.x + (2.0f * fabs(ball.velocity.x) * p_earliest_collider->surface_normal.x);
          ball.velocity.y = ball.velocity.y + (2.0f * fabs(ball.velocity.y) * p_earliest_collider->surface_normal.y);
        }
      }
      else
      {
        /* Edge collision detection - Perfectly deflect the ball velocity on non-paddle surfaces */
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
   const struct vec2f PLAYFIELD_CENTER = {
    p_windowing->window_width * 0.5f,
    p_windowing->window_height * 0.5f
   };
   if (region_ball_integrated.min.x > p_windowing->window_width)
   {
     /* Left paddle scored - Right player is up next */
    p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_SCORE);
     score_paddle_left++;
     set_ball_velocity(&ball, biased_random_ball_velocity(-1));
     ball.position = PLAYFIELD_CENTER;
   }
   if (region_ball_integrated.max.x < 0)
   {
     /* Right paddle scored */
     p_audio->play_sound_effect(AUDIO_PLAYER_SFX_TYPE_SCORE);
     score_paddle_right++;
     set_ball_velocity(&ball, biased_random_ball_velocity(1));
     ball.position = PLAYFIELD_CENTER;
   }  
}

static void screen_render
(
	const struct gameplay_dependencies_batcher * p_batcher,
	const struct gameplay_dependencies_windowing * p_windowing
)
{
  /* Background */
  p_batcher->color(25, 75, 75, 255);
  p_batcher->quadf(0, 0, 800, 600);

  /* Playfield divider */
  const int FIELD_DIV_LENGTH = p_windowing->window_height / 20;
  const int FIELD_DIV_COUNT = 12;
  const int FIELD_DIV_SPACE_COUNT = FIELD_DIV_COUNT - 1;
  const int FIELD_DIV_SPACE_LENGTH = (p_windowing->window_height - (FIELD_DIV_COUNT * FIELD_DIV_LENGTH)) / FIELD_DIV_SPACE_COUNT;
  const int FIELD_DIV_THICKNESS = 4;
  const struct range2f FIELD_RANGE_HORI = {
    (p_windowing->window_width * 0.5f) - (FIELD_DIV_THICKNESS * 0.5f),
    (p_windowing->window_width * 0.5f) + (FIELD_DIV_THICKNESS * 0.5f)
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

  /* Determine regions */
  const struct region2Df region_paddle_left = {
    { paddle_left.position.x - paddle_left.dimensions.x * 0.5f, paddle_left.position.y - paddle_left.dimensions.y * 0.5f },
    { paddle_left.position.x + paddle_left.dimensions.x * 0.5f, paddle_left.position.y + paddle_left.dimensions.y * 0.5f }
  };

  const struct region2Df region_paddle_right = {
    { paddle_right.position.x - paddle_right.dimensions.x * 0.5f, paddle_right.position.y - paddle_right.dimensions.y * 0.5f },
    { paddle_right.position.x + paddle_right.dimensions.x * 0.5f, paddle_right.position.y + paddle_right.dimensions.y * 0.5f }
  };

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
  char score_text[SCORE_TEXT_MAX_LENGTH];
  snprintf(score_text, SCORE_TEXT_MAX_LENGTH, "%d", score_paddle_left);
  batcher_text(score_text, p_windowing->window_width * 0.2f, p_windowing->window_height - 50, 9 * 5);
  snprintf(score_text, SCORE_TEXT_MAX_LENGTH, "%d", score_paddle_right);
  batcher_text(score_text, p_windowing->window_width * 0.75f, p_windowing->window_height - 50, 9 * 5);
}

static void screen_cleanup(void)
{
}

struct screen screen_pong_make(void)
{
	return screen_make
	(
		SCREEN_TYPE_PONG,
		screen_initialize,
		screen_integrate,
		screen_render,
		screen_cleanup
	);
}

/* Private helper function definitions */
struct ball make_ball(float center_x, float center_y, float diameter, float velocity_x, float velocity_y)
{
  struct ball ball;

  ball.position = (struct vec2f){ center_x, center_y };
  ball.diameter = diameter;
  ball.velocity = (struct vec2f){ velocity_x, velocity_y };

  return ball;
}

struct paddle make_paddle(float center_x, float center_y, float dimension_x, float dimension_y)
{
  struct paddle paddle;

  paddle.position = (struct vec2f){ center_x, center_y };
  paddle.dimensions = (struct vec2f){ dimension_x, dimension_y };

  return paddle;  
};

double degrees_to_radians(double degrees)
{
  return degrees * (M_PI / 180.0);
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

struct range2f project_region_onto_edge(const struct region2Df * p_region, const struct edge_collider * p_collider)
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