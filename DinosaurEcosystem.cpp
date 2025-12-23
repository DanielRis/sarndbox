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

#include "DinosaurEcosystem.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <chrono>

#include "WaterTable2.h"
#include "DepthImageRenderer.h"

/**********************************
Methods of class DinosaurEcosystem:
**********************************/

DinosaurEcosystem::DinosaurEcosystem(const WaterTable2* sWaterTable)
	:waterTable(sWaterTable),
	 depthRenderer(0),
	 nextDinosaurId(0),
	 randomFloat(0.0f, 1.0f),
	 lavaElevationThreshold(-10.0),  // Below this is lava
	 waterAvoidanceDepth(0.5),       // Avoid water deeper than this
	 handFleeRadius(0.15),           // Flee from hands within this radius
	 predatorSightRange(0.20),       // Predators can see this far
	 fleeDistance(0.25),             // Flee this far before calming down
	 respawnDelay(8.0f),             // Respawn after 8 seconds
	 animationSpeed(12.0f)           // 12 frames per second
	{
	/* Initialize random number generator with time-based seed */
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	rng.seed(seed);

	/* Set default bounds (will be updated later) */
	bounds.minX = -0.5;
	bounds.maxX = 0.5;
	bounds.minY = -0.4;
	bounds.maxY = 0.4;
	bounds.minZ = -20.0;
	bounds.maxZ = 100.0;
	}

DinosaurEcosystem::~DinosaurEcosystem(void)
	{
	}

void DinosaurEcosystem::setBounds(const Bounds& newBounds)
	{
	bounds = newBounds;
	}

void DinosaurEcosystem::setLavaThreshold(Scalar threshold)
	{
	lavaElevationThreshold = threshold;
	}

void DinosaurEcosystem::setDepthImageRenderer(const DepthImageRenderer* renderer)
	{
	depthRenderer = renderer;
	}

DinosaurEcosystem::TerrainInfo DinosaurEcosystem::queryTerrain(const Point& pos) const
	{
	/* Debug: limit output to first N calls */
	static int debugCount = 0;
	bool doDebug = (debugCount < 10);
	if(doDebug) ++debugCount;

	TerrainInfo info;
	info.elevation = 0.0;
	info.waterDepth = 0.0;
	info.isLava = false;

	if(doDebug)
		{
		std::cout << "queryTerrain: pos=(" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")" << std::endl;
		if(waterTable != 0)
			{
			const WaterTable2::Box& domain = waterTable->getDomain();
			std::cout << "  waterTable domain: X[" << domain.min[0] << " to " << domain.max[0] << "]"
			          << " Y[" << domain.min[1] << " to " << domain.max[1] << "]"
			          << " Z[" << domain.min[2] << " to " << domain.max[2] << "]" << std::endl;
			}
		std::cout << "  depthRenderer=" << (depthRenderer != 0 ? "yes" : "no") << std::endl;
		}

	/* Try to get actual terrain height from depth image */
	if(depthRenderer != 0)
		{
		info.elevation = depthRenderer->getHeightAt(pos[0], pos[1]);
		if(doDebug)
			std::cout << "  -> from depthRenderer: elevation=" << info.elevation << std::endl;
		}
	else if(waterTable != 0)
		{
		/* Fallback: use domain midpoint if no depth renderer */
		const WaterTable2::Box& domain = waterTable->getDomain();
		info.elevation = (domain.min[2] + domain.max[2]) * 0.5;
		if(doDebug)
			std::cout << "  -> from waterTable fallback: elevation=" << info.elevation << std::endl;
		}

	/* Check if below lava threshold */
	info.isLava = (info.elevation < lavaElevationThreshold);

	/* Water depth would need to be queried from water quantity texture
	   For now, estimate based on low areas */
	if(info.elevation < lavaElevationThreshold + 5.0)
		info.waterDepth = 0.0; // Lava, not water
	else
		info.waterDepth = 0.0; // TODO: Query actual water level

	return info;
	}

bool DinosaurEcosystem::isPositionSafe(const Point& pos) const
	{
	/* Check bounds */
	if(pos[0] < bounds.minX || pos[0] > bounds.maxX ||
	   pos[1] < bounds.minY || pos[1] > bounds.maxY)
		return false;

	/* Query terrain */
	TerrainInfo terrain = queryTerrain(pos);

	/* Unsafe if lava or deep water */
	if(terrain.isLava)
		return false;
	if(terrain.waterDepth > waterAvoidanceDepth)
		return false;

	return true;
	}

