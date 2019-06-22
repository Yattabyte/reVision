/* Spot light - lighting shader. */
#version 460

layout(location = 0) in vec4 vertex;

struct Light_Struct {
	mat4 lightV;
	mat4 lightPV;
	mat4 InverseLightPV;	
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
	int Shadow_Spot;	
};
layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	vec3 EyePosition;
	vec2 CameraDimensions;
};
layout (std430, binding = 3) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (location = 0) flat out mat4 CamPInverse;
layout (location = 4) flat out mat4 CamVInverse;
layout (location = 8) flat out vec3 CamEyePosition;
layout (location = 9) flat out vec2 CamDimensions;
layout (location = 10) flat out vec3 LightColorInt;
layout (location = 11) flat out vec3 LightPosition;
layout (location = 12) flat out vec3 LightVector;
layout (location = 13) flat out float LightRadius2;
layout (location = 14) flat out float LightCutoff;
layout (location = 15) flat out int Shadow_Spot;
layout (location = 16) flat out mat4 ShadowPV;

void main()
{		
	CamPInverse = inverse(pMatrix);
	CamVInverse = inverse(vMatrix);
	CamEyePosition = EyePosition;
	CamDimensions = CameraDimensions;	
	const int lightIndex = lightIndexes[gl_InstanceID];
	LightColorInt = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
	LightPosition = lightBuffers[lightIndex].LightPosition.xyz;
	LightVector = lightBuffers[lightIndex].LightVector.xyz;
	LightRadius2 = lightBuffers[lightIndex].LightRadius * lightBuffers[lightIndex].LightRadius;	
	LightCutoff = lightBuffers[lightIndex].LightCutoff;
	Shadow_Spot = lightBuffers[lightIndex].Shadow_Spot;
	ShadowPV = lightBuffers[lightIndex].lightPV;
	gl_Position = pMatrix * vMatrix * lightBuffers[lightIndexes[gl_InstanceID]].mMatrix * vertex;
}

