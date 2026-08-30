/*
 * Elite - The New Kind (Raylib Port)
 *
 * Raylib version of the main game handler.
 */

#include <stdio.h>
#include <string.h>
#include <math.h> 
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "raylib.h"

#include "config.h"
#include "gfx.h"
#include "main.h"
#include "vector.h"
#include "elite.h"
#include "docked.h"
#include "intro.h"
#include "shipdata.h"
#include "shipface.h"
#include "space.h"
#include "sound.h"
#include "threed.h"
#include "swat.h"
#include "random.h"
#include "options.h"
#include "stars.h"
#include "missions.h"
#include "pilot.h"
#include "file.h"
#include "keyboard.h"
#include "game_state.h"

#define current_screen (get_session_state()->current_screen)
#define docked (get_session_state()->is_docked)
#define finish (get_session_state()->is_finish)
#define game_over (get_session_state()->is_game_over)

#define cmdr (get_player_state()->current)
#define saved_cmdr (get_player_state()->saved)
#define myship (get_player_state()->vitals.ship_specs)
#define myship_energy (get_player_state()->vitals.energy)
#define front_shield (get_player_state()->vitals.front_shield)
#define aft_shield (get_player_state()->vitals.aft_shield)
#define laser_temp (get_player_state()->vitals.laser_temp)
#define detonate_bomb (get_player_state()->vitals.detonate_bomb)
#define auto_pilot (get_player_state()->vitals.auto_pilot)
#define witchspace (get_player_state()->vitals.witchspace)
#define hyper_ready (get_player_state()->vitals.hyper_ready)
#define can_fast_dock (get_player_state()->vitals.can_fast_dock)
#define mcount (get_player_state()->vitals.mcount)

#define docked_planet (get_universe_state()->docked_planet)
#define hyperspace_planet (get_universe_state()->hyperspace_planet)
#define current_planet_data (get_universe_state()->current_planet_data)
#define universe (get_universe_state()->objects)
#define ship_count (get_universe_state()->ship_count)

#define flight_speed (get_flight_state()->speed)
#define flight_roll (get_flight_state()->roll)
#define flight_climb (get_flight_state()->climb)
#define flight_yaw (get_flight_state()->yaw)
#define flight_roll_f (get_flight_state()->roll_f)
#define flight_climb_f (get_flight_state()->climb_f)
#define flight_yaw_f (get_flight_state()->yaw_f)

#define speed_cap (get_config_state()->speed_cap)
#define wireframe (get_config_state()->wireframe)
#define anti_alias_gfx (get_config_state()->anti_alias_gfx)
#define planet_render_style (get_config_state()->planet_render_style)
#define control_scheme (get_config_state()->control_scheme)
#define mouse_flight_mode (get_config_state()->mouse_flight_mode)
#define invert_pitch (get_config_state()->invert_pitch)
#define mouse_sensitivity (get_config_state()->mouse_sensitivity)
#define flight_assist (get_config_state()->flight_assist)
#define aspect_ratio_mode (get_config_state()->aspect_ratio_mode)
#define scaling_filter (get_config_state()->scaling_filter)
#define cross_x (get_chart_state()->cross_x)
#define cross_y (get_chart_state()->cross_y)
#define cross_timer (get_chart_state()->cross_timer)
#define planet_unknown (get_chart_state()->planet_unknown)
#define find_input (get_chart_state()->find_input)
#define find_name (get_chart_state()->find_name)

#define message_count (get_ui_state()->message_count)
#define message_string (get_ui_state()->message_string)
#define game_paused (get_ui_state()->game_paused)
#define draw_lasers (get_ui_state()->draw_lasers)
#define have_joystick (get_ui_state()->have_joystick)
#define rolling (get_ui_state()->rolling)
#define climbing (get_ui_state()->climbing)
#define yawing (get_ui_state()->yawing)

/*
 * Initialise the game parameters.
 */
void initialise_game(void)
{
	set_rand_seed (time(nullptr));
	current_screen = SCR_INTRO_ONE;

	restore_saved_commander();

	flight_speed = 1;
	flight_roll = 0;
	flight_climb = 0;
	flight_yaw = 0;
	docked = 1;
	front_shield = 255;
	aft_shield = 255;
	myship_energy = 255;
	draw_lasers = 0;
	mcount = 0;
	hyper_ready = 0;
	detonate_bomb = 0;
	find_input = false;
	witchspace = 0;
	game_paused = 0;
	auto_pilot = 0;
	
	create_new_stars();
	clear_universe();
	
	cross_x = -1;
	cross_y = -1;
	cross_timer = 0;

	myship.max_speed = 40;		/* 0.27 Light Mach */
	myship.max_roll = 31;
	myship.max_climb = 8;		/* CF 8 */
	myship.max_fuel = 70;		/* 7.0 Light Years */
}

void finish_game (void)
{
	finish = 1;
	game_over = 1;
}

