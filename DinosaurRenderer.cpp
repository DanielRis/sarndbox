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

#include "DinosaurRenderer.h"

#include <iostream>
#include <stdexcept>
#include <cmath>

#include <Images/RGBImage.h>
#include <Images/RGBAImage.h>
#include <Images/ReadImageFile.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/Extensions/GLARBFragmentShader.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/Extensions/GLARBShaderObjects.h>
#include <GL/Extensions/GLARBTextureFloat.h>
#include <GL/Extensions/GLARBTextureRectangle.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/Extensions/GLARBVertexShader.h>
#include <GL/GLTransformationWrappers.h>

#include "WaterTable2.h"
#include "ShaderHelper.h"
#include "Config.h"

/********************************************
Methods of class DinosaurRenderer::DataItem:
********************************************/

DinosaurRenderer::DataItem::DataItem(void)
	:quadVertexBuffer(0),
	 spriteShader(0)
	{
	/* Initialize all required extensions: */
	GLARBFragmentShader::initExtension();
	GLARBMultitexture::initExtension();
	GLARBShaderObjects::initExtension();
	GLARBVertexBufferObject::initExtension();
	GLARBVertexShader::initExtension();

	/* Allocate vertex buffer for quad: */
	glGenBuffersARB(1, &quadVertexBuffer);
	}

DinosaurRenderer::DataItem::~DataItem(void)
	{
	/* Release vertex buffer: */
	glDeleteBuffersARB(1, &quadVertexBuffer);

	/* Release all textures: */
	for(auto& pair : spriteTextures)
		glDeleteTextures(1, &pair.second);

	/* Release shader: */
	if(spriteShader != 0)
		glDeleteObjectARB(spriteShader);
	}

GLuint DinosaurRenderer::DataItem::getOrLoadTexture(const std::string& path)
	{
	/* Check if texture already loaded */
	auto it = spriteTextures.find(path);
	if(it != spriteTextures.end())
		return it->second;

	/* Load image using Vrui's Images library */
	Images::RGBImage rgbImage;
	try
		{
		rgbImage = Images::readImageFile(path.c_str());
		}
	catch(const std::exception& e)
		{
		std::cerr << "DinosaurRenderer: Failed to load sprite: " << path << " (" << e.what() << ")" << std::endl;
		return 0;
		}

	/* Get image dimensions */
	unsigned int width = rgbImage.getSize(0);
	unsigned int height = rgbImage.getSize(1);

	/* Create OpenGL texture */
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	/* Set texture parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Upload texture data as RGB (sprites will need color-keying for transparency) */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
	             GL_RGB, GL_UNSIGNED_BYTE, rgbImage.getPixels());

	/* Cache and return */
	spriteTextures[path] = textureId;

	std::cout << "DinosaurRenderer: Loaded sprite " << path
	          << " (" << width << "x" << height << ")" << std::endl;

	return textureId;
	}

/*********************************
Methods of class DinosaurRenderer:
*********************************/

DinosaurRenderer::DinosaurRenderer(const WaterTable2* sWaterTable)
	:waterTable(sWaterTable),
	 spriteWorldSize(0.05),      // Default sprite size in world units
	 spritesBasePath(std::string(CONFIG_SPRITEDIR) + "/")
	{
	}

void DinosaurRenderer::initContext(GLContextData& contextData) const
	{
	/* Create a data item and add it to the context: */
	DataItem* dataItem = new DataItem;
	contextData.addDataItem(this, dataItem);

	/* Create quad vertices for sprite rendering
	   Quad is centered at origin, extends from -0.5 to +0.5 */
	struct QuadVertex
		{
		GLfloat position[3];
		GLfloat texCoord[2];
		};

	QuadVertex quadVertices[4] =
		{
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}}, // Bottom-left
		{{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}}, // Bottom-right
		{{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}}, // Top-right
		{{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f}}  // Top-left
		};

	/* Upload quad vertices */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, dataItem->quadVertexBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	/* Create the sprite rendering shader */
	dataItem->spriteShader = linkVertexAndFragmentShader("SpriteShader");
	GLint* ulPtr = dataItem->spriteShaderUniforms;
	*(ulPtr++) = glGetUniformLocationARB(dataItem->spriteShader, "spriteSampler");
	*(ulPtr++) = glGetUniformLocationARB(dataItem->spriteShader, "projectionModelviewMatrix");
	*(ulPtr++) = glGetUniformLocationARB(dataItem->spriteShader, "spritePosition");
	*(ulPtr++) = glGetUniformLocationARB(dataItem->spriteShader, "spriteSize");
	*(ulPtr++) = glGetUniformLocationARB(dataItem->spriteShader, "frameOffset");
	*(ulPtr++) = glGetUniformLocationARB(dataItem->spriteShader, "frameSize");
	*(ulPtr++) = glGetUniformLocationARB(dataItem->spriteShader, "spriteAlpha");
	*(ulPtr++) = glGetUniformLocationARB(dataItem->spriteShader, "upVector");
	}

