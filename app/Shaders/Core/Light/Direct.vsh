/* Direct lighting. */
#version 460
#define MAX_PERSPECTIVE_ARRAY 6
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

struct Light_Struct {
	mat4 LightVP[MAX_PERSPECTIVE_ARRAY];
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightDirection;
	float CascadeEndClipSpace[MAX_PERSPECTIVE_ARRAY];
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
	int Shadow_Spot;
	int Light_Type;
};
layout (std430, binding = 4) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (location = 0) in vec3 vertex;

layout (location = 0) flat out mat4 pMatrixInverse;
layout (location = 4) flat out mat4 vMatrixInverse;
layout (location = 8) flat out vec3 EyePosition;
layout (location = 9) flat out vec2 CameraDimensions;
layout (location = 10) flat out int lightIndex;
layout (location = 11) flat out int lightType;


void main()
{	
	const int CamIndex = camIndexes[gl_InstanceID].x;
	pMatrixInverse = camBuffer[CamIndex].pMatrixInverse;
	vMatrixInverse = camBuffer[CamIndex].vMatrixInverse;
	EyePosition = camBuffer[CamIndex].EyePosition;
	CameraDimensions = camBuffer[CamIndex].CameraDimensions;
	lightIndex = lightIndexes[gl_InstanceID];	
	lightType = lightBuffers[lightIndex].Light_Type;
	gl_Position = camBuffer[CamIndex].pvMatrix * lightBuffers[lightIndex].mMatrix * vec4(vertex, 1.0); 
	gl_Layer = camIndexes[gl_InstanceID].y;
}