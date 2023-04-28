#ifndef SCREEN_STATE_MACHINE_H
#define SCREEN_STATE_MACHINE_H

/* Includes */
#include <pong_bool.h>
#include <screen.h>
#include <gameplay_dependencies.h>

/* Function prototypes */
pong_bool_te screen_state_machine_initialize
(
  enum screen_type initial_screen_type,
  const struct gameplay_dependencies_windowing * p_windowing
);
pong_bool_te screen_state_machine_tick
(
	double dts,
  const struct gameplay_dependencies_input * p_input,
  const struct gameplay_dependencies_batcher * p_batcher,
  const struct gameplay_dependencies_audio * p_audio,
  const struct gameplay_dependencies_windowing * p_windowing
);

#endif