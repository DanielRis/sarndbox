/***********************************************************************
TerrainQuery - Class to query terrain height and water depth at
arbitrary world coordinates by reading back GPU textures.
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

#ifndef TERRAINQUERY_INCLUDED
#define TERRAINQUERY_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLContextData.h>

#include "Types.h"

/* Forward declarations */
class WaterTable2;

class TerrainQuery
	{
	/* Embedded classes: */
	public:

	/* Terrain type enumeration */
	enum TerrainType
		{
		TERRAIN_NORMAL=0,  // Regular terrain above water
		TERRAIN_WATER,     // Underwater
		TERRAIN_LAVA       // Below lava threshold
		};

	/* Structure returned by terrain queries */
	struct TerrainInfo
		{
		Scalar terrainHeight;      // Actual sand surface elevation
		Scalar waterSurfaceHeight; // Water surface elevation (terrain + water)
		Scalar waterDepth;         // Water depth (0 if dry)
		TerrainType type;          // Terrain classification
		bool isValid;              // False if data not yet available
		};

	/* Elements: */
	private:
	const WaterTable2* waterTable;  // Water table for texture access

	/* Grid dimensions */
	unsigned int gridWidth;         // Width of cached grids
	unsigned int gridHeight;        // Height of cached grids

	/* CPU-side cached grids */
	std::vector<float> terrainGrid; // Bathymetry (terrain heights)
	std::vector<float> waterGrid;   // Water surface elevations

	/* World coordinate bounds */
	Scalar domainMin[3];
	Scalar domainMax[3];

	/* Configuration */
	Scalar lavaThreshold;           // Elevation below which is lava
	Scalar waterDepthThreshold;     // Water depth to classify as underwater

	/* State */
	bool dataValid;                 // True after first successful update
	int updateCounter;              // Throttle updates
	int updateFrequency;            // Update every N frames

	/* Private methods */
	float sampleBilinear(const std::vector<float>& grid, float x, float y) const;

	public:

	/* Constructors and destructors */
	TerrainQuery(const WaterTable2* sWaterTable);
	~TerrainQuery(void);

	/* Methods */

	/* Update cached grids from GPU textures (call each frame) */
	void update(GLContextData& contextData);

	/* Query terrain at world coordinates */
	TerrainInfo query(Scalar worldX, Scalar worldY) const;

	/* Check if data is available */
	bool isDataValid(void) const { return dataValid; }

	/* Configuration */
	void setLavaThreshold(Scalar threshold);
	void setWaterDepthThreshold(Scalar threshold);
	void setUpdateFrequency(int frames);
	};

#endif