Point DinosaurEcosystem::findValidSpawnPosition(void)
	{
	std::cout << "findValidSpawnPosition: bounds X[" << bounds.minX << " to " << bounds.maxX << "]"
	          << " Y[" << bounds.minY << " to " << bounds.maxY << "]" << std::endl;

	/* Try random positions until we find a safe one */
	for(int attempts = 0; attempts < 100; ++attempts)
		{
		Point pos;
		pos[0] = bounds.minX + randomFloat(rng) * (bounds.maxX - bounds.minX);
		pos[1] = bounds.minY + randomFloat(rng) * (bounds.maxY - bounds.minY);
		pos[2] = 0.0; // Will be updated from terrain query

		/* Get actual terrain height at this position */
		TerrainInfo terrain = queryTerrain(pos);
		pos[2] = terrain.elevation;

		if(isPositionSafe(pos))
			{
			std::cout << "  -> FOUND spawn pos: (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")" << std::endl;
			return pos;
			}
		}

	/* Fallback to center if no safe position found */
	Point center;
	center[0] = (bounds.minX + bounds.maxX) * 0.5;
	center[1] = (bounds.minY + bounds.maxY) * 0.5;
	center[2] = 0.0;

	/* Get terrain height at center */
	TerrainInfo terrain = queryTerrain(center);
	center[2] = terrain.elevation;

	std::cout << "  -> FALLBACK center: (" << center[0] << ", " << center[1] << ", " << center[2] << ")" << std::endl;
	return center;
	}

void DinosaurEcosystem::spawnDinosaurRandom(DinosaurSpecies species)
	{
	Point pos = findValidSpawnPosition();
	spawnDinosaur(species, pos);
	}

unsigned int DinosaurEcosystem::spawnDinosaur(DinosaurSpecies species, const Point& position)
	{
	Dinosaur dino;

	/* Set identity */
	dino.species = species;
	dino.id = nextDinosaurId++;

	/* Set position */
	dino.position = position;
	dino.velocity = Vector(0.0, 0.0, 0.0);
	dino.targetPosition = position;
	dino.targetElevation = position[2];

	/* Set animation state */
	dino.currentAction = ACTION_IDLE;
	dino.direction = static_cast<DinosaurDirection>(int(randomFloat(rng) * 8) % 8);
	dino.currentFrame = 0;
	dino.animationTimer = 0.0f;
	dino.frameTime = 1.0f / animationSpeed;

	/* Set AI state */
	dino.aiState = AI_IDLE;
	dino.targetDinoId = 0;
	dino.stateTimer = randomFloat(rng) * 2.0f; // Stagger initial behaviors
	dino.respawnTimer = 0.0f;

	/* Set flags */
	dino.isAlive = true;
	dino.isVisible = true;
	dino.alpha = 1.0f;

	dinosaurs.push_back(dino);

	const DinosaurSpeciesInfo& info = getSpeciesInfo(species);
	std::cout << "DinosaurEcosystem: Spawned " << info.name
	          << " #" << dino.id << " at ("
	          << position[0] << ", " << position[1] << ")" << std::endl;

	return dino.id;
	}

void DinosaurEcosystem::spawnInitialPopulation(void)
	{
	std::cout << "DinosaurEcosystem: Spawning initial population..." << std::endl;

	/* Herbivores */
	for(int i = 0; i < 5; ++i)
		spawnDinosaurRandom(DINO_TRICERATOPS);
	for(int i = 0; i < 3; ++i)
		spawnDinosaurRandom(DINO_STEGOSAURUS);
	for(int i = 0; i < 4; ++i)
		spawnDinosaurRandom(DINO_PARASAUROLOPHUS);
	for(int i = 0; i < 3; ++i)
		spawnDinosaurRandom(DINO_GALLIMIMUS);

	/* Predators */
	for(int i = 0; i < 2; ++i)
		spawnDinosaurRandom(DINO_TREX);
	for(int i = 0; i < 4; ++i)
		spawnDinosaurRandom(DINO_VELOCIRAPTOR);

	/* Add some variety with colored raptors */
	spawnDinosaurRandom(DINO_RAPTOR_BLUE);
	spawnDinosaurRandom(DINO_RAPTOR_RED);

	std::cout << "DinosaurEcosystem: Spawned " << dinosaurs.size()
	          << " dinosaurs" << std::endl;
	}

