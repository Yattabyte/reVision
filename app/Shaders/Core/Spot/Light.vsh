/* Spot light - lighting shader. */
#version 460

layout(location = 0) in vec4 vertex;

struct Light_Struct {
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
};
struct Shadow_Struct {	
	mat4 lightV;
	mat4 lightPV;
	mat4 InverseLightPV;	
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
layout (std430, binding = 4) readonly buffer Shadow_Index_Buffer {
	int shadowIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};
layout (std430, binding = 9) readonly buffer Shadow_Buffer {
	Shadow_Struct shadowBuffers[];
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
layout (location = 20) flat out float HasShadow;

void main()
{		
	CamPInverse = inverse(pMatrix);
	CamVInverse = inverse(vMatrix);
	CamEyePosition = EyePosition;
	CamDimensions = CameraDimensions;	
	const int lightIndex = lightIndexes[gl_InstanceID];
	const int shadowIndex = shadowIndexes[gl_InstanceID];
	LightColorInt = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
	LightPosition = lightBuffers[lightIndex].LightPosition.xyz;
	LightVector = lightBuffers[lightIndex].LightVector.xyz;
	LightRadius2 = lightBuffers[lightIndex].LightRadius * lightBuffers[lightIndex].LightRadius;	
	LightCutoff = lightBuffers[lightIndex].LightCutoff;
	Shadow_Spot = shadowBuffers[shadowIndex].Shadow_Spot;
	ShadowPV = shadowBuffers[shadowIndex].lightPV;
	HasShadow = shadowIndex != -1 ? 1.0f : 0.0f;	
	gl_Position = pMatrix * vMatrix * lightBuffers[lightIndexes[gl_InstanceID]].mMatrix * vertex;
}

