/*
 * Elite - The New Kind.
 *
 * Game State Management Implementation (C23 Standard)
 */

#include <string.h>
#include "game_state.h"
#include "planet.h"
#include "trade.h"

GameState g_state = {
	.config = {
		.speed_cap = 75,
		.wireframe = 0,
		.anti_alias_gfx = 0,
		.planet_render_style = 0,
		.hoopy_casinos = 0,
		.instant_dock = 0,
		.control_scheme = 1,      /* 0 = Classic, 1 = Modern */
		.mouse_flight_mode = 0,   /* 0 = Direct, 1 = Virtual Stick, 2 = Off */
		.invert_pitch = 0,        /* 0 = Normal, 1 = Inverted */
		.mouse_sensitivity = 1,   /* 0 = Low, 1 = Medium, 2 = High */
		.flight_assist = 1,       /* 0 = Off, 1 = On */
		.aspect_ratio_mode = 0,   /* 0 = 1:1 Square, 1 = 4:3 Retro, 2 = 16:9 Wide, 3 = Integer, 4 = Stretch */
		.scaling_filter = 0,      /* 0 = Point (Crisp), 1 = Bilinear (Smooth) */
		.display_mode = 0,        /* 0 = Maximized, 1 = Fullscreen, 2 = 800x600, etc. */
		.scanner_filename = "scanner.bmp",
		.scanner_cx = 256,
		.scanner_cy = 385 + 23,
		.compass_centre_x = 382,
		.compass_centre_y = 385 + 22,
	},
	.player = {
		.saved = {
			.name = "JAMESON",
			.mission = 0,
			.ship_x = 0x14,
			.ship_y = 0xAD,
			.galaxy = {0x4a, 0x5a, 0x48, 0x02, 0x53, 0xb7},
			.credits = 1000,
			.fuel = 70,
			.unused1 = 0,
			.galaxy_number = 0,
			.front_laser = PULSE_LASER,
			.rear_laser = 0,
			.left_laser = 0,
			.right_laser = 0,
			.unused2 = 0,
			.unused3 = 0,
			.cargo_capacity = 20,
			.current_cargo = {0},
			.ecm = 0,
			.fuel_scoop = 0,
			.energy_bomb = 0,
			.energy_unit = 0,
			.docking_computer = 0,
			.galactic_hyperdrive = 0,
			.escape_pod = 0,
			.unused4 = 0,
			.unused5 = 0,
			.unused6 = 0,
			.unused7 = 0,
			.missiles = 3,
			.legal_status = 0,
			.station_stock = {
				0x10, 0x0F, 0x11, 0x00, 0x03, 0x1C,
				0x0E, 0x00, 0x00, 0x0A, 0x00, 0x11,
				0x3A, 0x07, 0x09, 0x08, 0x00
			},
			.market_rnd = 0,
			.score = 0,
			.saved = 0x80,
		},
		.current = {
			.name = {0},
			.mission = 0,
			.ship_x = 0,
			.ship_y = 0,
			.galaxy = {0, 0, 0, 0, 0, 0},
			.credits = 0,
			.fuel = 0,
			.unused1 = 0,
			.galaxy_number = 0,
			.front_laser = 0,
			.rear_laser = 0,
			.left_laser = 0,
			.right_laser = 0,
			.unused2 = 0,
			.unused3 = 0,
			.cargo_capacity = 0,
			.current_cargo = {0},
			.ecm = 0,
			.fuel_scoop = 0,
			.energy_bomb = 0,
			.energy_unit = 0,
			.docking_computer = 0,
			.galactic_hyperdrive = 0,
			.escape_pod = 0,
			.unused4 = 0,
			.unused5 = 0,
			.unused6 = 0,
			.unused7 = 0,
			.missiles = 0,
			.legal_status = 0,
			.station_stock = {0},
			.market_rnd = 0,
			.score = 0,
			.saved = 0,
		},
		.vitals = {
			.ship_specs = {0},
		}
	},
	.flight = {
		.roll_f = 0.0,
		.climb_f = 0.0,
		.yaw_f = 0.0,
	},
	.env = {
		.carry_flag = 0,
	},
	.session = {
		.current_screen = 0,
	}
};

void game_state_restore_saved_commander(void)
{
	g_state.player.current = g_state.player.saved;

	g_state.env.docked_planet = find_planet(g_state.player.current.ship_x, g_state.player.current.ship_y);
	g_state.env.hyperspace_planet = g_state.env.docked_planet;

	generate_planet_data(&g_state.env.current_planet_data, g_state.env.docked_planet);
	generate_stock_market();
	set_stock_quantities(g_state.player.current.station_stock);
}

void game_state_reset_flight(void)
{
	g_state.flight.speed = 0;
	g_state.flight.roll = 0;
	g_state.flight.climb = 0;
	g_state.flight.yaw = 0;
	g_state.flight.roll_f = 0.0;
	g_state.flight.climb_f = 0.0;
	g_state.flight.yaw_f = 0.0;
}

void game_state_reset_universe(void)
{
	for (int i = 0; i < MAX_UNIV_OBJECTS; i++)
	{
		g_state.env.objects[i].type = 0;
		g_state.env.objects[i].flags = 0;
	}
	for (int i = 0; i <= NO_OF_SHIPS; i++)
	{
		g_state.env.ship_count[i] = 0;
	}
}

void game_state_init(void)
{
	game_state_restore_saved_commander();
	game_state_reset_flight();
	game_state_reset_universe();
}
