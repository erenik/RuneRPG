// Author: Emil Hedemalm
// Date: 2012-11-26
// Name: Simple UI Shader to paint the UI elements background textures
#version 410

// Uniforms
// Projection matrix provided by the client.
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
uniform mat4 modelMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
in vec3 in_Position;
in vec2 in_UV;

// Output data for the fragment shader
out vec2 UV_Coord;		// Just passed on
out vec3 position;

void main(void) {
	// Calculate matrices
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

	// Multiply mvp matrix onto the vertex coordinates.
	gl_Position = mvp * vec4(in_Position, 1);
	position = in_Position;
	// Just pass on the UV-coordinates ^^
	UV_Coord = in_UV;
}
