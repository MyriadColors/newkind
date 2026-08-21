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
 * Options.c
 */

#include <stdlib.h>
#include <string.h>

#include "elite.h"
#include "config.h"
#include "gfx.h"
#include "options.h"
#include "main.h"
#include "docked.h"
#include "file.h" 

#include "game_state.h"

static int hilite_item;
 
#define NUM_OPTIONS 4
#define NUM_SETTINGS 14

#define OPTION_BAR_WIDTH	(400)
#define OPTION_BAR_HEIGHT	(15)

struct option
{
	const char *text;
	int docked_only;
};

static const struct option option_list[NUM_OPTIONS] =
{
	{"Save Commander",	1},
	{"Load Commander",	1},
	{"Game Settings",	0},
	{"Quit",			0}	
};

struct setting
{
	const char *name;
	const char *value[6];
};

static const struct setting setting_list[NUM_SETTINGS] =
{
	{"Graphics:",		{"Solid", "Wireframe", "", "", "", ""}},
	{"Anti Alias:",		{"Off", "On", "", "", "", ""}},		
	{"Planet Style:",	{"Wireframe", "Green", "SNES", "Fractal", "", ""}},
	{"Planet Desc.:",	{"BBC", "MSX", "", "", "", ""}},
	{"Instant Dock:",	{"Off", "On", "", "", "", ""}},	
	{"Controls:",		{"Classic", "Modern", "", "", "", ""}},
	{"Mouse Flight:",	{"Direct", "Virtual", "Off", "", "", ""}},
	{"Pitch Mode:",		{"Standard", "Inverted", "", "", "", ""}},
	{"Mouse Sens:",		{"Low", "Medium", "High", "", "", ""}},
	{"Flight Assist:",	{"Off", "On", "", "", "", ""}},
	{"Display Mode:",	{"Maximized", "Fullscreen", "800x600", "1024x768", "1280x720", "1920x1080"}},
	{"Aspect Ratio:",	{"1:1 Square", "4:3 Retro", "16:9 Wide", "Integer", "Stretch", ""}},
	{"Filter:",			{"Point", "Bilinear", "", "", "", ""}},
	{"Save Settings",	{"", "", "", "", "", ""}}
};


