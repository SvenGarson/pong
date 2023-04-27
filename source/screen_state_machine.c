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
void screen_state_machine_run(struct screen initial_screen)
{
  /* Specify screen type to screen instance mapping */
  struct screen * p_screen_type_instance_list = malloc(sizeof(struct screen) * SCREEN_TYPE_COUNT);
  if (p_screen_type_instance_list == NULL)
  {
    fprintf(stderr, "\n[Screen state machine] Could not allocate space for screen type instances");
    return;
  }

  p_screen_type_instance_list[SCREEN_TYPE_MAIN_MENU] = screen_main_menu_make();
  p_screen_type_instance_list[SCREEN_TYPE_PONG] = screen_pong_make();

	/* Kick of with the provided screen and initialize it */
	struct screen active_screen = initial_screen;
  active_screen.p_initialize();

	/* Execute screen callback cycle until none requested */
	while(1)
  {
    /* Integrate and determine whether another screen is requested */
    /* TODO-GS: Pass the actual SDL dt in here */
    active_screen.p_integrate(0.016, screen_change_request);

    /* Render the active screen */
    active_screen.p_render();

    /* Change screens if necessary */
    if (screen_change_requested)
    {
      /* Reset request mechanism */
      screen_change_requested = PONG_FALSE;


      /* Cleanup the active screen */
      active_screen.p_cleanup();

      /* Detect when to exit the state machine */
      if (
        !screen_type_is_valid(latest_screen_type_requested) ||
        latest_screen_type_requested == SCREEN_TYPE_QUIT
      )
      {
        /* Stop processing screens */
      	break;
      }

      /* Switch to the new screen type and initialize before usage */
      /* TODO-GS: Get an instance from the register map + range check */
      active_screen = p_screen_type_instance_list[latest_screen_type_requested];
      active_screen.p_initialize();
    }
  }
}
