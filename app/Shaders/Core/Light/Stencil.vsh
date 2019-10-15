/* Direct lighting. */
#version 460
#define MAX_PERSPECTIVE_ARRAY 6
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout(location = 0) in vec3 vertex;

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
};
layout (std430, binding = 4) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};


void main()
{		
	const int CamIndex = camIndexes[gl_InstanceID].x;
	gl_Position = camBuffer[CamIndex].pvMatrix * lightBuffers[lightIndexes[gl_InstanceID]].mMatrix * vec4(vertex, 1.0); 
	gl_Layer = camIndexes[gl_InstanceID].y;
}

