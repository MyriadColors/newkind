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

#ifndef PLANET_H
#define PLANET_H

#include <stdint.h>

struct galaxy_seed
{
	uint8_t a;	/* 6c */
	uint8_t b;	/* 6d */
	uint8_t c;	/* 6e */
	uint8_t d;	/* 6f */
	uint8_t e;	/* 70 */
	uint8_t f;	/* 71 */
};


struct planet_data
{
	int government;
	int economy;
	int techlevel;
	int population;
	int productivity;
	int radius;
};


const char *describe_planet (struct galaxy_seed);
void capitalise_name (char *name);
void name_planet (char *gname, struct galaxy_seed glx);
struct galaxy_seed find_planet (int cx, int cy);
int find_planet_number (struct galaxy_seed planet);
void waggle_galaxy (struct galaxy_seed *glx_ptr);
void describe_inhabitants (char *str, struct galaxy_seed planet);
void generate_planet_data (struct planet_data *pl, struct galaxy_seed planet_seed);
void set_current_planet (struct galaxy_seed new_planet);

#endif

