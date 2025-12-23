/***********************************************************************
Dinosaur - Data structures for animated dinosaur sprites in the AR
Sandbox ecosystem simulation.
Copyright (c) 2024

This file is part of the Augmented Reality Sandbox (SARndbox).

The Augmented Reality Sandbox is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Augmented Reality Sandbox is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Augmented Reality Sandbox; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include "Dinosaur.h"

#include <cmath>

/* Species information table */
static const DinosaurSpeciesInfo speciesInfoTable[DINO_NUM_SPECIES] =
	{
	// DINO_TRICERATOPS - sturdy herbivore, herds together
		{
		"Triceratops",
		"triceratops",
		ROLE_HERBIVORE,
		0.015,   // walkSpeed - moderate
		0.035,   // runSpeed - decent escape speed
		0.15,    // sightRange - can see predators approaching
		0.0,     // attackRange - herbivore doesn't attack
		{15, 15, 15, 15, 15, 15} // frames per action (Idle, Walk, Run, Attack, Die, TakeDamage)
		},

	// DINO_STEGOSAURUS - slow, peaceful grazer
		{
		"Stegosaurus",
		"stegosaurus",
		ROLE_HERBIVORE,
		0.010,   // walkSpeed - slow
		0.025,   // runSpeed - not very fast
		0.12,    // sightRange
		0.0,     // attackRange
		{15, 15, 15, 15, 15, 15}
		},

	// DINO_PARASAUROLOPHUS - skittish runner
		{
		"Parasaurolophus",
		"parasaurolophus",
		ROLE_HERBIVORE,
		0.018,   // walkSpeed - quick walker
		0.045,   // runSpeed - very fast runner
		0.18,    // sightRange - alert, spots danger early
		0.0,     // attackRange
		{15, 15, 15, 15, 15, 15}
		},

	// DINO_GALLIMIMUS - extremely fast
		{
		"Gallimimus",
		"gallimimus",
		ROLE_HERBIVORE,
		0.022,   // walkSpeed - fast walker
		0.055,   // runSpeed - fastest herbivore
		0.20,    // sightRange - very alert
		0.0,     // attackRange
		{15, 15, 15, 15, 15, 15}
		},

	// DINO_TREX - slow but powerful predator
		{
		"T-Rex",
		"t_rex",
		ROLE_PREDATOR,
		0.012,   // walkSpeed - lumbering
		0.030,   // runSpeed - not as fast as it looks
		0.25,    // sightRange - excellent vision
		0.025,   // attackRange - big bite radius
		{15, 15, 15, 15, 15, 15}
		},

	// DINO_VELOCIRAPTOR - fast pack hunter
		{
		"Velociraptor",
		"velociraptor",
		ROLE_PREDATOR,
		0.020,   // walkSpeed - quick
		0.050,   // runSpeed - very fast
		0.18,    // sightRange
		0.015,   // attackRange - smaller
		{15, 15, 15, 15, 15, 15}
		},

	// DINO_RAPTOR_BLUE - fast pack hunter variant
		{
		"Blue Raptor",
		"blue_raptor",
		ROLE_PREDATOR,
		0.020,
		0.050,
		0.18,
		0.015,
		{15, 15, 15, 15, 15, 15}
		},

	// DINO_RAPTOR_GREEN - fast pack hunter variant
		{
		"Green Raptor",
		"green_raptor",
		ROLE_PREDATOR,
		0.020,
		0.050,
		0.18,
		0.015,
		{15, 15, 15, 15, 15, 15}
		},

	// DINO_RAPTOR_RED - fast pack hunter variant
		{
		"Red Raptor",
		"red_raptor",
		ROLE_PREDATOR,
		0.020,
		0.050,
		0.18,
		0.015,
		{15, 15, 15, 15, 15, 15}
		}
	};

/* Action name mapping for spritesheet filenames */
static const char* actionNames[ACTION_NUM_ACTIONS] =
	{
	"idle",
	"walk",
	"run",
	"attack1",  // Using attack1 as default attack animation
	"die",
	"takedamage"
	};

const DinosaurSpeciesInfo& getSpeciesInfo(DinosaurSpecies species)
	{
	return speciesInfoTable[species];
	}

DinosaurDirection calculateDirection(const Vector& velocity)
	{
	/* Handle zero velocity - keep current direction */
	Scalar speed = Geometry::mag(velocity);
	if(speed < 1.0e-6)
		return DIR_S; // Default facing south

	/* Calculate angle from velocity vector
	   Note: In sandbox coordinates, +Y is typically "up" on the projection,
	   and +X is to the right. Angle 0 = East, 90 = North, etc. */
	double angle = std::atan2(velocity[1], velocity[0]) * 180.0 / M_PI;

	/* Normalize to 0-360 range */
	if(angle < 0.0)
		angle += 360.0;

	/* Map to 8 directions (each direction covers 45 degrees)
	   E=0, NE=45, N=90, NW=135, W=180, SW=225, S=270, SE=315 */
	int dirIndex = static_cast<int>((angle + 22.5) / 45.0) % 8;

	/* Convert from angle-based index to our direction enum
	   Angle order: E, NE, N, NW, W, SW, S, SE
	   Our enum:    N, NE, E, SE, S, SW, W, NW */
	static const DinosaurDirection angleToDir[8] =
		{
		DIR_E,   // 0 degrees
		DIR_NE,  // 45 degrees
		DIR_N,   // 90 degrees
		DIR_NW,  // 135 degrees
		DIR_W,   // 180 degrees
		DIR_SW,  // 225 degrees
		DIR_S,   // 270 degrees
		DIR_SE   // 315 degrees
		};

	return angleToDir[dirIndex];
	}

std::string getSpritesheetPath(DinosaurSpecies species, DinosaurAction action)
	{
	const DinosaurSpeciesInfo& info = speciesInfoTable[species];

	/* Build relative path: <Species>/<Action>_Shadowless.png
	   DinosaurRenderer prepends CONFIG_SPRITEDIR */
	std::string path = info.spritePath;
	path += "/";
	path += actionNames[action];
	path += "_Shadowless.png";

	return path;
	}
