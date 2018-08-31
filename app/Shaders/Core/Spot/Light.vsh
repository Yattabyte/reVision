#version 460
#package "camera"

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

layout (std430, binding = 3) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 4) readonly buffer Shadow_Index_Buffer {
	int shadowIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (location = 0) flat out int LightIndex;
layout (location = 1) flat out int ShadowIndex;

void main()
{		
	LightIndex = lightIndexes[gl_InstanceID];
	ShadowIndex = shadowIndexes[gl_InstanceID];
	gl_Position = cameraBuffer.pMatrix * cameraBuffer.vMatrix * lightBuffers[lightIndexes[gl_InstanceID]].mMatrix * vertex;
}