void move_cross (int dx, int dy)
{
	cross_timer = 5;
	planet_unknown = 0;

	int step = kbd_shift_down ? 3 : 1;

	if (current_screen == SCR_SHORT_RANGE)
	{
		cross_x += (dx * 4 * step);
		cross_y += (dy * 4 * step);
		return;
	}

	if (current_screen == SCR_GALACTIC_CHART)
	{
		cross_x += (dx * 2 * step);
		cross_y += (dy * 2 * step);

		if (cross_x < 1)
			cross_x = 1;
			
		if (cross_x > 510)
			cross_x = 510;

		if (cross_y < 37)
			cross_y = 37;
		
		if (cross_y > 293)
			cross_y = 293;
	}
}

void draw_laser_sights(void)
{
	int laser = 0;
	int x1,y1,x2,y2;
	
	switch (current_screen)
	{
		case SCR_FRONT_VIEW:
			gfx_display_centre_text (32, "Front View", 120, GFX_COL_WHITE);
			laser = cmdr.front_laser;
			break;
		
		case SCR_REAR_VIEW:
			gfx_display_centre_text (32, "Rear View", 120, GFX_COL_WHITE);
			laser = cmdr.rear_laser;
			break;

		case SCR_LEFT_VIEW:
			gfx_display_centre_text (32, "Left View", 120, GFX_COL_WHITE);
			laser = cmdr.left_laser;
			break;

		case SCR_RIGHT_VIEW:
			gfx_display_centre_text (32, "Right View", 120, GFX_COL_WHITE);
			laser = cmdr.right_laser;
			break;
	}

	if (laser)
	{
		x1 = 128 * GFX_SCALE;
		y1 = (96-8) * GFX_SCALE;
		y2 = (96-16) * GFX_SCALE;
   
		gfx_draw_colour_line (x1-1, y1, x1-1, y2, GFX_COL_GREY_1); 
		gfx_draw_colour_line (x1, y1, x1, y2, GFX_COL_WHITE);
		gfx_draw_colour_line (x1+1, y1, x1+1, y2, GFX_COL_GREY_1); 

		y1 = (96+8) * GFX_SCALE;
		y2 = (96+16) * GFX_SCALE;
		
		gfx_draw_colour_line (x1-1, y1, x1-1, y2, GFX_COL_GREY_1); 
		gfx_draw_colour_line (x1, y1, x1, y2, GFX_COL_WHITE);
		gfx_draw_colour_line (x1+1, y1, x1+1, y2, GFX_COL_GREY_1); 

		x1 = (128-8) * GFX_SCALE;
		y1 = 96 * GFX_SCALE;
		x2 = (128-16) * GFX_SCALE;
		   
		gfx_draw_colour_line (x1, y1-1, x2, y1-1, GFX_COL_GREY_1); 
		gfx_draw_colour_line (x1, y1, x2, y1, GFX_COL_WHITE);
		gfx_draw_colour_line (x1, y1+1, x2, y1+1, GFX_COL_GREY_1); 

		x1 = (128+8) * GFX_SCALE;
		x2 = (128+16) * GFX_SCALE;

		gfx_draw_colour_line (x1, y1-1, x2, y1-1, GFX_COL_GREY_1); 
		gfx_draw_colour_line (x1, y1, x2, y1, GFX_COL_WHITE);
		gfx_draw_colour_line (x1, y1+1, x2, y1+1, GFX_COL_GREY_1); 
	}

	if (control_scheme == 1 && mouse_flight_mode == 1 && mouse_y < 380)
	{
		gfx_draw_colour_line (mouse_x - 4, mouse_y, mouse_x + 4, mouse_y, GFX_COL_CYAN);
		gfx_draw_colour_line (mouse_x, mouse_y - 4, mouse_x, mouse_y + 4, GFX_COL_CYAN);
	}

	draw_tactical_hud();
}

void arrow_right(void)
{
	switch (current_screen)
	{
		case SCR_MARKET_PRICES:
			buy_stock ();
			break;

		case SCR_SETTINGS:
			select_right_setting();
			break;

		case SCR_SHORT_RANGE:
		case SCR_GALACTIC_CHART:
			move_cross (1, 0);
			break;

		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_RIGHT_VIEW:
		case SCR_LEFT_VIEW:
			if (flight_roll > 0)
				flight_roll = 0;
			else
			{
				decrease_flight_roll();
				decrease_flight_roll();
				rolling = 1;
			}
			break;
	}
}

void arrow_left(void)
{
	switch (current_screen)
	{
		case SCR_MARKET_PRICES:
			sell_stock ();
			break;

		case SCR_SETTINGS:
			select_left_setting();
			break;

		case SCR_SHORT_RANGE:
		case SCR_GALACTIC_CHART:
			move_cross (-1, 0);
			break;

		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_RIGHT_VIEW:
		case SCR_LEFT_VIEW:
			if (flight_roll < 0)
				flight_roll = 0;
			else
			{
				increase_flight_roll();
				increase_flight_roll();
				rolling = 1;
			}
			break;
	}
}

