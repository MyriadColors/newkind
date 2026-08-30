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
 * file.c
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "game_state.h"

void write_config_file (void)
{
	FILE *fp;
	const ConfigState *cfg = get_config_state();
	
	fp = fopen ("newkind.cfg", "w");
	if (fp == nullptr)
		return;

	fprintf (fp, "%d\t\t# Game Speed, the lower the number the faster the game.\n", cfg->speed_cap);

	fprintf (fp, "%d\t\t# Graphics: 0 = Solid, 1 = Wireframe\n", cfg->wireframe);

	fprintf (fp, "%d\t\t# Anti-Alias Wireframe: 0 = Normal, 1 = Anti-Aliased\n", cfg->anti_alias_gfx);

	fprintf (fp, "%d\t\t# Planet style: 0 = Wireframe, 1 = Green, 2 = SNES, 3 = Fractal\n", cfg->planet_render_style);
	
	fprintf (fp, "%d\t\t# Planet Descriptions: 0 = Tree Grubs, 1 = Hoopy Casinos\n", cfg->hoopy_casinos);

	fprintf (fp, "%d\t\t# Instant dock: 0 = off, 1 = on\n", cfg->instant_dock);

	fprintf (fp, "%d\t\t# Controls: 0 = Classic, 1 = Modern\n", cfg->control_scheme);

	fprintf (fp, "%d\t\t# Mouse flight mode: 0 = Direct, 1 = Virtual Stick, 2 = Off\n", cfg->mouse_flight_mode);

	fprintf (fp, "%d\t\t# Pitch mode: 0 = Standard, 1 = Inverted\n", cfg->invert_pitch);

	fprintf (fp, "%d\t\t# Mouse sensitivity: 0 = Low, 1 = Medium, 2 = High\n", cfg->mouse_sensitivity);

	fprintf (fp, "%d\t\t# Flight assist: 0 = Off, 1 = On\n", cfg->flight_assist);
	
	fprintf (fp, "%d\t\t# Aspect ratio: 0 = 1:1, 1 = 4:3, 2 = 16:9, 3 = Integer, 4 = Stretch\n", cfg->aspect_ratio_mode);

	fprintf (fp, "%d\t\t# Texture filter: 0 = Point (Crisp), 1 = Bilinear (Smooth)\n", cfg->scaling_filter);

	fprintf (fp, "%d\t\t# Display mode: 0 = Maximized, 1 = Fullscreen, 2 = 800x600, 3 = 1024x768, 4 = 1280x720, 5 = 1920x1080\n", cfg->display_mode);

	fprintf (fp, "%d\t\t# Tactical HUD: 0 = Off, 1 = Standard, 2 = Full\n", cfg->tactical_hud);

	fprintf (fp, "%d\t\t# Master Volume: 0 - 100%%\n", cfg->master_volume);

	fprintf (fp, "%d\t\t# Music Volume: 0 - 100%%\n", cfg->music_volume);

	fprintf (fp, "%d\t\t# SFX Volume: 0 - 100%%\n", cfg->sfx_volume);

	fprintf (fp, "newscan.cfg\t# Name of scanner config file to use.\n");

	fclose (fp);
}


/*
 * Read a line from a .cfg file.
 * Ignore blanks, comments and strip white space.
 */

int read_cfg_line (char *str, int max_size, FILE *fp)
{
	char *s;

	do
	{	
		if (fgets (str, max_size, fp) == nullptr)
			return -1;

		for (s = str; *s; s++)					/* End of line at LF or # */
		{
			if ((*s == '\n') || (*s == '#'))
			{
				*s = '\0';
				break;
			} 		
		}

		if (s != str)							/* Trim any trailing white space */
		{
			s--;
			while (isspace((unsigned char)*s))
			{
				*s = '\0';
				if (s == str)
					break;
				s--;
			}
		}

	} while (*str == '\0');

	return 0;
}


/*
 * Read in the scanner .cfg file.
 */

