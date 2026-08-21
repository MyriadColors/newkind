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

#ifndef ELITE_H
#define ELITE_H

#include <stdint.h>
#include <stdbool.h>

#include "planet.h"
#include "trade.h"
#include "game_state.h"

#define SCR_INTRO_ONE		1
#define SCR_INTRO_TWO		2
#define SCR_GALACTIC_CHART	3
#define SCR_SHORT_RANGE		4
#define	SCR_PLANET_DATA		5
#define SCR_MARKET_PRICES	6
#define SCR_CMDR_STATUS		7
#define SCR_FRONT_VIEW		8
#define SCR_REAR_VIEW		9
#define SCR_LEFT_VIEW		10
#define SCR_RIGHT_VIEW		11
#define SCR_BREAK_PATTERN	12
#define SCR_INVENTORY		13
#define SCR_EQUIP_SHIP		14
#define SCR_OPTIONS			15
#define SCR_LOAD_CMDR		16
#define SCR_SAVE_CMDR		17
#define SCR_QUIT			18
#define SCR_GAME_OVER		19
#define SCR_SETTINGS		20
#define SCR_ESCAPE_POD		21

extern struct ship_data *ship_list[];

void restore_saved_commander (void);

#endif