void arrow_up (void)
{
	switch (current_screen)
	{
		case SCR_MARKET_PRICES:
			select_previous_stock();
			break;

		case SCR_EQUIP_SHIP:
			select_previous_equip();
			break;

		case SCR_OPTIONS:
			select_previous_option();
			break;

		case SCR_SETTINGS:
			select_up_setting();
			break;
		
		case SCR_SHORT_RANGE:
		case SCR_GALACTIC_CHART:
			move_cross (0, -1);
			break;

		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_RIGHT_VIEW:
		case SCR_LEFT_VIEW:
			if (flight_climb > 0)
				flight_climb = 0;
			else
			{
				decrease_flight_climb();
			}
			climbing = 1;
			break;
	}
}

void arrow_down (void)
{
	switch (current_screen)
	{
		case SCR_MARKET_PRICES:
			select_next_stock();
			break;

		case SCR_EQUIP_SHIP:
			select_next_equip();
			break;
		
		case SCR_OPTIONS:
			select_next_option();
			break;

		case SCR_SETTINGS:
			select_down_setting();
			break;
		
		case SCR_SHORT_RANGE:
		case SCR_GALACTIC_CHART:
			move_cross (0, 1);
			break;

		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_RIGHT_VIEW:
		case SCR_LEFT_VIEW:
			if (flight_climb < 0)
				flight_climb = 0;
			else
			{
				increase_flight_climb();
			}
			climbing = 1;
			break;

	}
}

void return_pressed (void)
{
	switch (current_screen)
	{
		case SCR_EQUIP_SHIP:
			buy_equip();
			break;
		
		case SCR_OPTIONS:
			do_option();
			break;

		case SCR_SETTINGS:
			toggle_setting();
			break;
	}	
}

void y_pressed (void)
{
	switch (current_screen)
	{
		case SCR_QUIT:
			finish_game();
			break;
	}
}

void n_pressed (void)
{
	switch (current_screen)
	{
		case SCR_QUIT:
			if (docked)
				display_commander_status();
			else
				current_screen = SCR_FRONT_VIEW;
			break;
	}
}

void d_pressed (void)
{
	switch (current_screen)
	{
		case SCR_GALACTIC_CHART:
		case SCR_SHORT_RANGE:
    		show_distance_to_planet();
			break;
		
		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_RIGHT_VIEW:
		case SCR_LEFT_VIEW:
			if (kbd_shift_down && can_fast_dock && !docked)
			{
				snd_play_sample (SND_DOCK);
				dock_player();
				current_screen = SCR_BREAK_PATTERN;
				break;
			}
			if (auto_pilot)
				disengage_auto_pilot();
			break;
	}
}

void f_pressed (void)
{
	if ((current_screen == SCR_GALACTIC_CHART) ||
		(current_screen == SCR_SHORT_RANGE))
	{
		find_input = true;
		*find_name = '\0';
		planet_unknown = 0;
	}
}

void add_find_char (int letter)
{
	char str[2];
	
	if (strlen (find_name) == 16)
		return;
		
	str[0] = toupper (letter);
	str[1] = '\0';
	strcat (find_name, str);
}

void delete_find_char (void)
{
	int len = strlen (find_name);
	if (len == 0)
		return;
		
	find_name[len - 1] = '\0';	
}

void o_pressed()
{
	switch (current_screen)
	{
		case SCR_GALACTIC_CHART:
		case SCR_SHORT_RANGE:
    		move_cursor_to_origin();
			break;
	}
}

void auto_dock (void)
{
	struct univ_object ship;

	ship.location.x = 0;
	ship.location.y = 0;
	ship.location.z = 0;
	
	set_init_matrix (ship.rotmat);
	ship.rotmat[2].z = 1;
	ship.rotmat[0].x = -1;
	ship.type = -96;
	ship.velocity = flight_speed;
	ship.acceleration = 0;
	ship.bravery = 0;
	ship.rotz = 0;
	ship.rotx = 0;

	auto_pilot_ship (&ship);

	if (ship.velocity > 22)
		flight_speed = 22;
	else
		flight_speed = ship.velocity;
		
	if (ship.velocity <= 0)
		flight_speed = 1;
		
	if (ship.rotx < 0)
		increase_flight_climb();
		
	if (ship.rotx > 0)
		decrease_flight_climb();
		
	if (ship.rotz < 0)
		decrease_flight_roll();

	if (ship.rotz > 0)
		increase_flight_roll();

	if (ship.rotz != 0)
		rolling = 1;

	if (ship.rotx != 0)
		climbing = 1;
}

