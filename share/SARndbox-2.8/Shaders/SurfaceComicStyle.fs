/***********************************************************************
SurfaceComicStyle - Shader fragment to apply cel-shading/posterization
for a child-friendly cartoon visual style.
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

uniform int comicColorLevels; // Number of discrete color levels (typically 4-8)

void applyComicStyle(inout vec4 baseColor)
	{
	/* Posterize each color channel to discrete levels for cel-shading effect */
	float levels = float(comicColorLevels);

	/* Quantize colors to create flat, poster-like appearance */
	baseColor.r = floor(baseColor.r * levels + 0.5) / levels;
	baseColor.g = floor(baseColor.g * levels + 0.5) / levels;
	baseColor.b = floor(baseColor.b * levels + 0.5) / levels;

	/* Boost saturation for more vibrant cartoon colors */
	float gray = dot(baseColor.rgb, vec3(0.299, 0.587, 0.114));
	baseColor.rgb = mix(vec3(gray), baseColor.rgb, 1.4); // 1.4x saturation boost

	/* Clamp to valid range */
	baseColor.rgb = clamp(baseColor.rgb, 0.0, 1.0);
	}