void DinosaurEcosystem::setDetectedHands(const std::vector<Point>& hands)
	{
	detectedHands = hands;
	}

bool DinosaurEcosystem::findNearestThreat(const Dinosaur& dino, Point& threatPos, Scalar& distance) const
	{
	distance = 999999.0;
	bool foundThreat = false;

	/* Check for nearby predators */
	for(const Dinosaur& other : dinosaurs)
		{
		if(!other.isAlive || other.id == dino.id)
			continue;

		if(isPredator(other.species))
			{
			Vector diff = other.position - dino.position;
			Scalar dist = Geometry::mag(diff);
			const DinosaurSpeciesInfo& info = getSpeciesInfo(dino.species);

			if(dist < info.sightRange && dist < distance)
				{
				distance = dist;
				threatPos = other.position;
				foundThreat = true;
				}
			}
		}

	/* Check for nearby hands */
	for(const Point& hand : detectedHands)
		{
		Vector diff = hand - dino.position;
		Scalar dist = Geometry::mag(diff);

		if(dist < handFleeRadius && dist < distance)
			{
			distance = dist;
			threatPos = hand;
			foundThreat = true;
			}
		}

	/* Check for nearby lava */
	TerrainInfo terrain = queryTerrain(dino.position);
	if(terrain.isLava)
		{
		/* Find direction away from lowest point (center of sandbox usually) */
		threatPos = dino.position;
		threatPos[2] = lavaElevationThreshold;
		distance = 0.01;  // Very close threat!
		foundThreat = true;
		}

	return foundThreat;
	}

bool DinosaurEcosystem::findNearestPrey(const Dinosaur& predator, unsigned int& preyId, Scalar& distance) const
	{
	distance = 999999.0;
	bool foundPrey = false;

	const DinosaurSpeciesInfo& predInfo = getSpeciesInfo(predator.species);

	for(const Dinosaur& other : dinosaurs)
		{
		if(!other.isAlive || other.id == predator.id)
			continue;

		if(isHerbivore(other.species))
			{
			Vector diff = other.position - predator.position;
			Scalar dist = Geometry::mag(diff);

			if(dist < predInfo.sightRange && dist < distance)
				{
				distance = dist;
				preyId = other.id;
				foundPrey = true;
				}
			}
		}

	return foundPrey;
	}

Vector DinosaurEcosystem::calculateAvoidanceVector(const Dinosaur& dino) const
	{
	Vector avoidance(0.0, 0.0, 0.0);

	/* Avoid sandbox boundaries */
	Scalar boundaryMargin = 0.05;

	if(dino.position[0] < bounds.minX + boundaryMargin)
		avoidance[0] += 1.0;
	if(dino.position[0] > bounds.maxX - boundaryMargin)
		avoidance[0] -= 1.0;
	if(dino.position[1] < bounds.minY + boundaryMargin)
		avoidance[1] += 1.0;
	if(dino.position[1] > bounds.maxY - boundaryMargin)
		avoidance[1] -= 1.0;

	/* Avoid lava (check nearby positions) */
	Scalar checkDist = 0.03;
	for(int dx = -1; dx <= 1; ++dx)
		{
		for(int dy = -1; dy <= 1; ++dy)
			{
			if(dx == 0 && dy == 0)
				continue;

			Point checkPos = dino.position;
			checkPos[0] += dx * checkDist;
			checkPos[1] += dy * checkDist;

			TerrainInfo terrain = queryTerrain(checkPos);
			if(terrain.isLava)
				{
				avoidance[0] -= dx * 2.0;
				avoidance[1] -= dy * 2.0;
				}
			if(terrain.waterDepth > waterAvoidanceDepth)
				{
				avoidance[0] -= dx * 1.0;
				avoidance[1] -= dy * 1.0;
				}
			}
		}

	/* Normalize if non-zero */
	Scalar mag = Geometry::mag(avoidance);
	if(mag > 0.001)
		avoidance = avoidance / mag;

	return avoidance;
	}

Point DinosaurEcosystem::calculateHerdCenter(const Dinosaur& dino) const
	{
	Point center(0.0, 0.0, 0.0);
	int count = 0;

	for(const Dinosaur& other : dinosaurs)
		{
		if(!other.isAlive || other.id == dino.id)
			continue;

		/* Only herd with same species */
		if(other.species == dino.species)
			{
			Vector diff = other.position - dino.position;
			Scalar dist = Geometry::mag(diff);

			/* Only consider nearby herd members */
			if(dist < 0.15)
				{
				center[0] += other.position[0];
				center[1] += other.position[1];
				center[2] += other.position[2];
				++count;
				}
			}
		}

	if(count > 0)
		{
		center[0] /= count;
		center[1] /= count;
		center[2] /= count;
		return center;
		}

	/* No herd members nearby, return current position */
	return dino.position;
	}

