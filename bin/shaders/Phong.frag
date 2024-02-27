// Author: Emil Hedemalm
// Date: 2012-10-29
#version 410

// For bit-wise operations!
// #extension GL_EXT_gpu_shader4 : enable

// Uniforms
// Position of eye in the world.
uniform vec4 eyePosition	= vec4(0.0, 5.0, 0.0, 0.0);
// Color applied to all stuff in the final.
uniform vec4 primaryColorVec4 = vec4(1.0, 1.0, 1.0, 1.0);

// Lights
#define MAX_LIGHTS	9
uniform vec4 light_ambient = vec4(1,1,1,1);
uniform vec4 light_diffuse[MAX_LIGHTS];
uniform vec4 light_specular[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];
uniform vec3 light_attenuation[MAX_LIGHTS];	// Constant, linear and quadratic

// Position or direction, depending on w-parameter 0 = direction, 1 = positional
uniform int light_type[MAX_LIGHTS];

// Spotlight data
uniform vec3 light_spotDirection[MAX_LIGHTS];
uniform float light_spotCutoff[MAX_LIGHTS];
uniform int light_spotExponent[MAX_LIGHTS];
// Total amount of active lights
uniform int activeLights = 0;	// Lights active, from index 0 to activeLights-1

// Material statistics
uniform vec4	materialAmbient		= vec4(0.2, 0.2, 0.2, 1.0);
uniform vec4	materialDiffuse		= vec4(0.8, 0.8, 0.8, 1.0);
uniform vec4	materialSpecular	= vec4(1.1, 1.1, 1.1, 1.0);
uniform int		materialShininess	= 8	;

// 2D Texture texture
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2D fogOfWarMap;
uniform sampler2D cloudsMap; // for cloud-shadows

/// Individual bools for if the above samplers are active or not!
uniform bool useDiffuseMap;
uniform bool useSpecularMap;
uniform bool useNormalMap;

uniform bool inFogOfWar = true;
uniform vec3 fogOfWarSize = vec3(5,1,5);

uniform vec4 cloudsOffsetAlphaScale; // xy = offset on sampling, z = alpha, if we want to fade/in out it.

uniform float fogBegin = 500.0f;
uniform float fogEnd = 2500.0f;
uniform vec3 fogColor = vec3(0,0,0);

// Input data from the fragment shader
in vec3 v_normal;		// Interpolated coordinates that have been transformed to view space
in vec2 UV_Coord;	// Just passed on
in vec3 position;	// World coordinates of the fragment
in vec3 vecToEye;	// Vector from vertex to eye
in vec4 v_Tangent;	// Tangent!

out vec4 fragColor;

