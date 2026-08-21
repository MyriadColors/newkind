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

/*
 * missions.c
 *
 * Code to handle the special missions.
 */

#include <stdlib.h>
#include <string.h>
 
#include "config.h"
#include "elite.h"
#include "gfx.h"
#include "vector.h"
#include "space.h"
#include "planet.h"
#include "main.h"
#include "missions.h" 
#include "keyboard.h"
 

const char *mission1_brief_a =
	"Greetings Commander, I am Captain Curruthers of "
	"Her Majesty's Space Navy and I beg a moment of your "
	"valuable time.  We would like you to do a little job "
	"for us.  The ship you see here is a new model, the "
	"Constrictor, equiped with a top secret new shield "
	"generator.  Unfortunately it's been stolen.";

const char *mission1_brief_b =
	"It went missing from our ship yard on Xeer five months ago "
	"and was last seen at Reesdice. Your mission should you decide "
	"to accept it, is to seek and destroy this ship. You are "
	"cautioned that only Military Lasers will get through the new "
	"shields and that the Constrictor is fitted with an E.C.M. "
	"System. Good Luck, Commander. ---MESSAGE ENDS.";

const char *mission1_brief_c =
	"It went missing from our ship yard on Xeer five months ago "
	"and is believed to have jumped to this galaxy. "
	"Your mission should you decide to accept it, is to seek and "
	"destroy this ship. You are cautioned that only Military Lasers "
	"will get through the new shields and that the Constrictor is "
	"fitted with an E.C.M. System. Good Luck, Commander. ---MESSAGE ENDS.";

const char *mission1_debrief =
	"There will always be a place for you in Her Majesty's Space Navy. "
	"And maybe sooner than you think... ---MESSAGE ENDS.";

const char *mission1_pdesc[] =
{
	"THE CONSTRICTOR WAS LAST SEEN AT REESDICE, COMMANDER.",
	"A STRANGE LOOKING SHIP LEFT HERE A WHILE BACK. LOOKED BOUND FOR AREXE.",
	"YEP, AN UNUSUAL NEW SHIP HAD A GALACTIC HYPERDRIVE FITTED HERE, USED IT TOO.",
	"I HEAR A WEIRD LOOKING SHIP WAS SEEN AT ERRIUS.",
	"THIS STRANGE SHIP DEHYPED HERE FROM NOWHERE, SUN SKIMMED AND JUMPED. I HEAR IT WENT TO INBIBE.",
	"ROGUE SHIP WENT FOR ME AT AUSAR. MY LASERS DIDN'T EVEN SCRATCH ITS HULL.",
	"OH DEAR ME YES. A FRIGHTFUL ROGUE WITH WHAT I BELIEVE YOU PEOPLE CALL A LEAD "
		"POSTERIOR SHOT UP LOTS OF THOSE BEASTLY PIRATES AND WENT TO USLERI.",
	"YOU CAN TACKLE THE VICIOUS SCOUNDREL IF YOU LIKE. HE'S AT ORARRA.",
	"THERE'S A REAL DEADLY PIRATE OUT THERE.",
	"BOY ARE YOU IN THE WRONG GALAXY!",
	"COMING SOON: ELITE - DARKNESS FALLS.",				
};

const char *mission2_brief_a =
	"Attention Commander, I am Captain Fortesque of Her Majesty's Space Navy. "
	"We have need of your services again. If you would be so good as to go to "
	"Ceerdi you will be briefed.If succesful, you will be rewarded."
	"---MESSAGE ENDS.";
	
const char *mission2_brief_b =
	"Good Day Commander. I am Agent Blake of Naval Intelligence. As you know, "
	"the Navy have been keeping the Thargoids off your ass out in deep space "
	"for many years now. Well the situation has changed. Our boys are ready "
	"for a push right to the home system of those murderers.";

const char *mission2_brief_c =
	"I have obtained the defence plans for their Hive Worlds. The beetles "
	"know we've got something but not what. If I transmit the plans to our "
	"base on Birera they'll intercept the transmission. I need a ship to "
	"make the run. You're elected. The plans are unipulse coded within "
	"this transmission. You will be paid. Good luck Commander. ---MESSAGE ENDS.";

const char *mission2_debrief =
	"You have served us well and we shall remember. "
	"We did not expect the Thargoids to find out about you."
	"For the moment please accept this Navy Extra Energy Unit as payment. "
	"---MESSAGE ENDS.";

	

const char *mission_planet_desc (struct galaxy_seed planet)
{
	int pnum;
	const SessionState *session = get_session_state();
	const UniverseState *env = get_universe_state();
	const PlayerState *player = get_player_state();

	if (!session->is_docked)
		return nullptr;

	if ((planet.a != env->docked_planet.a) ||
	    (planet.b != env->docked_planet.b) ||
	    (planet.c != env->docked_planet.c) ||
	    (planet.d != env->docked_planet.d) ||
	    (planet.e != env->docked_planet.e) ||
	    (planet.f != env->docked_planet.f))
		return nullptr;
	
	pnum = find_planet_number (planet);
	
	if (player->current.galaxy_number == 0)
	{
		switch (pnum)
		{
			case 150:
				return mission1_pdesc[0];
			
			case 36:
				return mission1_pdesc[1];

			case 28:
				return mission1_pdesc[2];							
		}
	}

	if (player->current.galaxy_number == 1)
	{
		switch (pnum)
		{
			case 32:
			case 68:
			case 164:
			case 220:
			case 106:
			case 16:
			case 162:
			case 3:
			case 107:
			case 26:
			case 192:
			case 184:
			case 5:
				return mission1_pdesc[3];
		
			case 253:
				return mission1_pdesc[4];
			
			case 79:
				return mission1_pdesc[5];

			case 53:
				return mission1_pdesc[6];							

			case 118:
				return mission1_pdesc[7];							

			case 193:
				return mission1_pdesc[8];							
		}
	}

	if ((player->current.galaxy_number == 2) && (pnum == 101))
		return mission1_pdesc[9];							
	
	return nullptr;
}


