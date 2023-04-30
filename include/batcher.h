#ifndef BATCHER_H
#define BATCHER_H

/* Includes */
#include <pong_bool.h>
#include <region2Di.h>

/* Batcher function definitions */
pong_bool_te batcher_initialize(void);
void batcher_cleanup(void);
void batcher_color
(
  unsigned char red,
  unsigned char green,
  unsigned char blue,
  unsigned char alpha
);
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
void batcher_render(void);
pong_bool_te batcher_text_region
(
  const char * p_text,
  int base_x,
  int base_y,
  int font_height,
  struct region2Di * p_out_region
);

#endif
