/* Includes */
#include <window_context.h>
#include <stdio.h>
#include <screen_state_machine.h>
#include <gameplay_dependencies.h>

/* Callbacks */
pong_bool_te gameplay_tick_callback
(
  double dts,
  const struct gameplay_dependencies_input * p_input,
  const struct gameplay_dependencies_batcher * p_batcher,
  const struct gameplay_dependencies_audio * p_audio,
  const struct gameplay_dependencies_windowing * p_windowing
)
{
  /*
      Tick the state machine for every game play tick and
      quit the gameloop when the state machine is done
  */
  return screen_state_machine_tick(dts, p_input, p_batcher, p_audio, p_windowing);
}

pong_bool_te initialize_callback(const struct gameplay_dependencies_windowing * p_windowing)
{
  /* Initialize the screen state machine */
  if (screen_state_machine_initialize(SCREEN_TYPE_PONG, p_windowing) == PONG_FALSE)
  {
    fprintf(stderr, "\n[Pong] Could not initialize the screen state machine");
    return PONG_FALSE;
  }

  return PONG_TRUE;
}

/* Entry point */
int main(void)
{
  /* Create usable window and resources */
  if (window_context_initialize(initialize_callback) == PONG_FALSE)
  {
    fprintf(stderr, "\n[Pong] Could not create window context");
    return -1;
  }

  /* Run the loop and start ticking the callback - Cleans up after itself */
  window_context_run(gameplay_tick_callback);

  /* Return to OS successfully */
  return 0;
}