void handle_flight_keys (void)
{
	int key;

	kbd_poll_keyboard();

	if (find_input)
	{
		key = kbd_read_key();
		
		if (key == 27) /* ESC */
		{
			find_input = false;
			planet_unknown = 0;
			return;
		}

		if (key == '\r')
		{
			find_input = false;
			find_planet_by_name (find_name);
			return;
		}

		if (key == 8)
		{
			delete_find_char();
			return;
		}

		if (key >= 'a' && key <= 'z')
		{
			add_find_char(key);
			return;
		}

		if (key >= 'A' && key <= 'Z')
		{
			add_find_char(key);
			return;
		}

		return;
	}

	if (docked)
	{
		if (handle_quicknav_mouse_input())
			return;
	}

	if (current_screen == SCR_GALACTIC_CHART || current_screen == SCR_SHORT_RANGE)
	{
		handle_chart_mouse_input();
	}
	else if (current_screen == SCR_MARKET_PRICES)
	{
		handle_market_mouse_input();
	}
	else if (current_screen == SCR_EQUIP_SHIP)
	{
		handle_equip_mouse_input();
	}
	else if (current_screen == SCR_OPTIONS)
	{
		handle_options_mouse_input();
	}
	else if (current_screen == SCR_SETTINGS)
	{
		handle_settings_mouse_input();
	}

	if (kbd_F1_pressed)
	{
		if (docked)
		{
			launch_player();
		}
		else
		{
			if (current_screen != SCR_FRONT_VIEW)
			{
				current_screen = SCR_FRONT_VIEW;
				gfx_clear_display();
			}
		}
	}

	if (kbd_F2_pressed)
	{
		if (!docked && (current_screen != SCR_REAR_VIEW))
		{
			current_screen = SCR_REAR_VIEW;
			gfx_clear_display();
		}
	}

	if (kbd_F3_pressed)
	{
		if (!docked && (current_screen != SCR_LEFT_VIEW))
		{
			current_screen = SCR_LEFT_VIEW;
			gfx_clear_display();
		}
	}

	if (kbd_F4_pressed)
	{
		if (docked)
		{
			if (current_screen != SCR_EQUIP_SHIP)
			{
				current_screen = SCR_EQUIP_SHIP;
				equip_ship();
			}
		}
		else if (current_screen != SCR_RIGHT_VIEW)
		{
			current_screen = SCR_RIGHT_VIEW;
			gfx_clear_display();
		}
	}

	if (kbd_F5_pressed)
	{
		if (current_screen != SCR_GALACTIC_CHART)
		{
			current_screen = SCR_GALACTIC_CHART;
			cross_x = hyperspace_planet.d * GFX_SCALE;
			cross_y = (hyperspace_planet.b / (2 / GFX_SCALE)) + (18 * GFX_SCALE) + 1;
			cross_timer = 0;
			planet_unknown = 0;
			find_input = 0;
			display_galactic_chart();
		}
	}

	if (kbd_F6_pressed)
	{
		if (current_screen != SCR_SHORT_RANGE)
		{
			current_screen = SCR_SHORT_RANGE;
			cross_x = ((hyperspace_planet.d - docked_planet.d) * 4 * GFX_SCALE) + GFX_X_CENTRE;
			cross_y = ((hyperspace_planet.b - docked_planet.b) * 2 * GFX_SCALE) + GFX_Y_CENTRE;
			cross_timer = 0;
			planet_unknown = 0;
			find_input = 0;
			display_short_range_chart();
		}
	}

	if (kbd_F7_pressed)
	{
		if (current_screen != SCR_PLANET_DATA)
		{
			current_screen = SCR_PLANET_DATA;
			display_data_on_planet();
		}
	}

	if (kbd_F8_pressed)
	{
		if (docked)
		{
			if (current_screen != SCR_MARKET_PRICES)
			{
				current_screen = SCR_MARKET_PRICES;
				display_market_prices();
			}
		}
	}

	if (kbd_F9_pressed)
	{
		if (current_screen != SCR_CMDR_STATUS)
		{
			current_screen = SCR_CMDR_STATUS;
			display_commander_status();
		}
	}

	if (kbd_F10_pressed)
	{
		if (current_screen != SCR_INVENTORY)
		{
			current_screen = SCR_INVENTORY;
			display_inventory();
		}
	}

	if (kbd_F11_pressed)
	{
		if (current_screen != SCR_OPTIONS)
		{
			current_screen = SCR_OPTIONS;
			display_options();
		}
	}

	if (kbd_F12_pressed)
	{
		gfx_toggle_maximize();
	}

	int is_flight_view = (!docked) && ((current_screen == SCR_FRONT_VIEW) ||
	                                   (current_screen == SCR_REAR_VIEW) ||
	                                   (current_screen == SCR_LEFT_VIEW) ||
	                                   (current_screen == SCR_RIGHT_VIEW));

	if (is_flight_view)
	{
		/* Missile Controls */
		if (kbd_target_missile_pressed)
			arm_missile();

		if (kbd_fire_missile_pressed)
			fire_missile();

		if (kbd_unarm_missile_pressed)
			unarm_missile();

		/* Classic Flight Control Scheme */
		if (control_scheme == 0)
		{
			/* Classic Roll: Comma / Period / Left / Right Arrow */
			if (IsKeyDown(KEY_COMMA) || IsKeyDown(KEY_LEFT))
			{
				if (flight_roll < 0)
					flight_roll_f = 0.0;
				else
					increase_flight_roll();
				rolling = 1;
			}

			if (IsKeyDown(KEY_PERIOD) || IsKeyDown(KEY_RIGHT))
			{
				if (flight_roll > 0)
					flight_roll_f = 0.0;
				else
					decrease_flight_roll();
				rolling = 1;
			}

			/* Classic Pitch: S (Climb) / X (Dive) / Up / Down Arrow */
			if (IsKeyDown(KEY_S) || IsKeyDown(KEY_UP))
			{
				if (flight_climb < 0)
					flight_climb_f = 0.0;
				else
					increase_flight_climb();
				climbing = 1;
			}

			if (IsKeyDown(KEY_X) || IsKeyDown(KEY_DOWN))
			{
				if (flight_climb > 0)
					flight_climb_f = 0.0;
				else
					decrease_flight_climb();
				climbing = 1;
			}
		}
		/* Modern Flight Control Scheme */
		else if (control_scheme == 1)
		{
			/* Mouse Actions (only when Mouse Flight is not Off) */
			if (mouse_flight_mode != 2)
			{
				if (mouse_left_down && draw_lasers == 0)
				{
					draw_lasers = fire_laser();
				}

				if (mouse_right_pressed)
				{
					if (missile_target == MISSILE_UNARMED)
						arm_missile();
					else if (missile_target >= 0)
						fire_missile();
					else
						unarm_missile();
				}

				if (mouse_middle_pressed)
				{
					if (cmdr.ecm)
						activate_ecm(1);
					else
						jump_warp();
				}

				/* Mouse Wheel Throttle */
				if (mouse_wheel_delta > 0.05f)
				{
					flight_speed += (int)(mouse_wheel_delta * 2.0f + 0.5f);
					if (flight_speed > myship.max_speed)
						flight_speed = myship.max_speed;
				}
				else if (mouse_wheel_delta < -0.05f)
				{
					flight_speed += (int)(mouse_wheel_delta * 2.0f - 0.5f);
					if (flight_speed < 1)
						flight_speed = 1;
				}

				/* Mouse Flight Steering */
				double sens = (mouse_sensitivity == 0) ? 0.40 : (mouse_sensitivity == 1) ? 0.75 : 1.25;

				if (mouse_flight_mode == 0) /* Direct Delta */
				{
					if (mouse_dx != 0 || mouse_dy != 0)
					{
						if (mouse_dx != 0)
						{
							double m_dx = (double)mouse_dx;
							double sign_x = (m_dx > 0.0) ? 1.0 : -1.0;
							double yaw_cmd = sign_x * pow(fabs(m_dx), 1.18) * sens * 1.5;
							flight_yaw_f += yaw_cmd;
							if (flight_yaw_f > myship.max_roll) flight_yaw_f = myship.max_roll;
							if (flight_yaw_f < -myship.max_roll) flight_yaw_f = -myship.max_roll;
							flight_yaw = (int)round(flight_yaw_f);
							yawing = 1;
						}

						if (mouse_dy != 0)
						{
							double m_dy = (double)mouse_dy;
							double sign_y = (m_dy > 0.0) ? 1.0 : -1.0;
							double p_dir = (invert_pitch == 0) ? 1.0 : -1.0;
							double pitch_cmd = p_dir * sign_y * pow(fabs(m_dy), 1.18) * sens * 1.5;
							flight_climb_f += pitch_cmd;
							if (flight_climb_f > myship.max_climb) flight_climb_f = myship.max_climb;
							if (flight_climb_f < -myship.max_climb) flight_climb_f = -myship.max_climb;
							flight_climb = (int)round(flight_climb_f);
							climbing = 1;
						}
					}
				}
				else if (mouse_flight_mode == 1) /* Virtual Joystick */
				{
					int dx = mouse_x - 256;
					int dy = mouse_y - 192;

					if (abs(dx) > 12)
					{
						double eff_dx = (dx > 0) ? (dx - 12) : (dx + 12);
						double sign_x = (eff_dx > 0.0) ? 1.0 : -1.0;
						double target_yaw = sign_x * pow(fabs(eff_dx) / 120.0, 1.2) * myship.max_roll * sens * 1.3;
						if (target_yaw > myship.max_roll) target_yaw = myship.max_roll;
						if (target_yaw < -myship.max_roll) target_yaw = -myship.max_roll;

						flight_yaw_f += (target_yaw - flight_yaw_f) * 0.35;
						flight_yaw = (int)round(flight_yaw_f);
						yawing = 1;
					}

					if (abs(dy) > 12)
					{
						double eff_dy = (dy > 0) ? (dy - 12) : (dy + 12);
						double sign_y = (eff_dy > 0.0) ? 1.0 : -1.0;
						double p_dir = (invert_pitch == 0) ? 1.0 : -1.0;
						double target_climb = p_dir * sign_y * pow(fabs(eff_dy) / 100.0, 1.2) * myship.max_climb * sens * 1.3;
						if (target_climb > myship.max_climb) target_climb = myship.max_climb;
						if (target_climb < -myship.max_climb) target_climb = -myship.max_climb;

						flight_climb_f += (target_climb - flight_climb_f) * 0.35;
						flight_climb = (int)round(flight_climb_f);
						climbing = 1;
					}
				}
			}

			/* Modern Keyboard Flight */
			if (kbd_left_down)
			{
				flight_yaw_f -= (myship.max_roll * 0.35);
				if (flight_yaw_f < -myship.max_roll) flight_yaw_f = -myship.max_roll;
				flight_yaw = (int)round(flight_yaw_f);
				yawing = 1;
			}

			if (kbd_right_down)
			{
				flight_yaw_f += (myship.max_roll * 0.35);
				if (flight_yaw_f > myship.max_roll) flight_yaw_f = myship.max_roll;
				flight_yaw = (int)round(flight_yaw_f);
				yawing = 1;
			}

			if (kbd_q_down)
			{
				flight_roll_f += (myship.max_roll * 0.35);
				if (flight_roll_f > myship.max_roll) flight_roll_f = myship.max_roll;
				flight_roll = (int)round(flight_roll_f);
				rolling = 1;
			}

			if (kbd_e_down)
			{
				flight_roll_f -= (myship.max_roll * 0.35);
				if (flight_roll_f < -myship.max_roll) flight_roll_f = -myship.max_roll;
				flight_roll = (int)round(flight_roll_f);
				rolling = 1;
			}

			if (kbd_up_down)
			{
				double p_step = (invert_pitch == 0 ? -1.0 : 1.0) * (myship.max_climb * 0.35);
				flight_climb_f += p_step;
				if (flight_climb_f > myship.max_climb) flight_climb_f = myship.max_climb;
				if (flight_climb_f < -myship.max_climb) flight_climb_f = -myship.max_climb;
				flight_climb = (int)round(flight_climb_f);
				climbing = 1;
			}

			if (kbd_down_down)
			{
				double p_step = (invert_pitch == 0 ? 1.0 : -1.0) * (myship.max_climb * 0.35);
				flight_climb_f += p_step;
				if (flight_climb_f > myship.max_climb) flight_climb_f = myship.max_climb;
				if (flight_climb_f < -myship.max_climb) flight_climb_f = -myship.max_climb;
				flight_climb = (int)round(flight_climb_f);
				climbing = 1;
			}
		}
	}

	if (kbd_fire_pressed)
	{
		if (!docked && is_flight_view)
		{
			if (draw_lasers == 0)
			{
				draw_lasers = fire_laser();
			}
		}
	}

	if (kbd_inc_speed_pressed)
	{
		if (!docked && is_flight_view)
		{
			if (flight_speed < myship.max_speed)
				flight_speed++;
		}
	}

	if (kbd_dec_speed_pressed)
	{
		if (!docked && is_flight_view)
		{
			if (flight_speed > 1)
				flight_speed--;
		}
	}

	if (!is_flight_view)
	{
		if (kbd_up_pressed)
			arrow_up();

		if (kbd_down_pressed)
			arrow_down();

		if (kbd_left_pressed)
			arrow_left();

		if (kbd_right_pressed)
			arrow_right();
	}

	if (kbd_enter_pressed)
		return_pressed();

	if (kbd_y_pressed)
		y_pressed();

	if (kbd_n_pressed)
		n_pressed();

	if (kbd_d_pressed)
		d_pressed();

	if (kbd_find_pressed)
		f_pressed();

	if (kbd_origin_pressed)
		o_pressed();

	if (kbd_ecm_pressed)
	{
		if (!docked && cmdr.ecm)
			activate_ecm(1);
	}

	if (kbd_hyperspace_pressed)
	{
		if (!docked)
			start_hyperspace();
	}

	if (kbd_jump_pressed)
	{
		if (!docked)
			jump_warp();
	}

	if (kbd_dock_pressed)
	{
		if (!docked && cmdr.docking_computer)
			auto_pilot = 1;
	}
}

