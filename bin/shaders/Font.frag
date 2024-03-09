// Author: Emil Hedemalm
#version 410

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Yush.
uniform vec4 primaryColorVec4 = vec4(1,1,1,1);
uniform vec4 secondaryColorVec4 = vec4(.5,.5,.5,1);
/// Highlight that is added linearly upon the final product.
uniform vec4 highlightColorVec4 = vec4(0,0,0,0);

// Input data from the fragment shader
in vec2 UV_Coord;	// Just passed on
in vec3 worldCoord;	// World coordinates of the fragment
in vec3 vecToEye;	// Vector from vertex to eye
in vec3 position;

in vec3 debugColor;

/** Default 0 - pass through.
	1 - Simple White font - apply primaryColorVec4 multiplicatively
	2 - Replacer. Replaces a set amount of colors in the font for other designated colors (primarily primaryColorVec4 and se)
*/
uniform int colorEquation = 1;

out vec4 fragColor;

void main(void)
{
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture(baseImage, UV_Coord);
	fragColor = baseFrag;

	if (debugColor.x > 0)
		fragColor = vec4(debugColor.xyz, 1);

	// New rendering with TTF. The baseImage texture will contain the alpha channel basically!
	fragColor.xyzw = primaryColorVec4.xyzw;
	fragColor += highlightColorVec4;
	fragColor.w *= baseFrag.x;
	//fragColor.xyzw += 0.1f;

/*
	// Is it outside the text? Add shader if it's nearby?
	if (true || textShadow == 1) {
		if (baseFrag.r == 0) {
			float shadowOffset = 1.0 / 512;
			float shadowValue = 0.0;
			vec4 shadowReferenceFrag = texture(baseImage, UV_Coord + vec2(-shadowOffset, shadowOffset));
			if (shadowReferenceFrag.x != 0) {
				fragColor.xyz -= vec3(1,1,1);
				fragColor.w += 0.5;
			}
			return;
		}
	}
*/

}


