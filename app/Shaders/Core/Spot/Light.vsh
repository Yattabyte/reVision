/* Spot light - lighting shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout(location = 0) in vec4 vertex;

struct Light_Struct {
	mat4 lightPV;
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
	int Shadow_Spot;	
};
layout (std430, binding = 4) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (location = 0) flat out mat4 pMatrixInverse;
layout (location = 4) flat out mat4 vMatrixInverse;
layout (location = 8) flat out vec2 CameraDimensions;
layout (location = 9) flat out vec3 EyePosition;
layout (location = 10) flat out vec3 LightColorInt;
layout (location = 12) flat out vec3 LightPosition;
layout (location = 13) flat out vec3 LightVector;
layout (location = 14) flat out float LightRadius2;
layout (location = 15) flat out float LightCutoff;
layout (location = 16) flat out int Shadow_Spot;
layout (location = 17) flat out mat4 ShadowPV;


void main()
{		
	const int CamIndex = camIndexes[gl_InstanceID].x;
	pMatrixInverse = camBuffer[CamIndex].pMatrixInverse;
	vMatrixInverse = camBuffer[CamIndex].vMatrixInverse;
	CameraDimensions = camBuffer[CamIndex].CameraDimensions;
	EyePosition = camBuffer[CamIndex].EyePosition;
	const int lightIndex = lightIndexes[gl_InstanceID];
	LightColorInt = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
	LightPosition = lightBuffers[lightIndex].LightPosition.xyz;
	LightVector = lightBuffers[lightIndex].LightVector.xyz;
	LightRadius2 = lightBuffers[lightIndex].LightRadius * lightBuffers[lightIndex].LightRadius;	
	LightCutoff = lightBuffers[lightIndex].LightCutoff;
	Shadow_Spot = lightBuffers[lightIndex].Shadow_Spot;
	ShadowPV = lightBuffers[lightIndex].lightPV;
	gl_Position = camBuffer[CamIndex].pMatrix * camBuffer[CamIndex].vMatrix * lightBuffers[lightIndexes[gl_InstanceID]].mMatrix * vertex;
	gl_Layer = camIndexes[gl_InstanceID].y;
}

