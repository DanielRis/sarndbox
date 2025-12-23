/***********************************************************************
DinosaurEcosystem - Class to manage the dinosaur population and AI
behaviors in the AR Sandbox ecosystem simulation.
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

#ifndef DINOSAURECOSYSTEM_INCLUDED
#define DINOSAURECOSYSTEM_INCLUDED

#include <vector>
#include <random>

#include "Types.h"
#include "Dinosaur.h"

/* Forward declarations */
class WaterTable2;
class DepthImageRenderer;

class DinosaurEcosystem
	{
	/* Embedded classes: */
	public:

	/* Structure for sandbox bounds */
	struct Bounds
		{
		Scalar minX, maxX;
		Scalar minY, maxY;
		Scalar minZ, maxZ;  // Elevation range
		};

	/* Terrain query result */
	struct TerrainInfo
		{
		Scalar elevation;     // Terrain height
		Scalar waterDepth;    // Water depth (0 if no water)
		bool isLava;          // True if below lava threshold
		};

	private:

	/* Elements: */
	const WaterTable2* waterTable;           // For terrain/water queries
	const DepthImageRenderer* depthRenderer; // For terrain height sampling
	Bounds bounds;                       // Sandbox boundaries
	std::vector<Dinosaur> dinosaurs;     // All dinosaur instances
	unsigned int nextDinosaurId;         // For unique IDs

	/* Random number generation */
	std::mt19937 rng;
	std::uniform_real_distribution<float> randomFloat;

	/* Simulation parameters */
	Scalar lavaElevationThreshold;       // Elevation below which is "lava"
	Scalar waterLevelThreshold;          // Elevation below which water pools
	Scalar waterAvoidanceDepth;          // Water depth to start avoiding
	Scalar handFleeRadius;               // Distance to flee from hands
	Scalar predatorSightRange;           // How far predators can see prey
	Scalar fleeDistance;                 // How far to flee before stopping
	float respawnDelay;                  // Seconds before respawn after death
	float animationSpeed;                // Animation frames per second
	Scalar speedScale;                   // Movement speed multiplier (scales with -dino parameter)

	/* Hand detection data (updated externally) */
	std::vector<Point> detectedHands;

	/* Private methods: */

	/* Spawn a dinosaur at a random valid position */
	void spawnDinosaurRandom(DinosaurSpecies species);

	/* Find a valid spawn position avoiding water and lava */
	Point findValidSpawnPosition(void);

	/* Query terrain at a position */
	TerrainInfo queryTerrain(const Point& pos) const;

	/* Update a single dinosaur's AI */
	void updateDinosaurAI(Dinosaur& dino, float deltaTime);

	/* Update dinosaur animation */
	void updateDinosaurAnimation(Dinosaur& dino, float deltaTime);

	/* Update dinosaur movement */
	void updateDinosaurMovement(Dinosaur& dino, float deltaTime);

	/* Find nearest threat (predator, hand, or lava) for herbivore */
	bool findNearestThreat(const Dinosaur& dino, Point& threatPos, Scalar& distance) const;

	/* Find nearest prey for predator */
	bool findNearestPrey(const Dinosaur& predator, unsigned int& preyId, Scalar& distance) const;

	/* Check if position is safe (no water, no lava) */
	bool isPositionSafe(const Point& pos) const;

	/* Steer away from hazards (water, lava, bounds) */
	Vector calculateAvoidanceVector(const Dinosaur& dino) const;

	/* Calculate herd center for herbivore */
	Point calculateHerdCenter(const Dinosaur& dino) const;

	/* Choose a random wander target */
	Point chooseWanderTarget(const Dinosaur& dino);

	public:

	/* Constructors and destructors: */
	DinosaurEcosystem(const WaterTable2* sWaterTable);
	~DinosaurEcosystem(void);

	/* Methods: */

	/* Set the sandbox bounds (call after calibration) */
	void setBounds(const Bounds& newBounds);

	/* Set lava elevation threshold */
	void setLavaThreshold(Scalar threshold);

	/* Set water level threshold (elevation below which water pools) */
	void setWaterLevelThreshold(Scalar threshold);

	/* Set the depth image renderer for terrain height sampling */
	void setDepthImageRenderer(const DepthImageRenderer* renderer);

	/* Set movement speed scale (should match sprite scale) */
	void setSpeedScale(Scalar scale);

	/* Spawn initial population */
	void spawnInitialPopulation(void);

	/* Spawn a specific dinosaur at a position */
	unsigned int spawnDinosaur(DinosaurSpecies species, const Point& position);

	/* Update all dinosaurs (call every frame) */
	void update(float deltaTime);

	/* Update hand positions for flee behavior */
	void setDetectedHands(const std::vector<Point>& hands);

	/* Get all dinosaurs for rendering */
	const std::vector<Dinosaur>& getDinosaurs(void) const { return dinosaurs; }

	/* Get number of alive dinosaurs */
	unsigned int getAliveCount(void) const;

	/* Get number of dinosaurs by role */
	unsigned int getHerbivoreCount(void) const;
	unsigned int getPredatorCount(void) const;
	};

#endif
