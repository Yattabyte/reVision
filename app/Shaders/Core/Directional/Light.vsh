/* Directional light - lighting shader. */
#version 460
#define NUM_CASCADES 4
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout (location = 0) in vec3 vertex;

struct Light_Struct {
	mat4 LightVP[NUM_CASCADES];
	float CascadeEndClipSpace[NUM_CASCADES];
	vec4 LightColor;
	vec4 LightDirection;
	float LightIntensity;
	int Shadow_Spot;
};
layout (std430, binding = 4) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out mat4 pMatrixInverse;
layout (location = 5) flat out mat4 vMatrixInverse;
layout (location = 9) flat out vec3 EyePosition;
layout (location = 10) flat out vec3 LightColorInt;
layout (location = 11) flat out vec3 LightDirection;
layout (location = 12) flat out int Shadow_Spot;
layout (location = 13) flat out vec4 CascadeEndClipSpace;
layout (location = 14) flat out mat4 LightVP[NUM_CASCADES];


void main()
{	
	const int CamIndex = camIndexes[gl_InstanceID].x;
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	pMatrixInverse = camBuffer[CamIndex].pMatrixInverse;
	vMatrixInverse = camBuffer[CamIndex].vMatrixInverse;
	EyePosition = camBuffer[CamIndex].EyePosition;
	const int lightIndex = lightIndexes[gl_InstanceID];
	LightColorInt = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
	LightDirection = lightBuffers[lightIndex].LightDirection.xyz;
	Shadow_Spot = lightBuffers[lightIndex].Shadow_Spot;
	for (uint x = 0; x < NUM_CASCADES; ++x) {
		CascadeEndClipSpace[x] = lightBuffers[lightIndex].CascadeEndClipSpace[x];
		LightVP[x] = lightBuffers[lightIndex].LightVP[x];
	}
	gl_Position = vec4(vertex, 1);
	gl_Layer = camIndexes[gl_InstanceID].y;
}