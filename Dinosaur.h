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

#ifndef DINOSAUR_INCLUDED
#define DINOSAUR_INCLUDED

#include <string>
#include "Types.h"

/* Enumeration for dinosaur species */
enum DinosaurSpecies
	{
	DINO_TRICERATOPS=0,
	DINO_STEGOSAURUS,
	DINO_PARASAUROLOPHUS,
	DINO_GALLIMIMUS,
	DINO_TREX,
	DINO_VELOCIRAPTOR,
	DINO_RAPTOR_BLUE,
	DINO_RAPTOR_GREEN,
	DINO_RAPTOR_RED,
	DINO_NUM_SPECIES
	};

/* Enumeration for behavior role */
enum DinosaurRole
	{
	ROLE_HERBIVORE=0,
	ROLE_PREDATOR
	};

/* Enumeration for animation actions */
enum DinosaurAction
	{
	ACTION_IDLE=0,
	ACTION_WALK,
	ACTION_RUN,
	ACTION_ATTACK,
	ACTION_DIE,
	ACTION_TAKEDAMAGE,
	ACTION_NUM_ACTIONS
	};

/* Enumeration for 8 movement directions */
enum DinosaurDirection
	{
	DIR_N=0,   // North (up)
	DIR_NE,    // Northeast
	DIR_E,     // East (right)
	DIR_SE,    // Southeast
	DIR_S,     // South (down)
	DIR_SW,    // Southwest
	DIR_W,     // West (left)
	DIR_NW,    // Northwest
	DIR_NUM_DIRECTIONS
	};

/* Enumeration for AI states */
enum DinosaurAIState
	{
	AI_IDLE=0,        // Standing still
	AI_WANDERING,     // Moving to random target
	AI_GRAZING,       // Herbivore eating (idle animation)
	AI_FLEEING,       // Running from threat (predator, hand, lava)
	AI_HUNTING,       // Predator chasing prey
	AI_ATTACKING,     // Predator attacking prey
	AI_DYING,         // Playing death animation
	AI_DEAD           // Waiting for respawn
	};

/* Structure for species-specific parameters */
struct DinosaurSpeciesInfo
	{
	const char* name;              // Display name
	const char* spritePath;        // Path to spritesheet folder
	DinosaurRole role;             // Herbivore or predator
	Scalar walkSpeed;              // Normal walking speed (world units/sec)
	Scalar runSpeed;               // Running/fleeing speed
	Scalar sightRange;             // Distance to detect threats/prey
	Scalar attackRange;            // Distance to trigger attack
	int framesPerAction[ACTION_NUM_ACTIONS]; // Animation frame counts
	};

/* Main dinosaur entity structure */
struct Dinosaur
	{
	/* Identity */
	DinosaurSpecies species;       // What kind of dinosaur
	unsigned int id;               // Unique identifier

	/* Position and movement */
	Point position;                // Current 3D position (x, y, elevation)
	Vector velocity;               // Current velocity vector
	Point targetPosition;          // Where we're trying to go
	Scalar targetElevation;        // Terrain height at current position

	/* Animation state */
	DinosaurAction currentAction;  // Current animation (walk, run, etc.)
	DinosaurDirection direction;   // Facing direction (0-7)
	int currentFrame;              // Current animation frame
	float animationTimer;          // Time accumulator for animation
	float frameTime;               // Seconds per frame

	/* AI state */
	DinosaurAIState aiState;       // Current behavior state
	unsigned int targetDinoId;     // ID of dinosaur being chased/fled from
	float stateTimer;              // Time in current state
	float respawnTimer;            // Countdown to respawn after death

	/* Flags */
	bool isAlive;                  // False when dead/waiting for respawn
	bool isVisible;                // For fade in/out effects
	float alpha;                   // Opacity for fade effects
	};

/* Helper functions */

/* Get species info for a given species */
const DinosaurSpeciesInfo& getSpeciesInfo(DinosaurSpecies species);

/* Calculate direction enum from velocity vector */
DinosaurDirection calculateDirection(const Vector& velocity);

/* Get the spritesheet filename for an action */
std::string getSpritesheetPath(DinosaurSpecies species, DinosaurAction action);

/* Check if a species is a predator */
inline bool isPredator(DinosaurSpecies species)
	{
	return getSpeciesInfo(species).role == ROLE_PREDATOR;
	}

/* Check if a species is an herbivore */
inline bool isHerbivore(DinosaurSpecies species)
	{
	return getSpeciesInfo(species).role == ROLE_HERBIVORE;
	}

#endif
