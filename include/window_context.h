#ifndef WINDOW_CONTEXT_H
#define WINDOW_CONTEXT_H

/* Includes */
#include <pong_bool.h>
#include <gameplay_dependencies.h>

/* Datatypes */
typedef pong_bool_te (* window_context_initialize_tf)
(
  const struct gameplay_dependencies_windowing * p_windowing
);
typedef pong_bool_te (* window_context_gameplay_tick_tf)
(
	double dts,
  const struct gameplay_dependencies_input * p_input,
  const struct gameplay_dependencies_batcher * p_batcher,
  const struct gameplay_dependencies_audio * p_audio,
  const struct gameplay_dependencies_windowing * p_windowing
);

/* Function prototypes */
pong_bool_te window_context_initialize(window_context_initialize_tf p_callback_initialize);
pong_bool_te window_context_run(window_context_gameplay_tick_tf p_callback_tick);

#endif