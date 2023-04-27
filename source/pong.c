/* Includes */
#include <window_context.h>
#include <stdio.h>
#include <screen_state_machine.h>

/* Callbacks */
void gameplay_tick_callback(double dt)
{
  /*
      Tick the state machine for every game play tick and
      quit the gameloop when the state machine is done
  */
  return screen_state_machine_tick();
}

/* Entry point */
int main(void)
{
  /* Create usable window and resources */
  if (window_context_initialize() == PONG_FALSE)
  {
    fprintf(stderr, "\n[Pong] Could not create window context");
    return -1;
  }

  /* Initialize the screen state machine */
  if (screen_state_machine_initialize(SCREEN_TYPE_MAIN_MENU) == PONG_FALSE)
  {
    fprintf(stderr, "\n[Pong] Could not initialize the screen state machine");
    return -1;
  }

  /* Run the loop and start ticking the callback - Cleans up after itself */
  window_context_run(gameplay_tick_callback);

  /* Return to OS successfully */
  return 0;
}