void main()
{
	// For testing shit
	vec3 constantColor = vec3(0,0,0);

	// Set depth first..
	float depth = gl_FragCoord.z;
	/// Set depth of the pixel as it was!
	gl_FragDepth = depth; //depth.x;
	// Base texel for color, usually gathered from the DiffuseMap.
	vec4 texel;
	// Texture image data. This will be the base for the colors.
	if (useDiffuseMap){
		/// Call the blended texture fragment Texel ^^ Could be derived from multiple textures! ^.^
		texel = texture(diffuseMap, UV_Coord);
	}
	else
		texel = vec4(0.3,0.3,0.3,1.0);

	fragColor = texel;

	vec3 normal = v_normal;

	// Calculate vector to eye from vertex.
	vec3 vecToEye = eyePosition.xyz - position.xyz;
	/// Set color to 0.
	fragColor = vec4(0,0,0,0);

	/// Set alpha now. .. what
	fragColor.w = texel.w;

	// Luminosity for each color
	vec3 diffuseLuminosity = vec3(0,0,0);
	vec3 specularLuminosity = vec3(0,0,0);

	bool doLights = true;
	if (doLights)
	for (int i = 0; i < activeLights; ++i){
		vec3 lightLuminosity = vec3(0,0,0);
		vec3 lightSpecular = vec3(0,0,0);
		// Directional lighting ^^
		if (light_type[i] == 2){

			vec3 lightDirection = normalize(light_position[i].xyz);
			vec3 normalizedNormal = normalize(vec3(normal.xyz));

			vec2 samplingOffsetFromHeight = -(lightDirection * position.y).xz;

			vec4 cloudsSample = texture(cloudsMap, (position.xz * 0.5 + samplingOffsetFromHeight + cloudsOffsetAlphaScale.xy) / cloudsOffsetAlphaScale.w );
			float inCloudsFactor = 1 - cloudsSample.w * 0.6 * cloudsOffsetAlphaScale.z;

			float difIntensity = max(dot(lightDirection, normalizedNormal), 0.0);
			/// Opt out if the diffuse-intensity is 0. (L-Dot-N == 0)
			/// The specular has no chance to be above 0 if this is the case!
			if (difIntensity <= 0)
				continue;
			lightLuminosity += difIntensity * light_diffuse[i].xyz;

			// Only appl7y constant light attenuation for the directional lights!?
			float distance = length(light_position[i].xyz - position.xyz);
			float attenuation = 1 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * pow(distance, 2)); 
			lightLuminosity *= attenuation;

			// Specular - intensity same, but consider the half vector and eye position.
			vec3 vectorToEye = normalize(vecToEye);
			vec3 halfVector = normalize((lightDirection + vectorToEye));
			float initialBrightness = max(dot(halfVector, normalizedNormal), 0.0);
			int smoothness = 12;
			float totalBrightness = initialBrightness;
			for (int i = 0; i < materialShininess; ++i){
				totalBrightness *= initialBrightness;
			}
			lightSpecular += totalBrightness * light_specular[i].xyz;
			lightSpecular *= attenuation;

			lightLuminosity *= inCloudsFactor;
			lightSpecular *= inCloudsFactor;
		}
		// Positional lighting o-o;
		else if (light_type[i] == 1){

			// Diffuse
			vec3 lightDirection = normalize(position.xyz - light_position[i].xyz);
			vec3 normalizedNormal = normalize(vec3(normal.xyz));

			float difIntensity = max(-dot(lightDirection, normalizedNormal), 0.0);
			/// Opt out if the diffuse-intensity is 0. (L-Dot-N == 0)
			/// The specular has no chance to be above 0 if this is the case!
			if (difIntensity <= 0)
				continue;

			lightLuminosity += difIntensity * light_diffuse[i].xyz;

			// Only apply constant light attenuation for the directional lights!?
			float distance = length(light_position[i].xyz - position.xyz);
			float attenuation = 1 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * pow(distance, 2)); 
			lightLuminosity *= attenuation;

			// Specular - intensity same, but consider the half vector and eye position.
			// First calc vector to eye from the fragment
			vec3 vectorToEye = normalize(vecToEye);

			// Calculate the mid-way vector between the angle of the light and the vector to the eye.
			// Since the light direction is in the opposite direction at first, negate it.
			vec3 halfVector = normalize((-lightDirection + vectorToEye) / 2);
			float initialBrightness = max(dot(halfVector, normalizedNormal), 0.0);

			int smoothness = 12;
			float totalBrightness = initialBrightness;
			for (int i = 0; i < materialShininess; ++i){
				totalBrightness *= initialBrightness;
			}
			lightSpecular += totalBrightness * light_specular[i].xyz;
			lightSpecular *= attenuation;

			lightSpecular *= 0.3;

		}
		// Spotlights!!!
		else if (light_type[i] == 3){
			/// First compare lightDirection with the spot's direction.
			vec3 lightDirection = normalize(light_position[i].xyz - position.xyz);
			// Check if we should do more calculations at all or not...
			float spotDotCutoff = dot(normalize(-light_spotDirection[i]), lightDirection);
			if (spotDotCutoff > light_spotCutoff[i]){
				// Now just do normal calculations as per positional light-sources ^^
				vec3 normalizedNormal = normalize(vec3(normal.xyz));
				float difIntensity = max(dot(lightDirection, normalizedNormal), 0.0);
				lightLuminosity += difIntensity * light_diffuse[i].xyz;
				// Only apply constant light attenuation for the directional lights!?
				float distance = length(light_position[i].xyz - position.xyz);
				float attenuation = 1 / (light_attenuation[i].x + light_attenuation[i].y * distance + light_attenuation[i].z * pow(distance, 2)); 
				lightLuminosity *= attenuation;

				// Specular - intensity same, but consider the half vector and eye position.
				vec3 vectorToEye = normalize(vecToEye);
				vec3 halfVector = normalize((lightDirection + vectorToEye) / 2);
				float initialBrightness = max(dot(halfVector, normalizedNormal), 0.0);
				int smoothness = 12;
				float totalBrightness = initialBrightness;
				for (int i = 0; i < materialShininess; ++i){
					totalBrightness *= initialBrightness;
				}
				lightSpecular += totalBrightness * light_specular[i].xyz;
				lightSpecular *= attenuation;

				// Calculate the spotlight exponent
				float spotIntensity = 1.0f;
				for (int j = 0; j < light_spotExponent[i]; ++j){
					spotIntensity *= spotDotCutoff;
				}
				/// Apply the spot intensity to both the added specular and diffuse luminosity
				lightSpecular *= spotIntensity;
				lightLuminosity *= spotIntensity;
			}
		}
		// Add the diffuseLuminosity
		diffuseLuminosity += lightLuminosity;
		specularLuminosity += lightSpecular;
	}

	// Final illumination done, multiply with diffuse ^^
	fragColor.xyz += texel.xyz * diffuseLuminosity * materialDiffuse.xyz;

	vec3 specularTotal = vec3(0,0,0);

	/// Check if using specular map.
	if (useSpecularMap)
	{
		vec4 specTex = texture(specularMap, UV_Coord);
		specularTotal.xyz += specTex.xyz * specularLuminosity * materialSpecular.xyz;
	}
	// Default, use diffuse-map as specular.
	else
		specularTotal.xyz += texel.xyz * specularLuminosity * materialSpecular.xyz;

	fragColor.xyz += specularTotal;

	/// Add global ambient ^^
	fragColor.xyz += texel.xyz * light_ambient.xyz * materialAmbient.xyz;

	// Additional multiplier.
	fragColor.xyz *= primaryColorVec4.xyz;

	fragColor.w = texel.w;

	if (constantColor.x > 0 || constantColor.y > 0 || constantColor.z  > 0)
		fragColor.xyz = constantColor;

	// Add distance fog.
	float distance = length(vecToEye);
	float ratioFog = 0;
	if (distance > fogEnd) {
		ratioFog = 1;
	}
	else if (distance > fogBegin){
		ratioFog = (distance - fogBegin) / (fogEnd - fogBegin);
	}
	// Reduce fog intensity the higher we are - be able to see clearly downward.
	if (eyePosition.y > 5) {
		ratioFog *= 10 / (eyePosition.y + 5);
	}

	fragColor.xyz = fragColor.xyz * ( 1 - ratioFog) +  fogColor * ratioFog;

	vec2 worldPosToTexturePos = vec2(position.x / fogOfWarSize.x, (fogOfWarSize.z - position.z) / fogOfWarSize.z);
	vec4 inFowSample = texture(fogOfWarMap, worldPosToTexturePos);
	if (inFowSample.x < 0.1) {
		fragColor.xyz *= 0.3;
	}

//	fragColor.x = 0.5;

}

