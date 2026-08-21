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

#include <stdlib.h>

#include "config.h"
#include "elite.h"
#include "vector.h"
#include "planet.h"
#include "shipdata.h"
#include "game_state.h"

struct ship_data *ship_list[NO_OF_SHIPS + 1] =
{
	nullptr,
	&missile_data,
	&coriolis_data,
	&esccaps_data,
	&alloy_data,
	&cargo_data,
	&boulder_data,
	&asteroid_data,
	&rock_data,
	&orbit_data,
	&transp_data,
	&cobra3a_data,
	&pythona_data,
	&boa_data,
	&anacnda_data,
	&hermit_data,
	&viper_data,
	&sidewnd_data,
	&mamba_data,
	&krait_data,
	&adder_data,
	&gecko_data,
	&cobra1_data,
	&worm_data,
	&cobra3b_data,
	&asp2_data,
	&pythonb_data,
	&ferdlce_data,
	&moray_data,
	&thargoid_data,
	&thargon_data,
	&constrct_data,
	&cougar_data,
	&dodec_data
};

void restore_saved_commander (void)
{
	game_state_restore_saved_commander ();
}

