// Author: Emil Hedemalm
// Date: 2012-10-29
#version 120

// Uniforms
// 2D Texture texture
uniform sampler2D diffuseMap;

// Input data from the fragment shader
in vec3 normal;		// Interpollated coordinates that have been transformed to view space
in vec2 UV_Coord;	// Just passed on
in vec3 worldCoord;	// World coordinates of the fragment
in vec3 vecToEye;	// Vector from vertex to eye
in vec3 position;

// Lights
#define MAX_LIGHTS	25
uniform vec3 global_ambient = vec3(1,1,1);
uniform vec4 light_diffuse[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];	// Position or direction, depending on w-parameter 0 = direction, 1 = positional
uniform vec3 light_attenuation[MAX_LIGHTS];	// Constant, linear and quadratic
uniform int light_type[MAX_LIGHTS];	
// Spotlight data
uniform vec3 light_spotDirection[MAX_LIGHTS];
uniform float light_spotCutoff[MAX_LIGHTS];
uniform int light_spotExponent[MAX_LIGHTS];
// Total amount of active lights
uniform int activeLights = 0;	// Lights active, from index 0 to activeLights-1


void main()
{
	// Main sprite texture data.
	vec4 texDiffuse = texture2D(diffuseMap, UV_Coord);	
	
	vec4 color = vec4(texDiffuse);
	color.xyz *= global_ambient;	

	/// Luminosity granted by diffuse lighting.
	vec3 diffuseLuminosity = vec3(0,0,0);
	
		
	/// Lighting
	for (int i = 0; i < activeLights; ++i)
	{			
		float distance = length(light_position[i] - position);
		
		vec3 lightLuminosity = vec3(1,1,1);
		lightLuminosity = light_diffuse[i].xyz;
		
		// Only apply constant light attenuation for the directional lights!?
		float attenuation = 1 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * pow(distance, 2)); 
		lightLuminosity *= attenuation;
			
		
		
		// Add the diffuseLuminosity
		diffuseLuminosity += lightLuminosity;
		
	
		vec3 lightFactor = light_diffuse[i].xyz;
		clamp(lightFactor, 0, 0.1);
	//	color.xyz += lightFactor * (1 / distance);
		if (distance <= 0)
			distance = 1;
		float inverseDistance = 1 / distance;
		
	}
	color.xyz += diffuseLuminosity * texDiffuse.xyz;
	
	gl_FragColor = color;	
	return;
}
 
