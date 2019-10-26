/* Bounce lighting. */
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

layout (location = 0) uniform int LightCount = 0;

layout (location = 0) in vec3 vertex;

layout (location = 0) flat out mat4 vMatrix;
layout (location = 4) flat out mat4 CamPVMatrix;
layout (location = 8) flat out int Shadow_Spot;
layout (location = 9) flat out vec3 ColorModifier;
layout (location = 10) flat out int lightIndex;


void main()
{
	// Draw order arranged like: (layer 0)[light 0, light 1, light 2], (layer 1)[light 0, light 1, light 2], etc	
	const int CamIndex = camIndexes[gl_InstanceID % LightCount].x;
    gl_Position = vec4(vertex, 1);
    gl_Layer = gl_InstanceID / LightCount;
	lightIndex = lightIndexes[gl_InstanceID % LightCount];
	vMatrix = camBuffer[CamIndex].vMatrix;
	CamPVMatrix = camBuffer[CamIndex].pvMatrix;
	Shadow_Spot = lightBuffers[lightIndex].Shadow_Spot;
	ColorModifier = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
}