void info_message (const char *message)
{
	strcpy (message_string, message);
	message_count = 37;
}

void set_commander_name (const char *path)
{
	const char *fname = GetFileName(path);
	char *cname = cmdr.name;
	int i;

	for (i = 0; i < 31; i++)
	{
		if (!isalnum((unsigned char)*fname)) {
			break;
		}
		*cname++ = toupper((unsigned char)*fname++);
	}

	*cname = '\0';
}

void save_commander_screen (void)
{
	char path[255];
	int okay;
	int rv;
	
	current_screen = SCR_SAVE_CMDR;

	gfx_clear_display();
	gfx_display_centre_text (10, "SAVE COMMANDER", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_update_screen();
	
	strcpy (path, cmdr.name);
	strcat (path, ".nkc");
	
	okay = gfx_request_file ("Save Commander", path, "nkc");
	
	if (!okay)
	{
		display_options();
		return;
	}

	rv = save_commander_file (path);

	if (rv)
	{
		gfx_display_modal_message ("Error Saving Commander!", "Could not save commander file.");
		return;
	}
	
	gfx_display_modal_message ("Commander Saved.", path);

	set_commander_name (path);
	saved_cmdr = cmdr;
	saved_cmdr.ship_x = docked_planet.d;
	saved_cmdr.ship_y = docked_planet.b;
}

void load_commander_screen (void)
{
	char path[255];
	int rv;

	gfx_clear_display();
	gfx_display_centre_text (10, "LOAD COMMANDER", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);
	gfx_update_screen();
	
	strcpy (path, "jameson.nkc");
	
	rv = gfx_request_file ("Load Commander", path, "nkc");

	if (rv == 0)
		return;

	rv = load_commander_file (path);

	if (rv)
	{
		saved_cmdr = cmdr;
		gfx_display_modal_message ("Error Loading Commander!", "Could not load commander file.");
		return;
	}
	
	restore_saved_commander();
	set_commander_name (path);
	saved_cmdr = cmdr;
	update_console();
}

void run_first_intro_screen (void)
{
	current_screen = SCR_INTRO_ONE;

	snd_play_midi (SND_ELITE_THEME, 1);

	initialise_intro1();

	double last_intro1_time = GetTime();

	while (!WindowShouldClose())
	{
		double now = GetTime();
		double tick_interval = (double)speed_cap / 1000.0;
		if (tick_interval < 0.01) tick_interval = 0.075;

		if (now - last_intro1_time >= tick_interval)
		{
			update_intro1();
			last_intro1_time = now;
		}

		gfx_update_screen();

		kbd_poll_keyboard();

		if (kbd_y_pressed)
		{
			snd_stop_midi();	
			load_commander_screen();
			break;
		}
		
		if (kbd_n_pressed)
		{ 
			snd_stop_midi();	
			break;
		}
	} 
}

void run_second_intro_screen (void)
{
	current_screen = SCR_INTRO_TWO;
	
	snd_play_midi (SND_BLUE_DANUBE, 1);
		
	initialise_intro2();

	flight_speed = 3;
	flight_roll = 0;
	flight_climb = 0;

	double last_intro2_time = GetTime();

	while (!WindowShouldClose())
	{
		double now = GetTime();
		double tick_interval = (double)speed_cap / 1000.0;
		if (tick_interval < 0.01) tick_interval = 0.075;

		if (now - last_intro2_time >= tick_interval)
		{
			update_intro2();
			last_intro2_time = now;
		}

		gfx_update_screen();

		kbd_poll_keyboard();

		if (kbd_space_pressed) 
			break;
	} 

	snd_stop_midi();
}

void run_game_over_screen(void)
{
	int i;
	int newship;
	EliteMatrix rotmat;
	int type;
	
	current_screen = SCR_GAME_OVER;
	gfx_set_clip_region (1, 1, 510, 383);
	
	flight_speed = 6;
	flight_roll = 0;
	flight_climb = 0;
	clear_universe();

	set_init_matrix (rotmat);

	newship = add_new_ship (SHIP_COBRA3, 0, 0, -400, rotmat, 0, 0);
	universe[newship].flags |= FLG_DEAD;

	for (i = 0; i < 5; i++)
	{
		type = (rand255() & 1) ? SHIP_CARGO : SHIP_ALLOY;
		newship = add_new_ship (type, (rand255() & 63) - 32,
								(rand255() & 63) - 32, -400, rotmat, 0, 0);
		universe[newship].rotz = ((rand255() * 2) & 255) - 128;
		universe[newship].rotx = ((rand255() * 2) & 255) - 128;
		universe[newship].velocity = rand255() & 15;
	}
	
	for (i = 0; i < 100 && !WindowShouldClose(); i++)
	{
		gfx_clear_display();
		update_starfield();
		update_universe();
		gfx_display_centre_text (190, "GAME OVER", 140, GFX_COL_GOLD);
		gfx_update_screen();
	}
}

void display_break_pattern (void)
{
	int i;

	gfx_set_clip_region (1, 1, 510, 383);
	gfx_clear_display();
	
	for (i = 0; i < 20; i++)
	{
		gfx_draw_circle (256, 192, 30 + i * 15, GFX_COL_WHITE);
		gfx_update_screen();
	}	

	if (docked)
	{
		check_mission_brief();
		display_commander_status();
		update_console();
	}
	else
		current_screen = SCR_FRONT_VIEW;
}

void initialise_hardware (void)
{
	have_joystick = 0;
}

int main(void)
{
	initialise_hardware();
	read_config_file();

	if (gfx_graphics_startup() == 1)
	{
		return 1;
	}
	
	snd_sound_startup();
	kbd_keyboard_startup();
	
	finish = 0;
	auto_pilot = 0;
	
	while (!finish && !WindowShouldClose())
	{
		game_over = 0;	
		initialise_game();
		dock_player();

		update_console();

		current_screen = SCR_FRONT_VIEW;
		run_first_intro_screen();
		run_second_intro_screen();

		dock_player ();
		display_commander_status ();
		
		double last_sim_time = GetTime();

		while (!game_over && !WindowShouldClose())
		{
			snd_update_sound();
			gfx_update_screen();
			gfx_set_clip_region (1, 1, 510, 383);

			rolling = 0;
			climbing = 0;
			yawing = 0;

			handle_flight_keys ();

			if (game_paused)
				continue;

			double now = GetTime();
			double tick_interval = (double)speed_cap / 1000.0;
			if (tick_interval < 0.01) tick_interval = 0.075;

			if (now - last_sim_time < tick_interval)
				continue;

			last_sim_time = now;
				
			if (message_count > 0)
				message_count--;

			if (flight_assist)
			{
				if (!rolling)
				{
					flight_roll_f *= 0.45;
					if (fabs(flight_roll_f) < 0.1)
						flight_roll_f = 0.0;
					flight_roll = (int)round(flight_roll_f);
				}

				if (!climbing)
				{
					flight_climb_f *= 0.45;
					if (fabs(flight_climb_f) < 0.1)
						flight_climb_f = 0.0;
					flight_climb = (int)round(flight_climb_f);
				}

				if (!yawing)
				{
					flight_yaw_f *= 0.45;
					if (fabs(flight_yaw_f) < 0.1)
						flight_yaw_f = 0.0;
					flight_yaw = (int)round(flight_yaw_f);
				}
			}
			else
			{
				if (!rolling)
				{
					if (flight_roll_f > 0) flight_roll_f -= 1.0;
					if (flight_roll_f < 0) flight_roll_f += 1.0;
					if (fabs(flight_roll_f) < 1.0) flight_roll_f = 0.0;
					flight_roll = (int)round(flight_roll_f);
				}

				if (!climbing)
				{
					if (flight_climb_f > 0) flight_climb_f -= 1.0;
					if (flight_climb_f < 0) flight_climb_f += 1.0;
					if (fabs(flight_climb_f) < 1.0) flight_climb_f = 0.0;
					flight_climb = (int)round(flight_climb_f);
				}

				if (!yawing)
				{
					if (flight_yaw_f > 0) flight_yaw_f -= 1.0;
					if (flight_yaw_f < 0) flight_yaw_f += 1.0;
					if (fabs(flight_yaw_f) < 1.0) flight_yaw_f = 0.0;
					flight_yaw = (int)round(flight_yaw_f);
				}
			}

			if (!docked)
			{
				gfx_acquire_screen();
					
				if ((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
					(current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW) ||
					(current_screen == SCR_INTRO_ONE) || (current_screen == SCR_INTRO_TWO) ||
					(current_screen == SCR_GAME_OVER))
				{
					gfx_clear_display();
					update_starfield();
				}

				if (auto_pilot)
				{
					auto_dock();
					if ((mcount & 127) == 0)
						info_message ("Docking Computers On");
				}

				update_universe ();

				if (docked)
				{
					update_console();
					gfx_release_screen();
					continue;
				}

				if ((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
					(current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW))
				{
					if (draw_lasers)
					{
						draw_laser_lines();
						draw_lasers--;
					}
					
					draw_laser_sights();
				}

				if (message_count > 0)
					gfx_display_centre_text (358, message_string, 120, GFX_COL_WHITE);
				else if (can_fast_dock)
					gfx_display_centre_text (358, "PRESS SHIFT-D TO DOCK", 120, GFX_COL_GOLD);
					
				if (hyper_ready)
				{
					display_hyper_status();
					if ((mcount & 3) == 0)
					{
						countdown_hyperspace();
					}
				}

				gfx_release_screen();
			
				mcount--;
				if (mcount < 0)
					mcount = 255;

				if ((mcount & 7) == 0)
					regenerate_shields();

				if ((mcount & 31) == 10)
				{
					if (myship_energy < 50)
					{
						info_message ("ENERGY LOW");
						snd_play_sample (SND_BEEP);
					}

					update_altitude();
				}
				
				if ((mcount & 31) == 20)
					update_cabin_temp();
					
				if ((mcount == 0) && (!witchspace))
					random_encounter();
					
				cool_laser();				
				time_ecm();

				update_console();
			}

			if (current_screen == SCR_BREAK_PATTERN)
				display_break_pattern();

			if (current_screen == SCR_GALACTIC_CHART)
			{
				if (cross_timer > 0)
				{
					cross_timer--;
					if (cross_timer == 0)
					{
						show_distance_to_planet();
					}
				}
				display_galactic_chart();
			}
			else if (current_screen == SCR_SHORT_RANGE)
			{
				if (cross_timer > 0)
				{
					cross_timer--;
					if (cross_timer == 0)
					{
						show_distance_to_planet();
					}
				}
				display_short_range_chart();
			}
		}

		if (!finish && !WindowShouldClose())		
			run_game_over_screen();
	}

	snd_sound_shutdown();
	gfx_graphics_shutdown ();
	
	return 0;
}
