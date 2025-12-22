/***********************************************************************
SpriteShader - Shader to render animated dinosaur sprites on the
sandbox surface.
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

uniform mat4 projectionModelviewMatrix;  // Combined projection * modelview
uniform vec3 spritePosition;             // World position of sprite center
uniform vec2 spriteSize;                 // World-space size of sprite (width, height)
uniform vec2 frameOffset;                // UV offset to current frame in atlas
uniform vec2 frameSize;                  // UV size of one frame in atlas
uniform vec3 upVector;                   // World up direction (usually 0,0,1)

varying vec2 fragTexCoord;               // Output texture coordinate

void main()
	{
	/* gl_Vertex contains the quad corner offset (-0.5 to 0.5)
	   and gl_MultiTexCoord0 contains the base texture coordinate (0 to 1) */

	/* Calculate the right vector from the modelview matrix
	   This ensures sprites always face the camera/projector */
	vec3 rightVector = vec3(
		projectionModelviewMatrix[0][0],
		projectionModelviewMatrix[1][0],
		projectionModelviewMatrix[2][0]
	);
	rightVector = normalize(rightVector);

	/* Calculate the forward vector (perpendicular to both right and up) */
	vec3 forwardVector = cross(rightVector, upVector);
	forwardVector = normalize(forwardVector);

	/* Recalculate right to ensure orthogonality */
	rightVector = cross(upVector, forwardVector);
	rightVector = normalize(rightVector);

	/* Calculate world position of this vertex */
	vec3 worldPos = spritePosition
	              + rightVector * gl_Vertex.x * spriteSize.x
	              + upVector * gl_Vertex.y * spriteSize.y;

	/* Transform to clip space */
	gl_Position = projectionModelviewMatrix * vec4(worldPos, 1.0);

	/* Calculate texture coordinate within the spritesheet
	   Map the quad's texture coord (0-1) to the specific frame */
	fragTexCoord = frameOffset + gl_MultiTexCoord0.xy * frameSize;
	}
