/* Point light - lighting shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout(location = 0) in vec3 vertex;

struct Light_Struct {
	mat4 shadowVP[6];
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	float LightIntensity;
	float LightRadius;
	int Shadow_Spot;
};
layout (std430, binding = 4) readonly buffer Light_Index_Buffer {
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
layout (location = 12) flat out float LightRadius2;
layout (location = 13) flat out int ShadowSpotFinal;
layout (location = 14) flat out int lightIndex;


void main()
{		
	const int CamIndex = camIndexes[gl_InstanceID].x;
	CamPInverse = inverse(camBuffer[CamIndex].pMatrix);
	CamVInverse = inverse(camBuffer[CamIndex].vMatrix);
	CamEyePosition = camBuffer[CamIndex].EyePosition;
	CamDimensions = camBuffer[CamIndex].CameraDimensions;
	lightIndex = lightIndexes[gl_InstanceID];
	LightColorInt = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
	LightPosition = lightBuffers[lightIndex].LightPosition.xyz;
	LightRadius2 = lightBuffers[lightIndex].LightRadius * lightBuffers[lightIndex].LightRadius;	
	ShadowSpotFinal = lightBuffers[lightIndex].Shadow_Spot;	
	gl_Position = camBuffer[CamIndex].pMatrix * camBuffer[CamIndex].vMatrix * lightBuffers[lightIndex].mMatrix * vec4(vertex, 1.0); 
	gl_Layer = camIndexes[gl_InstanceID].y;
}