void read_scanner_config_file (const char *filename)
{
	FILE *fp;
	char str[256];
	ConfigState *cfg = get_config_state();
	
	fp = fopen (filename, "r");
	if (fp == nullptr)
		return;

	if (read_cfg_line (str, sizeof(str), fp) == 0)
		strcpy (cfg->scanner_filename, str);

	if (read_cfg_line (str, sizeof(str), fp) == 0)
	{
		sscanf (str, "%d,%d", &cfg->scanner_cx, &cfg->scanner_cy);
		cfg->scanner_cy += 385;
	}

	if (read_cfg_line (str, sizeof(str), fp) == 0)
	{
		sscanf (str, "%d,%d", &cfg->compass_centre_x, &cfg->compass_centre_y);
		cfg->compass_centre_y += 385;
	}
	
	fclose (fp);
}

/*
 * Read in the newkind.cfg file.
 */

void read_config_file (void)
{
	FILE *fp;
	char str[256];
	int extra_idx = 0;
	ConfigState *cfg = get_config_state();
	
	fp = fopen ("newkind.cfg", "r");
	if (fp == nullptr)
		return;

	if (read_cfg_line (str, sizeof(str), fp) == 0)
		sscanf (str, "%d", &cfg->speed_cap);

	if (read_cfg_line (str, sizeof(str), fp) == 0)
		sscanf (str, "%d", &cfg->wireframe);

	if (read_cfg_line (str, sizeof(str), fp) == 0)
		sscanf (str, "%d", &cfg->anti_alias_gfx);

	if (read_cfg_line (str, sizeof(str), fp) == 0)
		sscanf (str, "%d", &cfg->planet_render_style);
	
	if (read_cfg_line (str, sizeof(str), fp) == 0)
		sscanf (str, "%d", &cfg->hoopy_casinos);

	if (read_cfg_line (str, sizeof(str), fp) == 0)
		sscanf (str, "%d", &cfg->instant_dock);

	while (read_cfg_line (str, sizeof(str), fp) == 0)
	{
		if (strstr(str, ".cfg") != nullptr)
		{
			read_scanner_config_file (str);
			break;
		}
		else
		{
			if (extra_idx == 0) sscanf (str, "%d", &cfg->control_scheme);
			else if (extra_idx == 1) sscanf (str, "%d", &cfg->mouse_flight_mode);
			else if (extra_idx == 2) sscanf (str, "%d", &cfg->invert_pitch);
			else if (extra_idx == 3) sscanf (str, "%d", &cfg->mouse_sensitivity);
			else if (extra_idx == 4) sscanf (str, "%d", &cfg->flight_assist);
			else if (extra_idx == 5) sscanf (str, "%d", &cfg->aspect_ratio_mode);
			else if (extra_idx == 6) sscanf (str, "%d", &cfg->scaling_filter);
			else if (extra_idx == 7) sscanf (str, "%d", &cfg->display_mode);
			else if (extra_idx == 8) sscanf (str, "%d", &cfg->tactical_hud);
			else if (extra_idx == 9) sscanf (str, "%d", &cfg->master_volume);
			else if (extra_idx == 10) sscanf (str, "%d", &cfg->music_volume);
			else if (extra_idx == 11) sscanf (str, "%d", &cfg->sfx_volume);
			extra_idx++;
		}
	}
		
	fclose (fp);
}

int checksum (unsigned char *block)
{
	int acc,carry;
	int i;

	acc = 0x49;
	carry = 0;
	for (i = 0x49; i > 0; i--)
	{
		acc += block[i-1] + carry;
		carry = acc >> 8;
		acc &= 255;
		acc ^= block[i];
	}

	return acc;
}


