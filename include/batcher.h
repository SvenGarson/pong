#ifndef BATCHER_H
#define BATCHER_H

/* Includes */
#include <pong_bool.h>

/* Batcher function definitions */
pong_bool_te batcher_initialize(void);
void batcher_cleanup(void);
void batcher_text
(
  const char * p_text,
  int base_x,
  int base_y,
  int font_height
);
void batcher_quadf
(
  float min_x,
  float min_y,
  float max_x,
  float max_y
);

#endif
