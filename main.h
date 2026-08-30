/*
 * Elite - The New Kind.
 *
 * Reverse engineered from the BBC disk version of Elite.
 * Additional material by C.J.Pinder.
 *
 * The original Elite code is (C) I.Bell & D.Braben 1984.
 * This version re-engineered in C by C.J.Pinder 1999-2001.
 *
 * email: <christian@newkind.co.uk>
 *
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>

void info_message (const char *message);
void save_commander_screen (void);
void load_commander_screen (void);
void update_screen (void);

#include "game_state.h"

#define find_input (get_chart_state()->find_input)
#define find_name (get_chart_state()->find_name)
#define cross_timer (get_chart_state()->cross_timer)

#endif