void constrictor_mission_brief (void)
{
	EliteMatrix rotmat;
	PlayerState *player = get_player_state();
	SessionState *session = get_session_state();
	FlightState *flight = get_flight_state();
	UniverseState *env = get_universe_state();

	player->current.mission = 1;

	session->current_screen = SCR_FRONT_VIEW;

	gfx_clear_display();
	gfx_display_centre_text (10, "INCOMING MESSAGE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_pretty_text (16, 50, 300, 384, mission1_brief_a);
	gfx_display_pretty_text (16, 200, 470, 384,
	      (player->current.galaxy_number == 0) ? mission1_brief_b : mission1_brief_c);
		
	gfx_display_centre_text (330, "Press space to continue.", 140, GFX_COL_GOLD);
		
	clear_universe();
	set_init_matrix (rotmat);
	add_new_ship (SHIP_CONSTRICTOR, 200, 90, 600, rotmat, -127, -127);
	flight->roll = 0;
	flight->climb = 0;
	flight->speed = 0;

	do
	{
		gfx_clear_area (310, 50, 510, 180);
		update_universe ();
		env->objects[0].location.z = 600;
		gfx_update_screen();
		kbd_poll_keyboard();
	} while (!kbd_space_pressed);
}	


void constrictor_mission_debrief (void)
{
	int keyasc;
	PlayerState *player = get_player_state();

	player->current.mission = 3;
	player->current.score += 256;
	player->current.credits += 50000;
	
	gfx_clear_display();
	gfx_display_centre_text (10, "INCOMING MESSAGE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_centre_text (100, "Congratulations Commander!", 140, GFX_COL_GOLD);
	
	gfx_display_pretty_text (116, 132, 400, 384, mission1_debrief);

	gfx_display_centre_text (330, "Press space to continue.", 140, GFX_COL_GOLD);

	gfx_update_screen();
	
	do
	{
		keyasc = kbd_read_key();
	} while (keyasc != ' ');
}


void thargoid_mission_first_brief (void)
{
	int keyasc;
	PlayerState *player = get_player_state();

	player->current.mission = 4;
	
	gfx_clear_display();
	gfx_display_centre_text (10, "INCOMING MESSAGE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_pretty_text (116, 132, 400, 384, mission2_brief_a);

	gfx_display_centre_text (330, "Press space to continue.", 140, GFX_COL_GOLD);

	gfx_update_screen();
	
	do
	{
		keyasc = kbd_read_key();
	} while (keyasc != ' ');
}


void thargoid_mission_second_brief (void)
{
	int keyasc;
	PlayerState *player = get_player_state();

	player->current.mission = 5;
	
	gfx_clear_display();
	gfx_display_centre_text (10, "INCOMING MESSAGE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_pretty_text (16, 50, 300, 384, mission2_brief_b);
	gfx_display_pretty_text (16, 200, 470, 384, mission2_brief_c);

	gfx_draw_sprite (IMG_BLAKE, 352, 46);
	
	gfx_display_centre_text (330, "Press space to continue.", 140, GFX_COL_GOLD);

	gfx_update_screen();
	
	do
	{
		keyasc = kbd_read_key();
	} while (keyasc != ' ');
}


void thargoid_mission_debrief (void)
{
	int keyasc;
	PlayerState *player = get_player_state();

	player->current.mission = 6;
	player->current.score += 256;
	player->current.energy_unit = 2;
	
	gfx_clear_display ();
	gfx_display_centre_text (10, "INCOMING MESSAGE", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_centre_text (100, "Well done Commander.", 140, GFX_COL_GOLD);
	
	gfx_display_pretty_text (116, 132, 400, 384, mission2_debrief);

	gfx_display_centre_text (330, "Press space to continue.", 140, GFX_COL_GOLD);

	gfx_update_screen();
	
	do
	{
		keyasc = kbd_read_key();
	} while (keyasc != ' ');
}



void check_mission_brief (void)
{
	const PlayerState *player = get_player_state();
	const UniverseState *env = get_universe_state();

	if ((player->current.mission == 0) && (player->current.score >= 256) && (player->current.galaxy_number < 2))
	{
		constrictor_mission_brief();
		return;
	}

	if (player->current.mission == 2)
	{
		constrictor_mission_debrief();
		return;
	}

	if ((player->current.mission == 3) && (player->current.score >= 1280) && (player->current.galaxy_number == 2))
	{
		thargoid_mission_first_brief();
		return;
	}

	if ((player->current.mission == 4) && (env->docked_planet.d == 215) && (env->docked_planet.b == 84))
	{
		thargoid_mission_second_brief();
		return;
	}

	if ((player->current.mission == 5) && (env->docked_planet.d == 63) && (env->docked_planet.b == 72))
	{
		thargoid_mission_debrief();
		return;
	}
}

