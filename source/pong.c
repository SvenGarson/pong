/* Includes */
#include <window_context.h>
#include <stdio.h>

/* Callbacks */
void gameplay_tick_callback(double dt)
{
  printf("\nTicked at %f", dt);
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

  /* Run the loop and start ticking the callback */
  window_context_run(gameplay_tick_callback);

  /* Return to OS successfully */
  return 0;
}