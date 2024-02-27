// Author: Emil Hedemalm
// Date: 2022-04-24
// Name: UI shader supporting wrapped textures.
#version 410

// Uniforms
// 2D Texture texture
uniform sampler2D baseImage;

// Yush.
uniform vec4	primaryColorVec4 = vec4(1,1,1,1);
/// Highlight that is added linearly upon the final product.
uniform vec4	highlightColorVec4 = vec4(0,0,0,0);
uniform int		highlightMode = 0;

uniform int wrapTexture = 0;

// Input data from the fragment shader
in vec2 UV_Coord;	// Just passed on
in vec3 worldCoord;	// World coordinates of the fragment
in vec3 vecToEye;	// Vector from vertex to eye
in vec3 position;

out vec4 fragColor;

void main(void)
{
	// Texture image data. This will be the base for the colors.
	vec4 baseFrag = texture(baseImage, UV_Coord);

	if (wrapTexture == 1) {
		baseFrag = texture(baseImage, position.xy * 0.01);
	}

	vec4 color = clamp(baseFrag, vec4(0,0,0,0), vec4(1,1,1,1));

	fragColor = color;
	fragColor *= primaryColorVec4;
	if (highlightMode == 0){
	}
	else if (highlightMode == 1){ // 50% add, 50% multiply
		float hFactor = highlightColorVec4.x * 0.5;
		float highlightFactor = 1.0 + hFactor;
		fragColor.xyz *= highlightFactor;
		fragColor.xyz += vec3(1,1,1) * hFactor;
	}
	else if (highlightMode == 2){ // multiply only
		float hFactor = highlightColorVec4.x;
		float highlightFactor = 1.0 + hFactor;
		fragColor.xyz *= highlightFactor;
	}
	else if (highlightMode == 3){ // Additive only
		float hFactor = highlightColorVec4.x;
		fragColor.xyz += vec3(1,1,1) * hFactor;
	}
}