void DinosaurRenderer::setSpritesBasePath(const std::string& path)
	{
	spritesBasePath = path;
	}

void DinosaurRenderer::setSpriteSize(Scalar size)
	{
	spriteWorldSize = size;
	}

void DinosaurRenderer::render(
	const std::vector<Dinosaur>& dinosaurs,
	const PTransform& projection,
	const OGTransform& modelview,
	GLContextData& contextData) const
	{
	if(dinosaurs.empty())
		return;

	/* Get the data item: */
	DataItem* dataItem = contextData.retrieveDataItem<DataItem>(this);

	/* Calculate the combined projection-modelview matrix */
	PTransform projectionModelview = projection;
	projectionModelview *= modelview;

	/* Enable blending for transparency */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Disable depth writing but keep depth testing */
	glDepthMask(GL_FALSE);

	/* Bind the sprite shader */
	glUseProgramObjectARB(dataItem->spriteShader);
	const GLint* ulPtr = dataItem->spriteShaderUniforms;

	/* Set texture sampler */
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glUniform1iARB(*(ulPtr++), 0);

	/* Upload projection-modelview matrix */
	GLint pmvUniform = *(ulPtr++);
	glUniformARB(pmvUniform, projectionModelview);

	GLint posUniform = *(ulPtr++);
	GLint sizeUniform = *(ulPtr++);
	GLint frameOffsetUniform = *(ulPtr++);
	GLint frameSizeUniform = *(ulPtr++);
	GLint alphaUniform = *(ulPtr++);
	GLint upVectorUniform = *(ulPtr++);

	/* Set up vector (world Z axis) */
	glUniform3fARB(upVectorUniform, 0.0f, 0.0f, 1.0f);

	/* Set sprite size */
	glUniform2fARB(sizeUniform, GLfloat(spriteWorldSize), GLfloat(spriteWorldSize));

	/* Bind vertex buffer */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, dataItem->quadVertexBuffer);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), (void*)0);
	glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	/* Render each dinosaur */
	for(const Dinosaur& dino : dinosaurs)
		{
		if(!dino.isVisible)
			continue;

		/* Get texture for this dinosaur's current action */
		std::string texturePath = spritesBasePath + getSpritesheetPath(dino.species, dino.currentAction);
		GLuint textureId = dataItem->getOrLoadTexture(texturePath);
		if(textureId == 0)
			continue;

		glBindTexture(GL_TEXTURE_2D, textureId);

		/* Set dinosaur position */
		glUniform3fARB(posUniform,
			GLfloat(dino.position[0]),
			GLfloat(dino.position[1]),
			GLfloat(dino.position[2]));

		/* Set alpha for fade effects */
		glUniform1fARB(alphaUniform, dino.alpha);

		/* Calculate frame offset in texture
		   Spritesheets are organized as:
		   - 8 rows (directions): N, NE, E, SE, S, SW, W, NW (top to bottom)
		   - 15 columns (frames): animation frames (left to right)
		*/
		const int numFrames = 15;
		const int numDirections = 8;

		/* Map our direction enum to spritesheet row order */
		int dirRow = static_cast<int>(dino.direction);

		/* Calculate UV offset and size for the current frame */
		float frameU = float(dino.currentFrame) / float(numFrames);
		float frameV = float(dirRow) / float(numDirections);
		float frameSizeU = 1.0f / float(numFrames);
		float frameSizeV = 1.0f / float(numDirections);

		glUniform2fARB(frameOffsetUniform, frameU, frameV);
		glUniform2fARB(frameSizeUniform, frameSizeU, frameSizeV);

		/* Draw the sprite quad */
		glDrawArrays(GL_QUADS, 0, 4);
		}

	/* Clean up */
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgramObjectARB(0);

	/* Restore state */
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	}

const DinosaurRenderer::SpritesheetInfo& DinosaurRenderer::getSpritesheetInfo(
	DinosaurSpecies species, DinosaurAction action)
	{
	std::string path = getSpritesheetPath(species, action);

	/* Check cache */
	auto it = spritesheetInfoCache.find(path);
	if(it != spritesheetInfoCache.end())
		return it->second;

	/* Create default info (will be updated when texture is loaded) */
	SpritesheetInfo info;
	info.textureWidth = 960;   // Estimated based on sprite pack
	info.textureHeight = 512;
	info.frameWidth = 64;
	info.frameHeight = 64;
	info.numFrames = 15;
	info.numDirections = 8;

	spritesheetInfoCache[path] = info;
	return spritesheetInfoCache[path];
	}
