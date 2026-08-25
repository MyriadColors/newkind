/*
 * Elite - The New Kind.
 *
 * Game State Management System (C23 Standard)
 * Encapsulates scattered global variables into structured domain contexts.
 */

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "planet.h"
#include "trade.h"
#include "vector.h"
#include "shipdata.h"

#define MAX_UNIV_OBJECTS 20

#define PULSE_LASER		0x0F
#define BEAM_LASER		0x8F
#define MILITARY_LASER	0x97
#define MINING_LASER	0x32

#define FLG_DEAD			(1)
#define	FLG_REMOVE			(2)
#define FLG_EXPLOSION		(4)
#define FLG_ANGRY			(8)
#define FLG_FIRING			(16)
#define FLG_HAS_ECM			(32)
#define FLG_HOSTILE			(64)
#define FLG_CLOAKED			(128)
#define FLG_FLY_TO_PLANET	(256)
#define FLG_FLY_TO_STATION	(512)
#define FLG_INACTIVE		(1024)
#define FLG_SLOW			(2048)
#define FLG_BOLD			(4096)
#define FLG_POLICE			(8192)

struct point
{
	int x;
	int y;
	int z;
};

struct univ_object
{
	int type;
	Vector location;
	EliteMatrix rotmat;
	int rotx;
	int rotz;
	int flags;
	int energy;
	int velocity;
	int acceleration;
	int missiles;
	int target;
	int bravery;
	int exp_delta;
	int exp_seed;
	int distance;
};

struct commander
{
	char name[32];
	int mission;
	int ship_x;
	int ship_y;
	struct galaxy_seed galaxy;
	int credits;
	int fuel;
	int unused1;
	int galaxy_number;
	int front_laser;
	int rear_laser;
	int left_laser;
	int right_laser;
	int unused2;
	int unused3;
	int cargo_capacity;
	int current_cargo[NO_OF_STOCK_ITEMS];
	int ecm;
	int fuel_scoop;
	int energy_bomb;
	int energy_unit;
	int docking_computer;
	int galactic_hyperdrive;
	int escape_pod;
	int unused4;
	int unused5;
	int unused6;
	int unused7;
	int missiles;
	int legal_status;
	int station_stock[NO_OF_STOCK_ITEMS];
	int market_rnd;
	int score;
	int saved;
};

struct player_ship
{
	int max_speed;
	int max_roll;
	int max_climb;
	int max_fuel;
	int altitude;
	int cabtemp;
};

/* 1. Configuration & User Preferences */
typedef struct config_state
{
	int speed_cap;
	int wireframe;
	int anti_alias_gfx;
	int planet_render_style;
	int hoopy_casinos;
	int instant_dock;
	int control_scheme;
	int mouse_flight_mode;
	int invert_pitch;
	int mouse_sensitivity;
	int flight_assist;
	int aspect_ratio_mode;
	int scaling_filter;
	int display_mode;
	int tactical_hud;
	char scanner_filename[256];
	int scanner_cx;
	int scanner_cy;
	int compass_centre_x;
	int compass_centre_y;
} ConfigState;

/* 2. Player Ship & Live Vitals */
typedef struct player_vitals
{
	int front_shield;
	int aft_shield;
	int energy;
	int laser_temp;
	int mcount;
	int detonate_bomb;
	int witchspace;
	int auto_pilot;
	int hyper_ready;
	int can_fast_dock;
	struct player_ship ship_specs;
} PlayerVitals;

typedef struct player_state
{
	struct commander current;
	struct commander saved;
	PlayerVitals vitals;
} PlayerState;

/* 3. Flight Dynamics & Motion */
typedef struct flight_state
{
	int speed;
	int roll;
	int climb;
	int yaw;
	double roll_f;
	double climb_f;
	double yaw_f;
} FlightState;

/* 4. Universe & Environmental Entities */
typedef struct universe_state
{
	struct galaxy_seed docked_planet;
	struct galaxy_seed hyperspace_planet;
	struct planet_data current_planet_data;
	struct univ_object objects[MAX_UNIV_OBJECTS];
	int ship_count[NO_OF_SHIPS + 1];
	int carry_flag;
} UniverseState;

/* 5. Game Session & UI Flow */
typedef struct session_state
{
	int current_screen;
	int is_docked;
	int is_finish;
	int is_game_over;
} SessionState;

/* Unified Game State Root */
typedef struct game_state
{
	ConfigState config;
	PlayerState player;
	FlightState flight;
	UniverseState env;
	SessionState session;
} GameState;

/* Global Master Singleton Instance */
extern GameState g_state;

/* C23 Accessor API with [[nodiscard]] */
[[nodiscard]] static inline GameState *get_game_state(void)
{
	return &g_state;
}

[[nodiscard]] static inline ConfigState *get_config_state(void)
{
	return &g_state.config;
}

[[nodiscard]] static inline PlayerState *get_player_state(void)
{
	return &g_state.player;
}

[[nodiscard]] static inline FlightState *get_flight_state(void)
{
	return &g_state.flight;
}

[[nodiscard]] static inline UniverseState *get_universe_state(void)
{
	return &g_state.env;
}

[[nodiscard]] static inline SessionState *get_session_state(void)
{
	return &g_state.session;
}

/* State Management Operations */
void game_state_init(void);
void game_state_restore_saved_commander(void);
void game_state_reset_flight(void);
void game_state_reset_universe(void);

#endif /* GAME_STATE_H */