void quit_screen (void)
{
	get_session_state()->current_screen = SCR_QUIT;

	gfx_clear_display();
	gfx_display_centre_text (10, "GAME OPTIONS", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_centre_text (175, "QUIT GAME (Y/N)?", 140, GFX_COL_GOLD);		
}





void display_setting_item (int item)
{
	int x,y;
	int v;
	const ConfigState *cfg = get_config_state();

	if (item == (NUM_SETTINGS - 1))
	{
		y = 265;
		gfx_display_centre_text (y, setting_list[item].name, 120, GFX_COL_WHITE);
		return;
	}
	
	switch (item)
	{
		case 0:
			v = cfg->wireframe;
			break;
		
		case 1:
			v = cfg->anti_alias_gfx;
			break;
		
		case 2:
			v = cfg->planet_render_style;
			break;
		
		case 3:
			v = cfg->hoopy_casinos;
			break;
		
		case 4:
			v = cfg->instant_dock;
			break;

		case 5:
			v = cfg->control_scheme;
			break;

		case 6:
			v = cfg->mouse_flight_mode;
			break;

		case 7:
			v = cfg->invert_pitch;
			break;

		case 8:
			v = cfg->mouse_sensitivity;
			break;

		case 9:
			v = cfg->flight_assist;
			break;

		case 10:
			v = cfg->display_mode;
			break;

		case 11:
			v = cfg->aspect_ratio_mode;
			break;

		case 12:
			v = cfg->scaling_filter;
			break;

		default:
			v = 0;
			break;
	}
	
	x = (item & 1) * 250 + 32; 
	y = (item / 2) * 26 + 65;
	
	gfx_display_colour_text (x, y, setting_list[item].name, GFX_COL_WHITE);
	gfx_display_colour_text (x + 120, y, setting_list[item].value[v], GFX_COL_WHITE);
}


void highlight_setting (int item)
{
	int x,y;
	int width;
	
	if ((hilite_item != -1) && (hilite_item != item))
	{
		if (hilite_item == (NUM_SETTINGS - 1))
		{
			x = GFX_X_CENTRE - (OPTION_BAR_WIDTH / 2);
			y = 265;
			width = OPTION_BAR_WIDTH;
		}
		else
		{
			x = (hilite_item & 1) * 250 + 32 + 120; 
			y = (hilite_item / 2) * 26 + 65;
			width = 100;
		}

		gfx_clear_area (x, y, x + width, y + OPTION_BAR_HEIGHT);
		display_setting_item (hilite_item);		
	}

	if (item == (NUM_SETTINGS - 1))
	{
		x = GFX_X_CENTRE - (OPTION_BAR_WIDTH / 2);
		y = 265;
		width = OPTION_BAR_WIDTH;
	}
	else
	{
		x = (item & 1) * 250 + 32 + 120; 
		y = (item / 2) * 26 + 65;
		width = 100;
	}
	
	gfx_draw_rectangle (x, y, x + width, y + OPTION_BAR_HEIGHT, GFX_COL_DARK_RED);
	display_setting_item (item);		
	hilite_item = item;
}



void select_left_setting (void)
{
	if ((hilite_item & 1) != 0 && hilite_item < (NUM_SETTINGS - 1))
		highlight_setting (hilite_item - 1);
}

void select_right_setting (void)
{
	if (((hilite_item & 1) == 0) && (hilite_item < (NUM_SETTINGS - 2)))
		highlight_setting (hilite_item + 1);
}


void select_up_setting (void)
{
	if (hilite_item == (NUM_SETTINGS - 1))
	{
		highlight_setting (NUM_SETTINGS - 2);
		return;
	}

	if (hilite_item > 1)
		highlight_setting (hilite_item - 2);
}


void select_down_setting (void)
{
	if (hilite_item >= (NUM_SETTINGS - 3))
	{
		highlight_setting (NUM_SETTINGS - 1);
		return;
	}
	
	if (hilite_item < (NUM_SETTINGS - 2))
		highlight_setting (hilite_item + 2);
}

void toggle_setting (void)
{
	ConfigState *cfg = get_config_state();

	if (hilite_item == (NUM_SETTINGS - 1))
	{
		write_config_file();
		display_options();
		return;
	}

	switch (hilite_item)
	{
		case 0:
			cfg->wireframe ^= 1;
			break;
		
		case 1:
			cfg->anti_alias_gfx ^= 1;
			break;
		
		case 2:
			cfg->planet_render_style = (cfg->planet_render_style + 1) % 4;
			break;
		
		case 3:
			cfg->hoopy_casinos ^= 1;
			break;

		case 4:
			cfg->instant_dock ^= 1;
			break;

		case 5:
			cfg->control_scheme ^= 1;
			break;

		case 6:
			cfg->mouse_flight_mode = (cfg->mouse_flight_mode + 1) % 3;
			break;

		case 7:
			cfg->invert_pitch ^= 1;
			break;

		case 8:
			cfg->mouse_sensitivity = (cfg->mouse_sensitivity + 1) % 3;
			break;

		case 9:
			cfg->flight_assist ^= 1;
			break;

		case 10:
			cfg->display_mode = (cfg->display_mode + 1) % 6;
			gfx_apply_display_mode(cfg->display_mode);
			break;

		case 11:
			cfg->aspect_ratio_mode = (cfg->aspect_ratio_mode + 1) % 5;
			break;

		case 12:
			cfg->scaling_filter = (cfg->scaling_filter + 1) % 2;
			break;
	}

	highlight_setting (hilite_item);
}


void game_settings_screen (void)
{
	int i;

	get_session_state()->current_screen = SCR_SETTINGS;

	gfx_clear_display();
	gfx_display_centre_text (10, "GAME SETTINGS", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	for (i = 0; i < NUM_SETTINGS; i++)
	{
		display_setting_item (i);
	}

	hilite_item = -1;
	highlight_setting (0);
}


void display_option_item (int i)
{
	int y;
	int col;
	
	y = (384 - (30 * NUM_OPTIONS)) / 2;
	y += i * 30;
	col = ((!get_session_state()->is_docked) && option_list[i].docked_only) ? GFX_COL_GREY_1 : GFX_COL_WHITE;

	gfx_display_centre_text (y, option_list[i].text, 120, col);
}


void highlight_option (int i)
{
	int y;
	int x;
	
	if ((hilite_item != -1) && (hilite_item != i))
	{
		x = GFX_X_CENTRE - (OPTION_BAR_WIDTH / 2);
		y = (384 - (30 * NUM_OPTIONS)) / 2;
		y += hilite_item * 30;
		gfx_clear_area (x, y, x + OPTION_BAR_WIDTH, y + OPTION_BAR_HEIGHT);
		display_option_item (hilite_item);		
	}

	x = GFX_X_CENTRE - (OPTION_BAR_WIDTH / 2);
	y = (384 - (30 * NUM_OPTIONS)) / 2;
	y += i * 30;
	
	gfx_draw_rectangle (x, y, x + OPTION_BAR_WIDTH, y + OPTION_BAR_HEIGHT,
						GFX_COL_DARK_RED);
	display_option_item (i);		

	hilite_item = i;
}

void select_previous_option (void)
{
	if (hilite_item > 0)
		highlight_option (hilite_item - 1);
}

void select_next_option (void)
{
	if (hilite_item < (NUM_OPTIONS - 1))
		highlight_option (hilite_item + 1);
}


void do_option (void)
{
	if ((!get_session_state()->is_docked) && option_list[hilite_item].docked_only)
		return;

	switch (hilite_item)
	{
		case 0:
			save_commander_screen();
			break;
			
		case 1:
			load_commander_screen();
			display_commander_status();
			break;
		
		case 2:
			game_settings_screen();
			break;
		
		case 3:
			quit_screen();
			break;
	}
}


void display_options (void)
{
	int i;

	get_session_state()->current_screen = SCR_OPTIONS;
	
	gfx_clear_display();
	gfx_display_centre_text (10, "GAME OPTIONS", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_display_centre_text (300, "Version: Release 1.0", 120, GFX_COL_WHITE);
	gfx_display_centre_text (320, "www.newkind.co.uk", 120, GFX_COL_WHITE);
	gfx_display_centre_text (340, "Written by Christian Pinder 1999-2001", 120, GFX_COL_WHITE);
	gfx_display_centre_text (360, "Based on original code by Ian Bell & David Braben", 120, GFX_COL_WHITE);
	
	for (i = 0; i < NUM_OPTIONS; i++)
		display_option_item (i);

	hilite_item = -1;
	highlight_option (0);
}