int save_commander_file (const char *path)
{
	FILE *fp;
	unsigned char block[256];
	int i;
	int chk;
	const PlayerState *player = get_player_state();
	const UniverseState *env = get_universe_state();
	const struct commander *cmd = &player->current;
	
	fp = fopen (path, "wb");
	if (fp == nullptr)
		return 1;
	
	block[0]  = cmd->mission;
	block[1]  = env->docked_planet.d;
	block[2]  = env->docked_planet.b;
	block[3]  = cmd->galaxy.a;
	block[4]  = cmd->galaxy.b;
	block[5]  = cmd->galaxy.c;
	block[6]  = cmd->galaxy.d;
	block[7]  = cmd->galaxy.e;
	block[8]  = cmd->galaxy.f;
	block[9]  = (cmd->credits >> 24) & 255;
	block[10] = (cmd->credits >> 16) & 255;
	block[11] = (cmd->credits >> 8) & 255;
	block[12] = cmd->credits & 255;
	block[13] = cmd->fuel;
	block[14] = 4;
	block[15] = cmd->galaxy_number;
	block[16] = cmd->front_laser;
	block[17] = cmd->rear_laser;
	block[18] = cmd->left_laser;
	block[19] = cmd->right_laser;
	block[20] = 0;
	block[21] = 0;
	block[22] = cmd->cargo_capacity + 2;

	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
		block[23+i] = cmd->current_cargo[i];
	
	block[40] = cmd->ecm ? 255 : 0;
	block[41] = cmd->fuel_scoop ? 255 : 0;
	block[42] = cmd->energy_bomb ? 0x7F : 0;
	block[43] = cmd->energy_unit;
	block[44] = cmd->docking_computer ? 255 : 0;
	block[45] = cmd->galactic_hyperdrive ? 255 : 0;
	block[46] = cmd->escape_pod ? 255 : 0;
	block[47] = 0;
	block[48] = 0;
	block[49] = 0;
	block[50] = 0;
	block[51] = cmd->missiles;
	block[52] = cmd->legal_status;
	
	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
		block[53+i] = stock_market[i].current_quantity;
	
	block[70] = cmd->market_rnd;
	block[71] = cmd->score & 255;
	block[72] = cmd->score >> 8;
	block[73] = 0x20;

	chk = checksum (block);
	
	block[74] = chk ^ 0xA9;
	block[75] = chk;
	
	for (i = 76; i < 256; i++)
		block[i] = 0;

	if (fwrite (block, 256, 1, fp) != 1)
		return 1;
		
	if (fclose (fp) == EOF)
		return 1;	

	return 0;
}


int load_commander_file (const char *path)
{
	FILE *fp;
	unsigned char block[256];
	int i;
	int chk;
	PlayerState *player = get_player_state();
	struct commander *saved = &player->saved;
	
	fp = fopen (path, "rb");
	if (fp == nullptr)
		return 1;

	if (fread (block, 256, 1, fp) != 1)
		return 1;

	chk = checksum (block);

	if ((block[74] != (chk ^ 0xA9)) || (block[75] != chk))
		return 1;
	
	saved->mission = block[0];

	saved->ship_x = block[1];
	saved->ship_y = block[2];
	
	saved->galaxy.a = block[3];
	saved->galaxy.b = block[4];
	saved->galaxy.c = block[5];
	saved->galaxy.d = block[6];
	saved->galaxy.e = block[7];
	saved->galaxy.f = block[8];
	
	saved->credits = (int)(((uint32_t)block[9] << 24) |
	                       ((uint32_t)block[10] << 16) |
	                       ((uint32_t)block[11] << 8) |
	                       ((uint32_t)block[12]));

	saved->fuel = block[13];

	saved->galaxy_number = block[15];
	saved->front_laser = block[16];
	saved->rear_laser = block[17];
	saved->left_laser = block[18];
	saved->right_laser = block[19];

	saved->cargo_capacity = block[22] - 2;

	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
		saved->current_cargo[i] = block[23+i];
	
	saved->ecm = block[40];
	saved->fuel_scoop = block[41];
	saved->energy_bomb = block[42];
	saved->energy_unit = block[43];
	saved->docking_computer = block[44];
	saved->galactic_hyperdrive = block[45];
	saved->escape_pod = block[46];
	saved->missiles = block[51];
	saved->legal_status = block[52];
	
	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
		saved->station_stock[i] = block[53+i];
	
	saved->market_rnd = block[70];

	saved->score = block[71];
	saved->score += block[72] << 8;

	if (fclose (fp) == EOF)
		return 1;	

	return 0;
}




