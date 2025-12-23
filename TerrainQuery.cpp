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

#include "TerrainQuery.h"

#include <iostream>
#include <algorithm>
#include <cmath>

#include <GL/gl.h>
#include <GL/Extensions/GLARBTextureRectangle.h>

#include "WaterTable2.h"

/*****************************
Methods of class TerrainQuery:
*****************************/

TerrainQuery::TerrainQuery(const WaterTable2* sWaterTable)
	:waterTable(sWaterTable),
	 gridWidth(0),
	 gridHeight(0),
	 lavaThreshold(-10.0),
	 waterDepthThreshold(0.5),
	 dataValid(false),
	 updateCounter(0),
	 updateFrequency(5)
	{
	if(waterTable != 0)
		{
		/* Get grid dimensions from water table */
		const GLsizei* size = waterTable->getSize();
		gridWidth = size[0];
		gridHeight = size[1];

		/* Allocate CPU-side grids */
		terrainGrid.resize(gridWidth * gridHeight, 0.0f);
		waterGrid.resize(gridWidth * gridHeight, 0.0f);

		/* Get domain bounds */
		const WaterTable2::Box& domain = waterTable->getDomain();
		domainMin[0] = domain.min[0];
		domainMin[1] = domain.min[1];
		domainMin[2] = domain.min[2];
		domainMax[0] = domain.max[0];
		domainMax[1] = domain.max[1];
		domainMax[2] = domain.max[2];

		std::cout << "TerrainQuery: Initialized with grid " << gridWidth << "x" << gridHeight
		          << ", domain X[" << domainMin[0] << " to " << domainMax[0] << "]"
		          << " Y[" << domainMin[1] << " to " << domainMax[1] << "]"
		          << " Z[" << domainMin[2] << " to " << domainMax[2] << "]" << std::endl;
		}
	}

TerrainQuery::~TerrainQuery(void)
	{
	}

float TerrainQuery::sampleBilinear(const std::vector<float>& grid, float x, float y) const
	{
	/* Clamp coordinates to grid bounds */
	x = std::max(0.0f, std::min(x, float(gridWidth - 1)));
	y = std::max(0.0f, std::min(y, float(gridHeight - 1)));

	/* Get integer and fractional parts */
	int x0 = int(x);
	int y0 = int(y);
	int x1 = std::min(x0 + 1, int(gridWidth - 1));
	int y1 = std::min(y0 + 1, int(gridHeight - 1));
	float fx = x - float(x0);
	float fy = y - float(y0);

	/* Sample four corners */
	float v00 = grid[y0 * gridWidth + x0];
	float v10 = grid[y0 * gridWidth + x1];
	float v01 = grid[y1 * gridWidth + x0];
	float v11 = grid[y1 * gridWidth + x1];

	/* Bilinear interpolation */
	float v0 = v00 * (1.0f - fx) + v10 * fx;
	float v1 = v01 * (1.0f - fx) + v11 * fx;
	return v0 * (1.0f - fy) + v1 * fy;
	}

void TerrainQuery::update(GLContextData& contextData)
	{
	if(waterTable == 0)
		return;

	/* Throttle updates */
	if(++updateCounter < updateFrequency)
		return;
	updateCounter = 0;

	/* Read back bathymetry texture (terrain heights) */
	waterTable->bindBathymetryTexture(contextData);
	glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RED, GL_FLOAT, terrainGrid.data());
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

	/* Read back water quantity texture (water surface elevation in RED channel) */
	waterTable->bindQuantityTexture(contextData);
	glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RED, GL_FLOAT, waterGrid.data());
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

	dataValid = true;
	}

TerrainQuery::TerrainInfo TerrainQuery::query(Scalar worldX, Scalar worldY) const
	{
	TerrainInfo info;
	info.isValid = dataValid;
	info.type = TERRAIN_NORMAL;
	info.waterDepth = 0.0;

	if(!dataValid || waterTable == 0)
		{
		/* Return fallback values */
		info.terrainHeight = (domainMin[2] + domainMax[2]) * 0.5;
		info.waterSurfaceHeight = info.terrainHeight;
		return info;
		}

	/* Map world coordinates to normalized [0,1] range */
	float nx = float((worldX - domainMin[0]) / (domainMax[0] - domainMin[0]));
	float ny = float((worldY - domainMin[1]) / (domainMax[1] - domainMin[1]));

	/* Check bounds */
	if(nx < 0.0f || nx > 1.0f || ny < 0.0f || ny > 1.0f)
		{
		/* Outside domain, return edge values */
		nx = std::max(0.0f, std::min(1.0f, nx));
		ny = std::max(0.0f, std::min(1.0f, ny));
		}

	/* Map to grid coordinates */
	float gx = nx * float(gridWidth - 1);
	float gy = ny * float(gridHeight - 1);

	/* Sample with bilinear interpolation */
	info.terrainHeight = Scalar(sampleBilinear(terrainGrid, gx, gy));
	info.waterSurfaceHeight = Scalar(sampleBilinear(waterGrid, gx, gy));

	/* Calculate water depth (water surface is above terrain) */
	info.waterDepth = std::max(Scalar(0.0), info.waterSurfaceHeight - info.terrainHeight);

	/* Determine terrain type */
	if(info.terrainHeight < lavaThreshold)
		{
		info.type = TERRAIN_LAVA;
		}
	else if(info.waterDepth > waterDepthThreshold)
		{
		info.type = TERRAIN_WATER;
		}
	else
		{
		info.type = TERRAIN_NORMAL;
		}

	return info;
	}

void TerrainQuery::setLavaThreshold(Scalar threshold)
	{
	lavaThreshold = threshold;
	}

void TerrainQuery::setWaterDepthThreshold(Scalar threshold)
	{
	waterDepthThreshold = threshold;
	}

void TerrainQuery::setUpdateFrequency(int frames)
	{
	updateFrequency = std::max(1, frames);
	}
