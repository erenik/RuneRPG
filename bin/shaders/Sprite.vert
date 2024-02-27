// Author: Emil Hedemalm
// Date: 2014-04-06
// Shader for 2D-sprites!

#version 120

// Uniforms
// Model matrix 
uniform mat4 modelMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);	 
// View matrix 
uniform mat4 viewMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
// Projection matrix 
uniform mat4 projectionMatrix = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

// Input data for the shader
// in_Position was bound to attribute index 0, UV to index 1 and Normals to index 2.
in vec3 in_Position;
in vec2 in_UV;

// Output data for the fragment shader
varying vec2 UV_Coord;		// Just passed on
varying vec3 normal;		// Pass on and interpollate normals
varying vec3 position; 		// World position of zis vertex.

void main(){
	// Calculate matrices
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
	// World-space.
	position = (modelMatrix * vec4(in_Position,1)).xyz;
	// Multiply mvp matrix onto the vertex coordinates for screen-space
	gl_Position = mvp * vec4(in_Position, 1);
	
	// Pass on UV-coordinates
	UV_Coord = in_UV;
}