Point DinosaurEcosystem::chooseWanderTarget(const Dinosaur& dino)
	{
	/* Choose a random point within wander radius */
	Scalar wanderRadius = 0.15;

	for(int attempts = 0; attempts < 20; ++attempts)
		{
		Scalar angle = randomFloat(rng) * 2.0 * M_PI;
		Scalar dist = randomFloat(rng) * wanderRadius;

		Point target;
		target[0] = dino.position[0] + std::cos(angle) * dist;
		target[1] = dino.position[1] + std::sin(angle) * dist;
		target[2] = dino.position[2];

		if(isPositionSafe(target))
			return target;
		}

	/* Fallback: move toward center */
	Point center;
	center[0] = (bounds.minX + bounds.maxX) * 0.5;
	center[1] = (bounds.minY + bounds.maxY) * 0.5;
	center[2] = dino.position[2];
	return center;
	}

void DinosaurEcosystem::updateDinosaurAI(Dinosaur& dino, float deltaTime)
	{
	if(!dino.isAlive)
		{
		/* Handle respawn timer */
		if(dino.aiState == AI_DEAD)
			{
			dino.respawnTimer -= deltaTime;
			if(dino.respawnTimer <= 0.0f)
				{
				/* Respawn at new location */
				Point newPos = findValidSpawnPosition();
				dino.position = newPos;
				dino.isAlive = true;
				dino.isVisible = true;
				dino.alpha = 1.0f;
				dino.aiState = AI_IDLE;
				dino.currentAction = ACTION_IDLE;
				dino.stateTimer = 0.0f;

				const DinosaurSpeciesInfo& info = getSpeciesInfo(dino.species);
				std::cout << "DinosaurEcosystem: " << info.name
				          << " #" << dino.id << " respawned!" << std::endl;
				}
			}
		return;
		}

	dino.stateTimer += deltaTime;

	const DinosaurSpeciesInfo& info = getSpeciesInfo(dino.species);

	if(isHerbivore(dino.species))
		{
		/* Herbivore AI */
		Point threatPos;
		Scalar threatDist;

		if(findNearestThreat(dino, threatPos, threatDist))
			{
			/* Threat detected - FLEE! */
			dino.aiState = AI_FLEEING;
			dino.currentAction = ACTION_RUN;

			/* Run away from threat */
			Vector fleeDir = dino.position - threatPos;
			Scalar mag = Geometry::mag(fleeDir);
			if(mag > 0.001)
				fleeDir = fleeDir / mag;

			/* Add some randomness to flee direction */
			fleeDir[0] += (randomFloat(rng) - 0.5) * 0.3;
			fleeDir[1] += (randomFloat(rng) - 0.5) * 0.3;
			mag = Geometry::mag(fleeDir);
			if(mag > 0.001)
				fleeDir = fleeDir / mag;

			dino.velocity = fleeDir * info.runSpeed;
			dino.stateTimer = 0.0f;
			}
		else if(dino.aiState == AI_FLEEING)
			{
			/* Continue fleeing for a bit after threat disappears */
			if(dino.stateTimer > 2.0f)
				{
				dino.aiState = AI_WANDERING;
				dino.currentAction = ACTION_WALK;
				dino.targetPosition = chooseWanderTarget(dino);
				dino.stateTimer = 0.0f;
				}
			}
		else if(dino.aiState == AI_IDLE)
			{
			/* Occasionally start grazing or wandering */
			if(dino.stateTimer > 1.0f + randomFloat(rng) * 3.0f)
				{
				if(randomFloat(rng) < 0.3f)
					{
					/* Start grazing */
					dino.aiState = AI_GRAZING;
					dino.currentAction = ACTION_IDLE;
					dino.velocity = Vector(0.0, 0.0, 0.0);
					}
				else
					{
					/* Start wandering */
					dino.aiState = AI_WANDERING;
					dino.currentAction = ACTION_WALK;
					dino.targetPosition = chooseWanderTarget(dino);

					/* Consider herd - bias toward herd center */
					Point herdCenter = calculateHerdCenter(dino);
					dino.targetPosition[0] = dino.targetPosition[0] * 0.6 + herdCenter[0] * 0.4;
					dino.targetPosition[1] = dino.targetPosition[1] * 0.6 + herdCenter[1] * 0.4;
					}
				dino.stateTimer = 0.0f;
				}
			}
		else if(dino.aiState == AI_GRAZING)
			{
			/* Graze for a while then wander */
			if(dino.stateTimer > 2.0f + randomFloat(rng) * 4.0f)
				{
				dino.aiState = AI_WANDERING;
				dino.currentAction = ACTION_WALK;
				dino.targetPosition = chooseWanderTarget(dino);
				dino.stateTimer = 0.0f;
				}
			}
		else if(dino.aiState == AI_WANDERING)
			{
			/* Move toward target */
			Vector toTarget = dino.targetPosition - dino.position;
			Scalar distToTarget = Geometry::mag(toTarget);

			if(distToTarget < 0.02)
				{
				/* Reached target, become idle */
				dino.aiState = AI_IDLE;
				dino.currentAction = ACTION_IDLE;
				dino.velocity = Vector(0.0, 0.0, 0.0);
				dino.stateTimer = 0.0f;
				}
			else
				{
				/* Move toward target */
				toTarget = toTarget / distToTarget;
				dino.velocity = toTarget * info.walkSpeed;
				}
			}
		}
	else
		{
		/* Predator AI */
		unsigned int preyId;
		Scalar preyDist;

		/* First check for lava/water - predators also flee these */
		TerrainInfo terrain = queryTerrain(dino.position);
		if(terrain.isLava)
			{
			dino.aiState = AI_FLEEING;
			dino.currentAction = ACTION_RUN;

			/* Run toward center (away from lava) */
			Point center;
			center[0] = (bounds.minX + bounds.maxX) * 0.5;
			center[1] = (bounds.minY + bounds.maxY) * 0.5;
			Vector fleeDir = center - dino.position;
			Scalar mag = Geometry::mag(fleeDir);
			if(mag > 0.001)
				fleeDir = fleeDir / mag;

			dino.velocity = fleeDir * info.runSpeed;
			return;
			}

		if(findNearestPrey(dino, preyId, preyDist))
			{
			/* Found prey - start hunting */
			dino.aiState = AI_HUNTING;
			dino.targetDinoId = preyId;

			/* Find the prey dinosaur */
			for(const Dinosaur& prey : dinosaurs)
				{
				if(prey.id == preyId)
					{
					Vector toTarget = prey.position - dino.position;
					Scalar dist = Geometry::mag(toTarget);

					if(dist < info.attackRange)
						{
						/* Close enough to attack! */
						dino.aiState = AI_ATTACKING;
						dino.currentAction = ACTION_ATTACK;
						dino.velocity = Vector(0.0, 0.0, 0.0);
						dino.stateTimer = 0.0f;
						}
					else
						{
						/* Chase the prey */
						dino.currentAction = ACTION_RUN;
						if(dist > 0.001)
							toTarget = toTarget / dist;
						dino.velocity = toTarget * info.runSpeed;
						}
					break;
					}
				}
			}
		else if(dino.aiState == AI_ATTACKING)
			{
			/* Finish attack animation */
			if(dino.stateTimer > 1.0f)
				{
				/* Find and kill the prey */
				for(Dinosaur& prey : dinosaurs)
					{
					if(prey.id == dino.targetDinoId && prey.isAlive)
						{
						Vector diff = prey.position - dino.position;
						if(Geometry::mag(diff) < info.attackRange * 2.0)
							{
							/* Prey caught! */
							prey.isAlive = false;
							prey.aiState = AI_DYING;
							prey.currentAction = ACTION_DIE;
							prey.currentFrame = 0;
							prey.stateTimer = 0.0f;
							prey.velocity = Vector(0.0, 0.0, 0.0);

							const DinosaurSpeciesInfo& preyInfo = getSpeciesInfo(prey.species);
							std::cout << "DinosaurEcosystem: " << info.name
							          << " caught " << preyInfo.name << "!" << std::endl;
							}
						break;
						}
					}

				dino.aiState = AI_IDLE;
				dino.currentAction = ACTION_IDLE;
				dino.stateTimer = 0.0f;
				}
			}
		else
			{
			/* No prey visible - wander/patrol */
			if(dino.aiState != AI_WANDERING || dino.stateTimer > 5.0f)
				{
				dino.aiState = AI_WANDERING;
				dino.currentAction = ACTION_WALK;
				dino.targetPosition = chooseWanderTarget(dino);
				dino.stateTimer = 0.0f;
				}

			/* Move toward target */
			Vector toTarget = dino.targetPosition - dino.position;
			Scalar distToTarget = Geometry::mag(toTarget);

			if(distToTarget > 0.02)
				{
				toTarget = toTarget / distToTarget;
				dino.velocity = toTarget * info.walkSpeed;
				}
			else
				{
				dino.velocity = Vector(0.0, 0.0, 0.0);
				}
			}
		}

	/* Apply avoidance (boundaries, water, lava) */
	Vector avoidance = calculateAvoidanceVector(dino);
	if(Geometry::mag(avoidance) > 0.001)
		{
		dino.velocity = dino.velocity + avoidance * info.walkSpeed * 0.5;
		}
	}

