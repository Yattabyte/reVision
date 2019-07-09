/* Directional light - (indirect) light bounce shader. */
#version 460
#define NUM_CASCADES 4
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

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
layout (location = 0) uniform int LightCount = 0;
layout (location = 0) in vec3 vertex;

layout (location = 0) flat out mat4 vMatrix;
layout (location = 4) flat out mat4 CamPVMatrix;
layout (location = 8) flat out mat4 LightVP[NUM_CASCADES];
layout (location = 24) flat out vec4 CascadeEndClipSpace;
layout (location = 25) flat out int Shadow_Spot;
layout (location = 26) flat out vec3 ColorModifier;


void main()
{
	// Draw order arranged like: (layer 0)[light 0, light 1, light 2], (layer 1)[light 0, light 1, light 2], etc	
	const int CamIndex = camIndexes[gl_InstanceID % LightCount].x;
    gl_Position = vec4(vertex, 1);
    gl_Layer = gl_InstanceID / LightCount;
	const int lightIndex = lightIndexes[gl_InstanceID % LightCount];
	vMatrix = camBuffer[CamIndex].vMatrix;
	CamPVMatrix = camBuffer[CamIndex].pMatrix * camBuffer[CamIndex].vMatrix;
	for (uint x = 0; x < NUM_CASCADES; ++x) {
		LightVP[x] = lightBuffers[lightIndex].LightVP[x];
		CascadeEndClipSpace[x] = lightBuffers[lightIndex].CascadeEndClipSpace[x];
	}
	Shadow_Spot = lightBuffers[lightIndex].Shadow_Spot;
	ColorModifier = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
}
