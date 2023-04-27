#ifndef SCREEN_STATE_MACHINE_H
#define SCREEN_STATE_MACHINE_H

/* Includes */
#include <pong_bool.h>
#include <screen.h>

/* Function prototypes */
pong_bool_te screen_state_machine_initialize(enum screen_type initial_screen_type);
pong_bool_te screen_state_machine_tick(void);

#endif