void DinosaurEcosystem::updateDinosaurAnimation(Dinosaur& dino, float deltaTime)
	{
	/* Handle dying animation specially */
	if(dino.aiState == AI_DYING)
		{
		dino.animationTimer += deltaTime;
		if(dino.animationTimer >= dino.frameTime)
			{
			dino.animationTimer -= dino.frameTime;
			dino.currentFrame++;

			const DinosaurSpeciesInfo& info = getSpeciesInfo(dino.species);
			if(dino.currentFrame >= info.framesPerAction[ACTION_DIE])
				{
				/* Death animation complete, start fading */
				dino.currentFrame = info.framesPerAction[ACTION_DIE] - 1;
				dino.alpha -= deltaTime * 0.5f;

				if(dino.alpha <= 0.0f)
					{
					/* Fully faded, start respawn timer */
					dino.aiState = AI_DEAD;
					dino.isVisible = false;
					dino.respawnTimer = respawnDelay;
					}
				}
			}
		return;
		}

	/* Normal animation update */
	dino.animationTimer += deltaTime;
	if(dino.animationTimer >= dino.frameTime)
		{
		dino.animationTimer -= dino.frameTime;
		dino.currentFrame++;

		const DinosaurSpeciesInfo& info = getSpeciesInfo(dino.species);
		int maxFrames = info.framesPerAction[dino.currentAction];
		if(dino.currentFrame >= maxFrames)
			dino.currentFrame = 0;
		}

	/* Update direction based on velocity */
	if(Geometry::mag(dino.velocity) > 0.001)
		{
		dino.direction = calculateDirection(dino.velocity);
		}
	}

