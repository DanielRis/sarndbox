/***********************************************************************
DinosaurRenderer - Class to render animated dinosaur sprites in the AR
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

#ifndef DINOSAURRENDERER_INCLUDED
#define DINOSAURRENDERER_INCLUDED

#include <vector>
#include <map>
#include <string>
#include <GL/gl.h>
#include <GL/Extensions/GLARBShaderObjects.h>
#include <GL/GLObject.h>

#include "Types.h"
#include "Dinosaur.h"

/* Forward declarations */
class WaterTable2;

class DinosaurRenderer:public GLObject
	{
	/* Embedded classes: */
	public:

	/* Structure to hold spritesheet metadata */
	struct SpritesheetInfo
		{
		unsigned int textureWidth;   // Full texture width in pixels
		unsigned int textureHeight;  // Full texture height in pixels
		unsigned int frameWidth;     // Width of single frame
		unsigned int frameHeight;    // Height of single frame
		unsigned int numFrames;      // Number of frames per direction (columns)
		unsigned int numDirections;  // Number of directions (rows) - should be 8
		};

	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:

		/* OpenGL state management: */
		GLuint quadVertexBuffer;  // Vertex buffer for sprite quad

		/* Texture management - one per species per action */
		std::map<std::string, GLuint> spriteTextures;

		/* GLSL shader management: */
		GLhandleARB spriteShader;     // Shader program for sprite rendering
		GLint spriteShaderUniforms[8]; // Uniform locations

		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);

		/* Methods: */
		GLuint getOrLoadTexture(const std::string& path);
		};

	/* Elements: */
	const WaterTable2* waterTable;     // For terrain queries
	Scalar spriteWorldSize;            // Size of sprites in world units
	std::string spritesBasePath;       // Base path to sprites folder

	/* Spritesheet metadata cache */
	std::map<std::string, SpritesheetInfo> spritesheetInfoCache;

	/* Constructors and destructors: */
	public:
	DinosaurRenderer(const WaterTable2* sWaterTable);

	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;

	/* New methods: */

	/* Set the base path where sprite folders are located */
	void setSpritesBasePath(const std::string& path);

	/* Set the world-space size of sprites */
	void setSpriteSize(Scalar size);

	/* Render all dinosaurs */
	void render(
		const std::vector<Dinosaur>& dinosaurs,
		const PTransform& projection,
		const OGTransform& modelview,
		GLContextData& contextData) const;

	/* Get spritesheet info (loads and caches if needed) */
	const SpritesheetInfo& getSpritesheetInfo(DinosaurSpecies species, DinosaurAction action);
	};

#endif
