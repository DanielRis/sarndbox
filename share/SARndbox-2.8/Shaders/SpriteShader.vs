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

	/* For top-down sandbox view, sprites lie flat on the X-Y plane
	   X axis = world X (right)
	   Y axis = world Y (forward/back)
	   The sprite "up" direction becomes world Y */
	vec3 rightVector = vec3(1.0, 0.0, 0.0);   // World X axis
	vec3 forwardVector = vec3(0.0, 1.0, 0.0); // World Y axis

	/* Calculate world position of this vertex
	   Sprite lies flat on terrain at spritePosition.z */
	vec3 worldPos = spritePosition
	              + rightVector * gl_Vertex.x * spriteSize.x
	              + forwardVector * gl_Vertex.y * spriteSize.y;

	/* Transform to clip space */
	gl_Position = projectionModelviewMatrix * vec4(worldPos, 1.0);

	/* Calculate texture coordinate within the spritesheet
	   Map the quad's texture coord (0-1) to the specific frame */
	fragTexCoord = frameOffset + gl_MultiTexCoord0.xy * frameSize;
	}
