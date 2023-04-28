/* Includes */
#include <screen_state_machine.h>
#include <pong_bool.h>
#include <stdlib.h>
#include <stdio.h>
#include <screen_main_menu.h>
#include <screen_pong.h>

/* Private state */
static pong_bool_te screen_change_requested = PONG_FALSE;
static enum screen_type latest_screen_type_requested;
struct screen active_screen;
struct screen * p_screen_type_instance_list;

/* Callbacks */
static void screen_change_request(enum screen_type requested_screen_type)
{
  screen_change_requested = PONG_TRUE;
  latest_screen_type_requested = requested_screen_type;
}

/* Private helpers */
pong_bool_te screen_type_is_valid(enum screen_type screen_type)
{
  return (screen_type >= 0 && screen_type < SCREEN_TYPE_COUNT) ? PONG_TRUE : PONG_FALSE;
}

/* Function definitions */
pong_bool_te screen_state_machine_initialize(enum screen_type initial_screen_type)
{
  /* Prepare the state machine and report setup success */
  /* Check initial screen type */
  if (!screen_type_is_valid(initial_screen_type) || initial_screen_type == SCREEN_TYPE_QUIT)
    return PONG_FALSE;

  /* Specify screen type to screen instance mapping */
  p_screen_type_instance_list = malloc(sizeof(struct screen) * SCREEN_TYPE_COUNT);
  if (p_screen_type_instance_list == NULL)
  {
    fprintf(stderr, "\n[Screen state machine] Could not allocate space for screen type instances");
    return PONG_FALSE;
  }

  p_screen_type_instance_list[SCREEN_TYPE_MAIN_MENU] = screen_main_menu_make();
  p_screen_type_instance_list[SCREEN_TYPE_PONG] = screen_pong_make();

  /* Kick of with the provided screen and initialize it */
  active_screen = p_screen_type_instance_list[initial_screen_type];
  active_screen.p_initialize();

  return PONG_TRUE;
}

pong_bool_te screen_state_machine_tick
(
  double dts,
  const struct gameplay_dependencies_input * p_input,
  const struct gameplay_dependencies_batcher * p_batcher,
  const struct gameplay_dependencies_audio * p_audio
)
{
  /* Integrate and determine whether another screen is requested */
  active_screen.p_integrate(dts, p_input, p_batcher, p_audio, screen_change_request);

  /* Render the active screen */
  active_screen.p_render(p_batcher);

  /* Contine when not screen change is was requested */
  if (!screen_change_requested)
    return PONG_TRUE;

  /* Reset request mechanism */
  screen_change_requested = PONG_FALSE;

  /* Cleanup the active screen */
  active_screen.p_cleanup();

  /* Requested screen type is invalid */
  if (!screen_type_is_valid(latest_screen_type_requested))
  {
    fprintf(stderr, "\n[Screen state machine] Invalid screen type requested");
    return PONG_FALSE;
  }

  if (latest_screen_type_requested == SCREEN_TYPE_QUIT)
  {
    /* Requested to close the screen state machine */
    return PONG_FALSE;
  }

  /* Switch to the new screen type and initialize before usage */
  active_screen = p_screen_type_instance_list[latest_screen_type_requested];
  active_screen.p_initialize();

  /* Keep ticking the state machine */
  return PONG_TRUE;
}
