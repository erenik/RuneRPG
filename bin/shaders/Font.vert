// Author: Emil Hedemalm
#version 410

// Uniforms
// Projection matrix provided by the client.
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Input data for the shader
// in_Position was bound to in index 0, UV to index 1 and Normals to index 2.
in vec3 in_Position;
in vec2 in_UV;

// Output data for the fragment shader
out vec2 UV_Coord;		// Just passed on
out vec3 position;

/// Which character in the font we are to render.
uniform int character = 65;
uniform float scale = 1;
uniform float textureWidthHeightRatio = 1.0;
uniform vec2 pivot = vec2(0,0);

out vec3 debugColor;

void main(void) 
{
	debugColor = vec3(0,0,0);
	// Calculate matrices
	float scaleToUse = scale * 1.0;
	mat4 scaleMatrix = mat4(scaleToUse * textureWidthHeightRatio, 0.0, 0.0, 0.0, 0.0, scaleToUse, 0.0, 0.0, 0.0, 0.0, scaleToUse, 0.0, 0.0, 0.0, 0.0, 1.0);
	mat4 translationMatrix = 
		mat4(1.0, 0.0, 0.0, 0.0,
			 0.0, 1.0, 0.0, 0.0,
			 0.0, 0.0, 1.0, 0.0,
			 pivot.x, pivot.y, 0, 1.0);

	mat4 modelMatrix = translationMatrix * scaleMatrix;
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
	gl_Position = mvp * vec4(in_Position, 1);

	position = in_Position;
	UV_Coord = in_UV;
}
