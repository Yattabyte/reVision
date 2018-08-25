#version 460
#package "camera"

layout(location = 0) in vec3 vertex;

struct Light_Struct {
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	float LightIntensity;
	float LightRadius;
};

layout (std430, binding = 3) readonly buffer Light_Index_Buffer {
	uint lightIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (location = 0) flat out uint BufferIndex;

void main()
{		
	BufferIndex = gl_InstanceID;
	gl_Position = cameraBuffer.pMatrix * cameraBuffer.vMatrix * lightBuffers[lightIndexes[BufferIndex]].mMatrix * vec4(vertex, 1.0); 
}