void DinosaurEcosystem::updateDinosaurMovement(Dinosaur& dino, float deltaTime)
	{
	if(!dino.isAlive)
		return;

	/* Update position */
	dino.position[0] += dino.velocity[0] * deltaTime;
	dino.position[1] += dino.velocity[1] * deltaTime;

	/* Clamp to bounds */
	dino.position[0] = std::max(bounds.minX, std::min(bounds.maxX, dino.position[0]));
	dino.position[1] = std::max(bounds.minY, std::min(bounds.maxY, dino.position[1]));

	/* Update elevation (terrain following) */
	TerrainInfo terrain = queryTerrain(dino.position);
	Scalar targetZ = terrain.elevation;

	/* Smooth elevation following */
	Scalar elevationSpeed = 0.1;
	dino.position[2] += (targetZ - dino.position[2]) * elevationSpeed;
	}

void DinosaurEcosystem::update(float deltaTime)
	{
	/* Update all dinosaurs */
	for(Dinosaur& dino : dinosaurs)
		{
		updateDinosaurAI(dino, deltaTime);
		updateDinosaurAnimation(dino, deltaTime);
		updateDinosaurMovement(dino, deltaTime);
		}
	}

unsigned int DinosaurEcosystem::getAliveCount(void) const
	{
	unsigned int count = 0;
	for(const Dinosaur& dino : dinosaurs)
		if(dino.isAlive)
			++count;
	return count;
	}

unsigned int DinosaurEcosystem::getHerbivoreCount(void) const
	{
	unsigned int count = 0;
	for(const Dinosaur& dino : dinosaurs)
		if(dino.isAlive && isHerbivore(dino.species))
			++count;
	return count;
	}

unsigned int DinosaurEcosystem::getPredatorCount(void) const
	{
	unsigned int count = 0;
	for(const Dinosaur& dino : dinosaurs)
		if(dino.isAlive && isPredator(dino.species))
			++count;
	return count;
	}
